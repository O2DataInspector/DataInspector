#include "domain/SocketManagerService.h"
#include <iostream>
#include "domain/DIMessages.h"
#include "domain/Mappers.h"
#include "domain/model/Run.h"
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

SocketManagerService::SocketManagerService(int port,
                                           int threads,
                                           MessageRepository& messageRepository,
                                           DevicesRepository& devicesRepository,
                                           RunRepository& runRepository)
                                           : ioContext(),
                                             acceptor(ioContext, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
                                             threadPool(threads),
                                             devicesRepository(devicesRepository),
                                             messageRepository(messageRepository),
                                             runRepository(runRepository) {
  acceptNext();
}

void SocketManagerService::start() {
  for(int i=0; i<threadPool.size(); i++)
    threadPool.addJob([this]() {
      ioContext.run();
    });
}

std::function<void(std::size_t)> SocketManagerService::receiveErrorHandler(const Device& device, DISocket* socket) {
  return [this, device, socket](std::size_t size) {
    //TODO: multiple devices with the same name (time pipelining)
    std::cout << "SocketManagerService::receive - ERROR" << std::endl;
    devicesRepository.removeDevice(device.runId, device.name, socket);
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
      devicesRepository.removeDevice(device.runId, device.name, diSocket);
      return;
    }

    messageRepository.addMessage(device.runId, toDomain(message));
    diSocket->asyncReceive(receiveCallback(diSocket, device), receiveErrorHandler(device, diSocket));
  };
}

template <typename T>
T fromJson(const rapidjson::Document& doc);

template <>
DIMessages::RegisterDevice::Specs::Input fromJson(const rapidjson::Document& doc) {
  DIMessages::RegisterDevice::Specs::Input msg;

  auto& bindingValue = doc.FindMember("binding")->value;
  msg.binding = std::string{bindingValue.GetString(), bindingValue.GetStringLength()};

  auto& sourceChannelValue = doc.FindMember("sourceChannel")->value;
  msg.sourceChannel = std::string{sourceChannelValue.GetString(), sourceChannelValue.GetStringLength()};

  msg.timeslice = doc.FindMember("timeslice")->value.GetUint64();

  if(doc.HasMember("origin")) {
    auto& originValue = doc.FindMember("origin")->value;
    msg.origin = std::string{originValue.GetString(), originValue.GetStringLength()};
  }

  if(doc.HasMember("description")) {
    auto& descriptionValue = doc.FindMember("description")->value;
    msg.description = std::string{descriptionValue.GetString(), descriptionValue.GetStringLength()};
  }

  if(doc.HasMember("subSpec")) {
    msg.subSpec = doc.FindMember("subSpec")->value.GetUint();
  }

  return msg;
}

template <>
DIMessages::RegisterDevice::Specs::Output fromJson(const rapidjson::Document& doc) {
  DIMessages::RegisterDevice::Specs::Output msg;

  auto& bindingValue = doc.FindMember("binding")->value;
  msg.binding = std::string{bindingValue.GetString(), bindingValue.GetStringLength()};

  auto& channelValue = doc.FindMember("channel")->value;
  msg.channel = std::string{channelValue.GetString(), channelValue.GetStringLength()};

  msg.timeslice = doc.FindMember("timeslice")->value.GetUint64();
  msg.maxTimeslices = doc.FindMember("maxTimeslices")->value.GetUint64();

  auto& originValue = doc.FindMember("origin")->value;
  msg.origin = std::string{originValue.GetString(), originValue.GetStringLength()};

  auto& descriptionValue = doc.FindMember("description")->value;
  msg.description = std::string{descriptionValue.GetString(), descriptionValue.GetStringLength()};

  if(doc.HasMember("subSpec")) {
    msg.subSpec = doc.FindMember("subSpec")->value.GetUint();
  }

  return msg;
}

