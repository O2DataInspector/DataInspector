#include "infrastructure/InMemoryMessageRpository.h"
#include <iostream>

void InMemoryMessageRepository::addMessage(const std::string& analysisId, const Message& message) {
  messageMutex.lock();
  std::cout << "MessageRepository::addMessage - " << message.raw << std::endl;

  if(messages.count(analysisId) == 0)
    messages[analysisId] = std::deque<Message>{};

  messages[analysisId].emplace_back(message);

  messageMutex.unlock();
}

std::vector<Message> InMemoryMessageRepository::getOldestMessages(const std::string& analysisId, int count) {
  std::vector<Message> response{};
  messageMutex.lock();
  std::cout << "MessageRepository::getOldestMessages" << std::endl;

  auto& analysisMessages = messages[analysisId];

  if(analysisMessages.empty()) {
    std::cout << "EMPTY" << std::endl;
    messageMutex.unlock();
    return response;
  }

  uint64_t realCount = std::min((uint64_t) analysisMessages.size(), (uint64_t) count);
  for(int i=0; i<realCount; i++) {
    response.emplace_back(analysisMessages.front());
    analysisMessages.pop_front();
  }

  std::cout << "SIZE - " << response.size() << std::endl;

  messageMutex.unlock();

  return response;
}
