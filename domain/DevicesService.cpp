#include "domain/DevicesService.h"

Device DevicesService::getDevice(const std::string& analysisId, const std::string& deviceName) {
  return devicesRepository.getDevice(analysisId, deviceName);
}

std::vector<Device> DevicesService::getDevices(const std::string& analysisId) {
  return devicesRepository.getDevices(analysisId);
}

void DevicesService::intercept(const std::string& analysisId) {
  devicesRepository.intercept(analysisId);
}

void DevicesService::intercept(const std::string& analysisId, const std::vector<std::string>& deviceNames) {
  devicesRepository.intercept(analysisId, deviceNames);
}

void DevicesService::stopInterception(const std::string& analysisId, const std::vector<std::string>& deviceNames) {
  devicesRepository.stopInterception(analysisId, deviceNames);
}

void DevicesService::stopInterception(const std::string& analysisId) {
  devicesRepository.stopInterception(analysisId);
}
