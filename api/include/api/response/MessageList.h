#ifndef DIPROXY_MESSAGELIST_H
#define DIPROXY_MESSAGELIST_H

#include "domain/model/Message.h"
#include <vector>

namespace Response {
struct MessageList {
  std::vector<Message> messages;
};
}

#endif //DIPROXY_MESSAGELIST_H
