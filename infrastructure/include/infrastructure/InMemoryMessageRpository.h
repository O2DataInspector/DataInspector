#ifndef DIPROXY_INMEMORYMESSAGERPOSITORY_H
#define DIPROXY_INMEMORYMESSAGERPOSITORY_H

#include "domain/MessageRepository.h"
#include <mutex>
#include <deque>

class InMemoryMessageRepository: public MessageRepository {
public:
  InMemoryMessageRepository() {};

  void addMessage(const Message& message);
  std::vector<Message> getOldestMessages(const std::string& analysisId, int count);

private:
  std::mutex messageMutex;
  std::deque<Message> messages;
};

#endif //DIPROXY_INMEMORYMESSAGERPOSITORY_H
