#include "domain/SocketManagerService.h"
#include <iostream>
#include "domain/DIMessages.h"
#include "domain/Mappers.h"

SocketManagerService::SocketManagerService(int port, MessageRepository& messageRepository, DevicesRepository& devicesRepository): acceptor(port), devicesRepository(devicesRepository), messageRepository(messageRepository) {
  acceptor.start([this](DISocket& socket) -> void{
    std::cout<<"CONNECTED"<<std::endl;

    auto initMsg = socket.receive();

    if(initMsg.header.type != DIMessage::Header::Type::DEVICE_ON) {
      std::cout << "WRONG MESSAGE TYPE";
      return;
    }

    auto registerDeviceMsg = initMsg.get<DIMessages::RegisterDevice>();
    std::cout << registerDeviceMsg.name << " IS ACTIVE (analysisId=" << registerDeviceMsg.runId << ")" << std::endl;

    auto device = toDomain(registerDeviceMsg);
    this->devicesRepository.addDevice(device, &socket);

    while (true) {
      auto msg = socket.receive();

      if(msg.header.type == DIMessage::Header::Type::DEVICE_OFF) {
        std::cout << msg.payload << " IS NOT ACTIVE" << std::endl;
        this->devicesRepository.removeDevice(registerDeviceMsg.runId, registerDeviceMsg.name);
        return;
      } else if(msg.header.type == DIMessage::Header::Type::DATA) {
        std::cout << "MESSAGE RECEIVED" << std::endl;
        this->messageRepository.addMessage(registerDeviceMsg.runId, Message{msg.get<std::string>()});
      }
    }
  });
}