#ifndef DIPROXY_MESSAGEHEADERLIST_H
#define DIPROXY_MESSAGEHEADERLIST_H

#include <vector>
#include <string>

namespace Response {
struct MessageHeader {
  std::string id;
  std::string device;
  uint64_t startTime;
  uint64_t duration;
  uint64_t creationTimer;
  std::string origin;
  std::string description;
  uint64_t payloadSize;
  std::string payloadSerialization;
};

struct MessageHeaderList {
  std::vector<MessageHeader> messages;
};
}

#endif //DIPROXY_MESSAGEHEADERLIST_H
