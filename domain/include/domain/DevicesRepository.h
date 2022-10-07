#ifndef DIPROXY_DEVICESREPOSITORY_H
#define DIPROXY_DEVICESREPOSITORY_H

#include "domain/model/Device.h"
#include "DISocket.hpp"

class DevicesRepository {
public:
  DevicesRepository() {};

  virtual void addDevice(const Device& device, DISocket* socket) = 0;
  virtual void removeDevice(const std::string& analysisId, const std::string& deviceName) = 0;
  virtual Device getDevice(const std::string& analysisId, const std::string& deviceName) = 0;
  virtual std::vector<Device> getDevices(const std::string& analysisId) = 0;

  virtual void intercept(const std::string& analysisId) = 0;
  virtual void intercept(const std::string& analysisId, const std::vector<std::string>& deviceNames) = 0;
  virtual void stopInterception(const std::string& analysisId, const std::vector<std::string>& deviceNames) = 0;
  virtual void stopInterception(const std::string& analysisId) = 0;
};

#endif //DIPROXY_DEVICESREPOSITORY_H