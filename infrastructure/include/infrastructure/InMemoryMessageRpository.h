#ifndef DIPROXY_INMEMORYMESSAGERPOSITORY_H
#define DIPROXY_INMEMORYMESSAGERPOSITORY_H

#include "domain/MessageRepository.h"
#include <mutex>
#include <deque>
#include <unordered_map>

class InMemoryMessageRepository: public MessageRepository {
public:
  InMemoryMessageRepository() {};

  std::string addMessage(const std::string& runId, const Message& message) override;
  std::vector<Message> getOldestMessages(const std::string& runId, int count) override;

private:
  std::mutex messageMutex;
  std::unordered_map<std::string, std::deque<Message>> messages;
  int count = 0;
};

#endif //DIPROXY_INMEMORYMESSAGERPOSITORY_H
