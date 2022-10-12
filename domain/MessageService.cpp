#include "domain/MessageService.h"

std::vector<Message> MessageService::getOldestMessages(const std::string& runId, int count) {
  return messageRepository.getOldestMessages(runId, count);
}