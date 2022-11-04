#ifndef DIPROXY_DEVICELIST_H
#define DIPROXY_DEVICELIST_H

#include "domain/model/Device.h"

namespace Response {
struct DeviceList {
  std::vector<Device> devices;
};
}

#endif //DIPROXY_DEVICELIST_H
