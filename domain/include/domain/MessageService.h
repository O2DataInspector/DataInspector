#ifndef DIPROXY_MESSAGESERVICE_H
#define DIPROXY_MESSAGESERVICE_H

#include "domain/model/Message.h"
#include "MessageRepository.h"

class MessageService {
public:
  MessageService(MessageRepository& messageRepository): messageRepository(messageRepository) {};

  std::vector<Message> getOldestMessages(const std::string& runId, int count);

private:
  MessageRepository& messageRepository;
};

#endif //DIPROXY_MESSAGESERVICE_H
