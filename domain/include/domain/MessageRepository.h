#ifndef DIPROXY_MESSAGEREPOSITORY_H
#define DIPROXY_MESSAGEREPOSITORY_H

#include "domain/model/Message.h"
#include <vector>

class MessageRepository {
public:
  MessageRepository() {};

  virtual void addMessage(const std::string& analysisId, const Message& message) = 0;
  virtual std::vector<Message> getOldestMessages(const std::string& analysisId, int count) = 0;
};

#endif //DIPROXY_MESSAGEREPOSITORY_H
