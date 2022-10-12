#include <iostream>
#include "infrastructure/InMemoryDevicesRepository.h"
#include "domain/DIMessages.h"

void InMemoryDevicesRepository::addDevice(const Device& device, DISocket* socket) {
  devicesMutex.lock();
  std::cout << "DeviceRepository::addDevice" << std::endl;

  if(devices.count(device.runId) == 0)
    devices[device.runId] = std::vector<DeviceWithSocket>{};

  devices[device.runId].emplace_back(DeviceWithSocket{device, socket});
  devicesMutex.unlock();
}

void InMemoryDevicesRepository::removeDevice(const std::string& runId, const std::string& deviceName) {
  devicesMutex.lock();
  std::cout << "DeviceRepository::removeDevice" << std::endl;

  auto& analysisDevices = devices[runId];

  auto it = std::find_if(analysisDevices.begin(), analysisDevices.end(), [&runId, &deviceName](const DeviceWithSocket& deviceWithSocket) -> bool{
    return deviceWithSocket.device.name == deviceName;
  });

  if(it != analysisDevices.end())
    analysisDevices.erase(it);

  devicesMutex.unlock();
}

Device InMemoryDevicesRepository::getDevice(const std::string& runId, const std::string& deviceName) {
  devicesMutex.lock();
  std::cout << "DeviceRepository::getDevice" << std::endl;

  auto& analysisDevices = devices[runId];

  auto it = std::find_if(analysisDevices.begin(), analysisDevices.end(), [&runId, &deviceName](const DeviceWithSocket& deviceWithSocket) -> bool{
    return deviceWithSocket.device.name == deviceName;
  });

  if(it != analysisDevices.end()) {
    auto response = it->device;
    devicesMutex.unlock();
    return response;
  }

  devicesMutex.unlock();
  throw std::runtime_error("Wrong deviceName");
}

std::vector<Device> InMemoryDevicesRepository::getDevices(const std::string& runId) {
  devicesMutex.lock();
  std::cout << "DeviceRepository::getDevices" << std::endl;

  auto& analysisDevices = devices[runId];

  std::vector<Device> response{};
  std::transform(analysisDevices.begin(), analysisDevices.end(), std::back_inserter(response), [](const DeviceWithSocket& deviceWithSocket) -> Device{
    return deviceWithSocket.device;
  });
  devicesMutex.unlock();

  return response;
}

void InMemoryDevicesRepository::terminate(const std::string& runId) {
  devicesMutex.lock();
  std::cout << "DeviceRepository::intercept" << std::endl;
  for(auto& deviceWithSocket : devices[runId]) {
    deviceWithSocket.socket->send(DIMessage{DIMessage::Header::Type::TERMINATE});
  }
  devicesMutex.unlock();
}

void InMemoryDevicesRepository::intercept(const std::string& runId) {
  devicesMutex.lock();
  std::cout << "DeviceRepository::intercept" << std::endl;
  for(auto& deviceWithSocket : devices[runId]) {
    deviceWithSocket.socket->send(DIMessage{DIMessage::Header::Type::INSPECT_ON});
  }
  devicesMutex.unlock();
}

void InMemoryDevicesRepository::intercept(const std::string& runId, const std::vector<std::string>& deviceNames) {
  devicesMutex.lock();
  std::cout << "DeviceRepository::intercept" << std::endl;

  for(auto& deviceWithSocket : devices[runId]) {
    if(std::find(deviceNames.begin(), deviceNames.end(), deviceWithSocket.device.name) == deviceNames.end()) {
      deviceWithSocket.socket->send(DIMessage{DIMessage::Header::Type::INSPECT_OFF});
      continue;
    }

    deviceWithSocket.socket->send(DIMessage{DIMessage::Header::Type::INSPECT_ON});
  }
  devicesMutex.unlock();
}

void InMemoryDevicesRepository::stopInterception(const std::string& runId, const std::vector<std::string>& deviceNames) {
  devicesMutex.lock();
  std::cout << "DeviceRepository::stopInterception" << std::endl;
  for(auto& deviceWithSocket : devices[runId]) {
    if(std::find(deviceNames.begin(), deviceNames.end(), deviceWithSocket.device.name) == deviceNames.end())
      continue;

    deviceWithSocket.socket->send(DIMessage{DIMessage::Header::Type::INSPECT_OFF});
  }
  devicesMutex.unlock();
}

void InMemoryDevicesRepository::stopInterception(const std::string& runId) {
  devicesMutex.lock();
  std::cout << "DeviceRepository::stopInterception" << std::endl;
  for(auto& deviceWithSocket : devices[runId]) {
    deviceWithSocket.socket->send(DIMessage{DIMessage::Header::Type::INSPECT_OFF});
  }
  devicesMutex.unlock();
}