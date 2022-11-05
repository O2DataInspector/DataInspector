#ifndef DIPROXY_MESSAGESERVICE_H
#define DIPROXY_MESSAGESERVICE_H

#include "domain/model/Message.h"
#include "MessageRepository.h"

class MessageService {
public:
  MessageService(MessageRepository& messageRepository): messageRepository(messageRepository) {};

  Message getMessage(const std::string& id);
  std::vector<Message> newerMessages(const std::string& runId, const std::string& messageId, const std::vector<std::string>& devices, int count);

private:
  MessageRepository& messageRepository;
};

#endif //DIPROXY_MESSAGESERVICE_H
