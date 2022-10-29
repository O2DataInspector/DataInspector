#ifndef DIPROXY_MESSAGEREPOSITORY_H
#define DIPROXY_MESSAGEREPOSITORY_H

#include "domain/model/Message.h"
#include <vector>

class MessageRepository {
public:
  MessageRepository() {};

  virtual std::string addMessage(const std::string& runId, const Message& message) = 0;
  virtual Message getMessage(const std::string& id) = 0;
  virtual std::vector<Message> getOldestMessages(const std::string& runId, int count) = 0;
  virtual std::vector<std::string> newerMessages(const std::string& runId, const std::string& messageId, const std::vector<std::string>& devices, int count) = 0;
};

#endif //DIPROXY_MESSAGEREPOSITORY_H
