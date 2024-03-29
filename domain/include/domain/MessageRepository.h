#ifndef DIPROXY_MESSAGEREPOSITORY_H
#define DIPROXY_MESSAGEREPOSITORY_H

#include "domain/model/Message.h"
#include "domain/model/StatsRequest.h"
#include <vector>

struct MessageNotSaved : std::runtime_error {
  MessageNotSaved(const std::string& what) : std::runtime_error(what) {};
};

struct MessageNotFound : std::runtime_error {
  MessageNotFound(const std::string& what) : std::runtime_error(what) {};
};

class MessageRepository {
public:
  MessageRepository() {};

  virtual std::string addMessage(const std::string& runId, const Message& message) = 0;
  virtual Message getMessage(const std::string& id) = 0;
  virtual std::vector<Message> newerMessages(const std::string& runId, const std::string& messageId, const std::vector<std::string>& devices, int count) = 0;
  virtual std::vector<Message> search(const StatsRequest& request) = 0;
};

#endif //DIPROXY_MESSAGEREPOSITORY_H
