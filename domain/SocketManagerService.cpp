#include "domain/SocketManagerService.h"
#include <iostream>
#include "domain/DIMessages.h"

SocketManagerService::SocketManagerService(int port, MessageRepository& messageRepository, DevicesRepository& devicesRepository): acceptor(port), devicesRepository(devicesRepository), messageRepository(messageRepository) {
  acceptor.start([this](DISocket& socket) -> void{
    std::cout<<"CONNECTED"<<std::endl;

    auto initMsg = socket.receive();

    if(initMsg.header.type != DIMessage::Header::Type::DEVICE_ON) {
      std::cout << "WRONG MESSAGE TYPE";
      return;
    }

    auto registerDeviceMsg = initMsg.get<DIMessages::RegisterDevice>();
    std::cout << registerDeviceMsg.name << " IS ACTIVE (analysisId=" << registerDeviceMsg.analysisId << ")" << std::endl;

    Device device;
    device.analysisId = registerDeviceMsg.analysisId;
    device.name = registerDeviceMsg.name;
    this->devicesRepository.addDevice(device, &socket);

    while (true) {
      auto msg = socket.receive();

      if(msg.header.type == DIMessage::Header::Type::DEVICE_OFF) {
        std::cout << msg.payload << " IS NOT ACTIVE" << std::endl;
        this->devicesRepository.removeDevice(registerDeviceMsg.analysisId, registerDeviceMsg.name);
        return;
      } else if(msg.header.type == DIMessage::Header::Type::DATA) {
        std::cout << "MESSAGE RECEIVED" << std::endl;
        this->messageRepository.addMessage(registerDeviceMsg.analysisId, Message{msg.get<std::string>()});
      }
    }
  });
}