template <>
DIMessages::RegisterDevice::Specs::Forward fromJson(const rapidjson::Document& doc) {
  DIMessages::RegisterDevice::Specs::Forward msg;

  auto& bindingValue = doc.FindMember("binding")->value;
  msg.binding = std::string{bindingValue.GetString(), bindingValue.GetStringLength()};

  auto& channelValue = doc.FindMember("channel")->value;
  msg.channel = std::string{channelValue.GetString(), channelValue.GetStringLength()};

  msg.timeslice = doc.FindMember("timeslice")->value.GetUint64();
  msg.maxTimeslices = doc.FindMember("maxTimeslices")->value.GetUint64();

  if(doc.HasMember("origin")) {
    auto& originValue = doc.FindMember("origin")->value;
    msg.origin = std::string{originValue.GetString(), originValue.GetStringLength()};
  }

  if(doc.HasMember("description")) {
    auto& descriptionValue = doc.FindMember("description")->value;
    msg.description = std::string{descriptionValue.GetString(), descriptionValue.GetStringLength()};
  }

  if(doc.HasMember("subSpec")) {
    msg.subSpec = doc.FindMember("subSpec")->value.GetUint();
  }

  return msg;
}

template <>
DIMessages::RegisterDevice::Specs fromJson(const rapidjson::Document& doc) {
  DIMessages::RegisterDevice::Specs msg;

  msg.rank = doc.FindMember("rank")->value.GetUint64();
  msg.nSlots = doc.FindMember("nSlots")->value.GetUint64();
  msg.inputTimesliceId = doc.FindMember("inputTimesliceId")->value.GetUint64();
  msg.maxInputTimeslices = doc.FindMember("maxInputTimeslices")->value.GetUint64();

  msg.inputs = std::vector<DIMessages::RegisterDevice::Specs::Input>{};
  auto& inputsValue = doc.FindMember("inputs")->value;
  for(auto& inputValue : inputsValue.GetArray()) {
    rapidjson::Document inputDoc;
    inputDoc.CopyFrom(inputValue, inputDoc.GetAllocator());
    msg.inputs.push_back(fromJson<DIMessages::RegisterDevice::Specs::Input>(inputDoc));
  }

  msg.outputs = std::vector<DIMessages::RegisterDevice::Specs::Output>{};
  auto& outputsValue = doc.FindMember("outputs")->value;
  for(auto& outputValue : outputsValue.GetArray()) {
    rapidjson::Document outputDoc;
    outputDoc.CopyFrom(outputValue, outputDoc.GetAllocator());
    msg.outputs.push_back(fromJson<DIMessages::RegisterDevice::Specs::Output>(outputDoc));
  }

  msg.forwards = std::vector<DIMessages::RegisterDevice::Specs::Forward>{};
  auto& forwardsValue = doc.FindMember("forwards")->value;
  for(auto& forwardValue : forwardsValue.GetArray()) {
    rapidjson::Document forwardDoc;
    forwardDoc.CopyFrom(forwardValue, forwardDoc.GetAllocator());
    msg.forwards.push_back(fromJson<DIMessages::RegisterDevice::Specs::Forward>(forwardDoc));
  }

  return msg;
}

template <>
DIMessages::RegisterDevice fromJson(const rapidjson::Document& doc) {
  DIMessages::RegisterDevice msg;

  auto& nameValue = doc.FindMember("name")->value;
  msg.name = std::string{nameValue.GetString(), nameValue.GetStringLength()};

  auto& runIdValue = doc.FindMember("runId")->value;
  msg.runId = std::string{runIdValue.GetString(), runIdValue.GetStringLength()};

  rapidjson::Document specsDoc;
  specsDoc.CopyFrom(doc.FindMember("specs")->value, specsDoc.GetAllocator());
  msg.specs = fromJson<DIMessages::RegisterDevice::Specs>(specsDoc);

  return msg;
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

      auto registerDeviceMsg = fromJson<DIMessages::RegisterDevice>(initMsg.get<rapidjson::Document>());
      std::cout << registerDeviceMsg.name << " IS ACTIVE (runId=" << registerDeviceMsg.runId << ")" << std::endl;

      runRepository.updateStatus(registerDeviceMsg.runId, Run::Status::RUNNING);

      auto device = toDomain(registerDeviceMsg);
      devicesRepository.addDevice(device, diSocket);

      diSocket->asyncReceive(receiveCallback(diSocket, device), receiveErrorHandler(device, diSocket));
    });

    acceptNext();
  });
}
