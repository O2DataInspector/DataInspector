#include "domain/DevicesService.h"

Device DevicesService::getDevice(const std::string& runId, const std::string& deviceName) {
  return devicesRepository.getDevice(runId, deviceName);
}

std::vector<Device> DevicesService::getDevices(const std::string& runId) {
  return devicesRepository.getDevices(runId);
}

void DevicesService::intercept(const std::string& runId) {
  devicesRepository.intercept(runId);
}

void DevicesService::intercept(const std::string& runId, const std::vector<std::string>& deviceNames) {
  devicesRepository.intercept(runId, deviceNames);
}

void DevicesService::stopInterception(const std::string& runId, const std::vector<std::string>& deviceNames) {
  devicesRepository.stopInterception(runId, deviceNames);
}

void DevicesService::stopInterception(const std::string& runId) {
  devicesRepository.stopInterception(runId);
}
