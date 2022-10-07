#ifndef DIPROXY_SOCKETMANAGERSERVICE_H
#define DIPROXY_SOCKETMANAGERSERVICE_H

#include "DISocket.hpp"
#include "MessageRepository.h"
#include "DevicesRepository.h"

class SocketManagerService {
public:
  SocketManagerService(int port, MessageRepository& messageRepository, DevicesRepository& devicesRepository);

private:
  DIAcceptor acceptor;

  MessageRepository& messageRepository;
  DevicesRepository& devicesRepository;
};

#endif //DIPROXY_SOCKETMANAGERSERVICE_H
