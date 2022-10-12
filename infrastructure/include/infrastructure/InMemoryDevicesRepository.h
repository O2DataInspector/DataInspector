#ifndef DIPROXY_INMEMORYDEVICESREPOSITORY_H
#define DIPROXY_INMEMORYDEVICESREPOSITORY_H

#include "domain/model/Device.h"
#include "domain/DISocket.hpp"
#include "domain/DevicesRepository.h"
#include <unordered_map>

struct DeviceWithSocket {
  Device device;
  DISocket* socket;
};

class InMemoryDevicesRepository: public DevicesRepository {
public:
  InMemoryDevicesRepository() {};

  void addDevice(const Device& device, DISocket* socket) override;
  void removeDevice(const std::string& runId, const std::string& deviceName) override;
  Device getDevice(const std::string& runId, const std::string& deviceName) override;
  std::vector<Device> getDevices(const std::string& runId) override;

  void intercept(const std::string& runId) override;
  void intercept(const std::string& runId, const std::vector<std::string>& deviceNames) override;
  void stopInterception(const std::string& runId, const std::vector<std::string>& deviceNames) override;
  void stopInterception(const std::string& runId) override;

private:
  std::mutex devicesMutex;
  std::unordered_map<std::string, std::vector<DeviceWithSocket>> devices;
};


#endif //DIPROXY_INMEMORYDEVICESREPOSITORY_H
