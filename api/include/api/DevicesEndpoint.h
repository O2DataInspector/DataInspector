#ifndef DIPROXY_DEVICESENDPOINT_H
#define DIPROXY_DEVICESENDPOINT_H

#include "httplib.h"

#include "domain/DevicesService.h"

class DevicesEndpoint {
public:
  DevicesEndpoint(DevicesService& devicesService): devicesService(devicesService) {}

  void selectDevices(const httplib::Request& input, httplib::Response& output);
  void selectAll(const httplib::Request& input, httplib::Response& output);
  void unselectAll(const httplib::Request& input, httplib::Response& output);
  void getDevices(const httplib::Request& input, httplib::Response& output);

private:
  DevicesService& devicesService;
};

#endif //DIPROXY_DEVICESENDPOINT_H
