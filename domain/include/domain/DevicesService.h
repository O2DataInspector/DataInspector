#ifndef DIPROXY_DEVICESSERVICE_H
#define DIPROXY_DEVICESSERVICE_H

#include "domain/model/Device.h"
#include "domain/DevicesRepository.h"

class DevicesService {
public:
  DevicesService(DevicesRepository& devicesRepository): devicesRepository(devicesRepository) {};

  Device getDevice(const std::string& analysisId, const std::string& deviceName);
  std::vector<Device> getDevices(const std::string& analysisId);

  void intercept(const std::string& analysisId);
  void intercept(const std::string& analysisId, const std::vector<std::string>& deviceNames);
  void stopInterception(const std::string& analysisId, const std::vector<std::string>& deviceNames);
  void stopInterception(const std::string& analysisId);

private:
  DevicesRepository& devicesRepository;
};

#endif //DIPROXY_DEVICESSERVICE_H
