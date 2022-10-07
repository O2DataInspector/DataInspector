#include "infrastructure/InMemoryMessageRpository.h"
#include <iostream>

void InMemoryMessageRepository::addMessage(const Message& message) {
  messageMutex.lock();
  std::cout << "MessageRepository::addMessage - " << message.raw << std::endl;

  messages.emplace_back(message);

  messageMutex.unlock();
}

std::vector<Message> InMemoryMessageRepository::getOldestMessages(const std::string& analysisId, int count) {
  std::vector<Message> response{};
  messageMutex.lock();
  std::cout << "MessageRepository::getOldestMessages" << std::endl;

  if(messages.empty()) {
    std::cout << "EMPTY" << std::endl;
    messageMutex.unlock();
    return response;
  }

  uint64_t realCount = std::min((uint64_t) messages.size(), (uint64_t) count);
  for(int i=0; i<realCount; i++) {
    response.emplace_back(messages.front());
    messages.pop_front();
  }

  std::cout << "SIZE - " << response.size() << std::endl;

  messageMutex.unlock();

  return response;
}
