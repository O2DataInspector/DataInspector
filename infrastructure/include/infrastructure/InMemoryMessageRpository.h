#ifndef DIPROXY_INMEMORYMESSAGERPOSITORY_H
#define DIPROXY_INMEMORYMESSAGERPOSITORY_H

#include "domain/MessageRepository.h"
#include <mutex>
#include <deque>
#include <unordered_map>

class InMemoryMessageRepository: public MessageRepository {
public:
  InMemoryMessageRepository() {};

  void addMessage(const std::string& analysisId, const Message& message) override;
  std::vector<Message> getOldestMessages(const std::string& analysisId, int count) override;

private:
  std::mutex messageMutex;
  std::unordered_map<std::string, std::deque<Message>> messages;
};

#endif //DIPROXY_INMEMORYMESSAGERPOSITORY_H
