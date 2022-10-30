#include "domain/SocketManagerService.h"
#include <iostream>
#include "domain/DIMessages.h"
#include "domain/Mappers.h"
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

SocketManagerService::SocketManagerService(int port, int threads, MessageRepository& messageRepository, DevicesRepository& devicesRepository): ioContext(), acceptor(ioContext, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
                                                                                                                                               threadPool(threads), devicesRepository(devicesRepository), messageRepository(messageRepository) {
  acceptNext();
}

void SocketManagerService::start() {
  for(int i=0; i<threadPool.size(); i++)
    threadPool.addJob([this]() {
      ioContext.run();
    });
}

std::function<void(std::size_t)> SocketManagerService::receiveErrorHandler(const Device& device) {
  return [this, device](std::size_t size) {
    //TODO: multiple devices with the same name (time pipelining)
    std::cout << "SocketManagerService::receive - ERROR" << std::endl;
    devicesRepository.removeDevice(device.runId, device.name);
  };
}

Message toDomain(const DIMessage& diMessage) {
  Message msg;
  rapidjson::Document doc;
  doc.Parse(diMessage.payload, diMessage.header.payloadSize());

  auto& senderValue = doc.FindMember("sender")->value;
  msg.sender = std::string{senderValue.GetString(), senderValue.GetStringLength()};
  msg.startTime = doc.FindMember("startTime")->value.Get<uint64_t>();
  msg.duration = doc.FindMember("duration")->value.Get<uint64_t>();
  msg.creationTimer = doc.FindMember("creationTimer")->value.Get<uint64_t>();

  auto& originValue = doc.FindMember("origin")->value;
  msg.origin = std::string{originValue.GetString(), originValue.GetStringLength()};

  auto& descriptionValue = doc.FindMember("description")->value;
  msg.description = std::string{descriptionValue.GetString(), descriptionValue.GetStringLength()};
  msg.subSpecification = doc.FindMember("subSpecification")->value.Get<uint32_t>();
  msg.firstTForbit = doc.FindMember("firstTForbit")->value.Get<uint32_t>();
  msg.tfCounter = doc.FindMember("tfCounter")->value.Get<uint32_t>();
  msg.runNumber = doc.FindMember("runNumber")->value.Get<uint32_t>();
  msg.payloadSize = doc.FindMember("payloadSize")->value.Get<uint64_t>();
  msg.splitPayloadParts = doc.FindMember("splitPayloadParts")->value.Get<uint32_t>();

  auto& payloadSerializationValue = doc.FindMember("payloadSerialization")->value;
  msg.payloadSerialization = std::string{payloadSerializationValue.GetString(), payloadSerializationValue.GetStringLength()};
  msg.payloadSplitIndex = doc.FindMember("payloadSplitIndex")->value.Get<uint32_t>();

  auto& payloadValue = doc.FindMember("payload")->value;
  msg.payload = std::string{payloadValue.GetString(), payloadValue.GetStringLength()};

  if(doc.HasMember("payloadEndianness")) {
    auto& endiannessValue = doc.FindMember("payloadEndianness")->value;
    msg.payloadEndianness = std::string{endiannessValue.GetString(), endiannessValue.GetStringLength()};
  }

  if(doc.HasMember("taskHash"))
    msg.taskHash = doc.FindMember("taskHash")->value.Get<uint32_t>();

  msg.id = "";
  return msg;
}

std::function<void(DIMessage)> SocketManagerService::receiveCallback(DISocket* diSocket, const Device& device) {
  return [this,diSocket,device](DIMessage message) {
    if(message.header.type() == DIMessage::Header::Type::DEVICE_OFF) {
      std::cout << message.get<std::string>() << " IS NOT ACTIVE (runId=" << device.runId << ")" << std::endl;
      return;
    }

    messageRepository.addMessage(device.runId, toDomain(message));
    diSocket->asyncReceive(receiveCallback(diSocket, device), receiveErrorHandler(device));
  };
}

void SocketManagerService::acceptNext() {
  acceptor.async_accept([this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
    if(ec.failed()) {
      /// We can't do anything about it, because we still don't know runId for this device
      std::cout << "SocketManagerService::acceptNext - ERROR: " << ec.message() << std::endl;
      return;
    }

    /// Wait for registration message
    auto* diSocket = new DISocket{std::move(socket)};
    diSocket->asyncReceive([this, diSocket](DIMessage initMsg) {
      std::cout << "SocketManagerService::acceptNext - CONNECTED" << std::endl;
      if(initMsg.header.type() != DIMessage::Header::Type::DEVICE_ON) {
        std::cout << "WRONG MESSAGE TYPE: " << static_cast<uint32_t>(initMsg.header.type()) << std::endl;
        return;
      }

      auto registerDeviceMsg = initMsg.get<DIMessages::RegisterDevice>();
      std::cout << registerDeviceMsg.name << " IS ACTIVE (runId=" << registerDeviceMsg.runId << ")" << std::endl;

      auto device = toDomain(registerDeviceMsg);
      devicesRepository.addDevice(device, diSocket);

      diSocket->asyncReceive(receiveCallback(diSocket, device), receiveErrorHandler(device));
    });

    acceptNext();
  });
}
