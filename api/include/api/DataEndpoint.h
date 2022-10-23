#ifndef DIPROXY_DATAENDPOINT_H
#define DIPROXY_DATAENDPOINT_H

#include "httplib.h"

#include "domain/MessageService.h"

class DataEndpoint {
public:
  DataEndpoint(MessageService& messageService): messageService(messageService) {}

  void getMessages(const httplib::Request& input, httplib::Response& output);
  void newerMessages(const httplib::Request& input, httplib::Response& output);

private:
  MessageService& messageService;
};

#endif //DIPROXY_DATAENDPOINT_H
