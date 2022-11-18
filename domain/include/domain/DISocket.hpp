//
// Created by arczipt on 09.08.22.
//

#ifndef DIPROXY_DISOCKET_HPP
#define DIPROXY_DISOCKET_HPP

#include <cstring>
#include "boost/asio.hpp"
#include "boost/endian/conversion.hpp"
#include "boost/archive/binary_oarchive.hpp"
#include "boost/archive/binary_iarchive.hpp"
#include <sstream>
#include <array>
#include <iostream>
#include <rapidjson/document.h>

#define ASIO_CATCH(customMessage) catch(boost::system::system_error& err){ auto msg = std::string{err.what()}; auto code = std::to_string(err.code().value()); throw std::runtime_error{"Exception in DataInspector (boost_code=" + code + ", boost_msg=" + msg + ") - " + customMessage}; }

template <typename... T>
struct always_static_assert : std::false_type {
};

template <typename... T>
inline constexpr bool always_static_assert_v = always_static_assert<T...>::value;

struct DIMessage {
  struct __attribute__ ((packed)) Header {
    enum class Type : uint32_t {
      INVALID = 0,
      DATA = 1,
      DEVICE_ON = 2,
      DEVICE_OFF = 3,
      INSPECT_ON = 4,
      INSPECT_OFF = 5,
      TERMINATE = 6
    };

    Header(Type type, uint64_t payloadSize) : typeLE(boost::endian::native_to_little(static_cast<uint32_t>(type))), payloadSizeLE(boost::endian::native_to_little(payloadSize)) {}
    Header(Type type) : Header(type, 0) {}
    Header() : Header(Type::INVALID, 0) {}

    Type type() const {
      return static_cast<DIMessage::Header::Type>(boost::endian::little_to_native(typeLE));
    }

    uint64_t payloadSize() const {
      return boost::endian::little_to_native(payloadSizeLE);
    }

  private:
    uint32_t typeLE;
    uint64_t payloadSizeLE;
  };

  template<typename T>
  DIMessage(Header::Type type, const T& payload)
  {
    uint64_t payloadSize = 0;
    if constexpr (std::is_base_of_v<std::string, T>) {
      payloadSize = payload.size();
      this->payload = new char[payloadSize];
      std::memcpy(this->payload, payload.data(), payloadSize);
    } else if constexpr (std::is_integral_v<T>) {
      payloadSize = sizeof(T);
      payload = boost::endian::native_to_little(payload);
      this->payload = new char[payloadSize];
      std::memcpy(this->payload, &payload, payloadSize);
    } else {
      static_assert(always_static_assert_v<T>, "DISocket: Cannot create message of this type.");
    }

    header = Header{type, payloadSize};
  }

  DIMessage(Header::Type type, const char* data, uint64_t size) : header(type, size) {
    payload = new char[size];
    std::memcpy(payload, data, size);
  }

  DIMessage(Header::Type type) : header(type, 0), payload() {}
  DIMessage() : header(), payload(nullptr) {}

  DIMessage(const DIMessage& other) noexcept : header(other.header) {
    this->payload = new char[other.header.payloadSize()];
    std::memcpy(this->payload, other.payload, other.header.payloadSize());
  }

  DIMessage(DIMessage&& other) noexcept : header(other.header), payload(other.payload) {
    other.payload = nullptr;
  }

  DIMessage& operator=(const DIMessage& other) noexcept {
    if(&other == this)
      return *this;

    this->header = Header{other.header};

    delete[] payload;
    this->payload = new char[other.header.payloadSize()];
    std::memcpy(this->payload, other.payload, other.header.payloadSize());

    return *this;
  }

  DIMessage& operator=(DIMessage&& other) noexcept {
    header = Header{other.header};
    delete[] payload;
    payload = other.payload;

    other.payload = nullptr;
    return *this;
  }

  ~DIMessage()
  {
    delete[] payload;
  }

  template<typename T>
  T get() const {
    if constexpr (std::is_same_v<std::string, T>) {
      return std::string{payload, header.payloadSize()};
    } else if constexpr (std::is_integral_v<T>) {
      return boost::endian::little_to_native(*((T*) payload));
    } else if constexpr (std::is_same_v<rapidjson::Document, T>) {
      rapidjson::Document doc;
      doc.Parse(payload, header.payloadSize());
      return doc;
    } else {
      static_assert(always_static_assert_v<T>, "DISocket: Cannot create object of this type.");
    }
  }

  Header header;
  char* payload;
};

class DISocket {
public:
  DISocket(boost::asio::ip::tcp::socket&& socket) : socket(std::move(socket)) {}

  void send(const DIMessage& message) {
    try {
      socket.send(std::array<boost::asio::const_buffer, 2>{
              boost::asio::buffer(&message.header, sizeof(DIMessage::Header)),
              boost::asio::buffer(message.payload, message.header.payloadSize())
      });
    } ASIO_CATCH("DISocket::send")
  }
  void asyncSend(DIMessage&& message, const std::function<void(std::size_t)>& sendCallback = nullptr, const std::function<void(std::size_t)>& errorHandler = nullptr) {
    try {
      auto* messagePtr = new DIMessage(std::move(message));

      socket.async_send(std::array<boost::asio::const_buffer, 2>{
              boost::asio::buffer(&messagePtr->header, sizeof(DIMessage::Header)),
              boost::asio::buffer(messagePtr->payload, messagePtr->header.payloadSize())
      }, [messagePtr, sendCallback, errorHandler](boost::system::error_code ec, std::size_t size) {
        if(ec.failed()) {
          if(errorHandler)
            errorHandler(size);
        } else {
          if(sendCallback)
            sendCallback(size);
        }
        delete messagePtr;
      });
    } ASIO_CATCH("DISocket::asyncSend")
  }

  DIMessage receive() {
    try {
      DIMessage message;
      socket.receive(boost::asio::buffer(&message.header, sizeof(DIMessage::Header)));

      if (message.header.payloadSize() > 0) {
        message.payload = new char[message.header.payloadSize()];
        socket.receive(boost::asio::buffer(message.payload, message.header.payloadSize()));
      }

      return message;
    } ASIO_CATCH("DISocket::receive")
  }

  void asyncReceive(const std::function<void(DIMessage)>& receiveCallback, const std::function<void(std::size_t)>& errorHandler = nullptr) {
    try {
      auto* messagePtr = new DIMessage{};
      auto receiveBody = [messagePtr, this, errorHandler, receiveCallback]() {
        messagePtr->payload = new char[messagePtr->header.payloadSize()];
        boost::asio::async_read(socket, boost::asio::buffer(messagePtr->payload, messagePtr->header.payloadSize()), [messagePtr, errorHandler, receiveCallback](boost::system::error_code ec, std::size_t size) {
          if(ec.failed()) {
            if(errorHandler)
              errorHandler(size);
          } else {
            receiveCallback(std::move(*messagePtr));
          }
          delete messagePtr;
        });
      };

      boost::asio::async_read(socket, boost::asio::buffer(&messagePtr->header, sizeof(DIMessage::Header)), [messagePtr, errorHandler, receiveBody, receiveCallback](boost::system::error_code ec, std::size_t size) {
        if(ec.failed()) {
          if(errorHandler)
            errorHandler(size);
          delete messagePtr;
        } else if(messagePtr->header.payloadSize() > 0) {
          receiveBody();
        } else {
          receiveCallback(std::move(*messagePtr));
          delete messagePtr;
        }
      });
    } ASIO_CATCH("DISocket::asyncReceive")
  }

private:
  boost::asio::ip::tcp::socket socket;
};

#endif //DIPROXY_DISOCKET_HPP
