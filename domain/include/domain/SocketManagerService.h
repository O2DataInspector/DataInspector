#ifndef DIPROXY_SOCKETMANAGERSERVICE_H
#define DIPROXY_SOCKETMANAGERSERVICE_H

#include "DISocket.hpp"
#include "MessageRepository.h"
#include "DevicesRepository.h"
#include "utils/ThreadPool.h"

class SocketManagerService {
public:
  SocketManagerService(int port, int threads, MessageRepository& messageRepository, DevicesRepository& devicesRepository);

  void start();

private:
  void acceptNext();
  std::function<void(DIMessage)> receiveCallback(DISocket* diSocket, const Device& device);
  std::function<void(std::size_t)> receiveErrorHandler(const Device& device);

  boost::asio::io_context ioContext;
  boost::asio::ip::tcp::acceptor acceptor;

  MessageRepository& messageRepository;
  DevicesRepository& devicesRepository;

  ThreadPool threadPool;
};

#endif //DIPROXY_SOCKETMANAGERSERVICE_H
