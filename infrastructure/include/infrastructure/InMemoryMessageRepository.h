#ifndef DIPROXY_INMEMORYMESSAGEREPOSITORY_H
#define DIPROXY_INMEMORYMESSAGEREPOSITORY_H

#include "domain/MessageRepository.h"
#include <mutex>
#include <deque>
#include <unordered_map>

#include <mongoc/mongoc.h>

class InMemoryMessageRepository: public MessageRepository {
public:
  InMemoryMessageRepository() {};

  std::string addMessage(const std::string& runId, const Message& message) override;
  Message getMessage(const std::string& id) override;
  std::vector<Message> getOldestMessages(const std::string& runId, int count) override;
  std::vector<std::string> newerMessages(const std::string& runId, uint64_t time, const std::vector<std::string>& devices, int count) override;

private:
  std::mutex messageMutex;
  std::unordered_map<std::string, std::deque<Message>> messages;
  int count = 0;
  mongoc_client_t *client;
};

#endif //DIPROXY_INMEMORYMESSAGEREPOSITORY_H
