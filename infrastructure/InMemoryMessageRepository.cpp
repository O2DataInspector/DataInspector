#include "infrastructure/InMemoryMessageRpository.h"
#include <iostream>

void InMemoryMessageRepository::addMessage(const std::string& runId, const Message& message) {
  messageMutex.lock();
  std::cout << "MessageRepository::addMessage - " << message.raw << std::endl;

  if(messages.count(runId) == 0)
    messages[runId] = std::deque<Message>{};

  messages[runId].emplace_back(message);

  messageMutex.unlock();
}

std::vector<Message> InMemoryMessageRepository::getOldestMessages(const std::string& runId, int count) {
  std::vector<Message> response{};
  messageMutex.lock();
  std::cout << "MessageRepository::getOldestMessages" << std::endl;

  auto& analysisMessages = messages[runId];

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
