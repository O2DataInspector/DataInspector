#include "domain/MessageService.h"

std::vector<Message> MessageService::getOldestMessages(const std::string& runId, int count) {
  return messageRepository.getOldestMessages(runId, count);
}

std::vector<std::string> MessageService::newerMessages(const std::string& runId, uint64_t time, const std::vector<std::string>& devices) {
  return messageRepository.newerMessages(runId, time, devices);
}