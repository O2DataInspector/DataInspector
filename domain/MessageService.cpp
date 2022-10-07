#include "domain/MessageService.h"

std::vector<Message> MessageService::getOldestMessages(const std::string& analysisId, int count) {
  return messageRepository.getOldestMessages(analysisId, count);
}