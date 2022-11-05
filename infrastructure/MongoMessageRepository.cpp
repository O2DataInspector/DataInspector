#include "infrastructure/MongoMessageRepository.h"
#include <iostream>
#include <algorithm>

std::string MongoMessageRepository::addMessage(const std::string& runId, const Message& message) {
  //TODO

  auto* client = mongoc_client_pool_pop (pool);
  //....
  mongoc_client_pool_push (pool, client);

  throw std::runtime_error("Not implemented");
}

Message MongoMessageRepository::getMessage(const std::string& id) {
  //TODO
  throw std::runtime_error("Not implemented");
}

std::vector<Message> MongoMessageRepository::newerMessages(const std::string& runId, const std::string& messageId, const std::vector<std::string>& devices, int count) {
  //TODO - wyciągnąć wiadomości:
  //  - których id jest "większy" od messageId
  //  - były wysłane z device'ów w devices
  //  - maksymalnie limit
  throw std::runtime_error("Not implemented");
}
