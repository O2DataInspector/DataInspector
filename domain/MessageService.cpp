#include "domain/MessageService.h"

Message MessageService::getMessage(const std::string& id) {
  return messageRepository.getMessage(id);
}

std::vector<Message> MessageService::getOldestMessages(const std::string& runId, int count) {
  return messageRepository.getOldestMessages(runId, count);
}

std::vector<std::string> MessageService::newerMessages(const std::string& runId, uint64_t time, const std::vector<std::string>& devices, int count) {
  return messageRepository.newerMessages(runId, time, devices, count);
}