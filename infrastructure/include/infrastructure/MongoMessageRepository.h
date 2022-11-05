#ifndef DIPROXY_MONGOMESSAGEREPOSITORY_H
#define DIPROXY_MONGOMESSAGEREPOSITORY_H

#include "domain/MessageRepository.h"
#include "mongoc.h"

class MongoMessageRepository: public MessageRepository {
public:
  MongoMessageRepository(mongoc_client_t* client): client(client) {};

  std::string addMessage(const std::string& runId, const Message& message) override;
  Message getMessage(const std::string& id) override;
  std::vector<Message> newerMessages(const std::string& runId, const std::string& messageId, const std::vector<std::string>& devices, int count) override;

private:
  mongoc_client_t* client;
};

#endif //DIPROXY_MONGOMESSAGEREPOSITORY_H
