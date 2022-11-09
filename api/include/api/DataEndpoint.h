#ifndef DIPROXY_DATAENDPOINT_H
#define DIPROXY_DATAENDPOINT_H

#include "httplib.h"

#include "domain/MessageService.h"
#include "api/response/MessageList.h"
#include "api/response/MessageHeaderList.h"

class DataEndpoint {
public:
  DataEndpoint(MessageService& messageService): messageService(messageService) {}

  Message getMessage(const httplib::Request& input, httplib::Response& output);
  Response::MessageHeaderList newerMessages(const httplib::Request& input, httplib::Response& output);

private:
  MessageService& messageService;
};

#endif //DIPROXY_DATAENDPOINT_H
