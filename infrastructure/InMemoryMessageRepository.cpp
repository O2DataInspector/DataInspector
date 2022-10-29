#include "infrastructure/InMemoryMessageRpository.h"
#include <iostream>
#include <algorithm>

std::string InMemoryMessageRepository::addMessage(const std::string& runId, const Message& message) {
  messageMutex.lock();
  std::cout << "MessageRepository::addMessage" << std::endl;

  if(messages.count(runId) == 0)
    messages[runId] = std::deque<Message>{};

  auto id = message.id;
  if(id.empty())
    id = std::to_string(count++);

  messages[runId].emplace_back(message);
  (--messages[runId].end())->id = id;

  messageMutex.unlock();

  return id;
}

Message InMemoryMessageRepository::getMessage(const std::string& id) {
  messageMutex.lock();
  std::cout << "MessageRepository::getMessage" << std::endl;

  for(auto& run : messages) {
    for(auto& message : run.second) {
      if(message.id == id) {
        auto response = message;
        messageMutex.unlock();
        return response;
      }
    }
  }

  messageMutex.unlock();
  throw std::runtime_error("Id not found");
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

std::vector<std::string> InMemoryMessageRepository::newerMessages(const std::string& runId, const std::string& messageId, const std::vector<std::string>& devices, int count) {
  std::vector<std::string> response{};
  messageMutex.lock();
  std::cout << "MessageRepository::newerMessages" << std::endl;

  auto& analysisMessages = messages[runId];

  if(analysisMessages.empty()) {
    std::cout << "EMPTY" << std::endl;
    messageMutex.unlock();
    return response;
  }

  uint64_t time = std::find_if(analysisMessages.begin(), analysisMessages.end(), [messageId](const Message& message) {
    return message.id == messageId;
  })->creationTimer;
  uint64_t realCount = std::min((uint64_t) analysisMessages.size(), (uint64_t) count);
  for(auto& msg : analysisMessages) {
    if(msg.creationTimer > time && std::find(devices.begin(), devices.end(), msg.sender) != devices.end()) {
      response.push_back(msg.id);
      realCount--;

      if(realCount == 0)
        break;
    }
  }

  messageMutex.unlock();

  return response;
}
