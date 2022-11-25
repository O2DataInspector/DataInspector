#ifndef DIPROXY_MONGODEVICESREPOSITORY_H
#define DIPROXY_MONGODEVICESREPOSITORY_H

#include "domain/DevicesRepository.h"
#include <string>
#include <unordered_map>
#include <mutex>
#include "mongoc.h"

struct DeviceWithSocket {
  Device device;
  DISocket* socket;
};

class MongoDevicesRepository: public DevicesRepository {
public:
  MongoDevicesRepository(mongoc_client_pool_t* pool): pool(pool) {};

  void addDevice(const Device& device, DISocket* socket) override;
  void removeDevice(const std::string& runId, const std::string& deviceName, DISocket* socket) override;
  Device getDevice(const std::string& runId, const std::string& deviceName) override;
  std::vector<Device> getDevices(const std::string& runId) override;

  void terminate(const std::string& runId) override;
  void intercept(const std::string& runId) override;
  void intercept(const std::string& runId, const std::vector<std::string>& deviceNames) override;
  void stopInterception(const std::string& runId, const std::vector<std::string>& deviceNames) override;
  void stopInterception(const std::string& runId) override;

private:
  mongoc_client_pool_t* pool;
  std::mutex runDevicesMutex;
  std::unordered_map<std::string, std::vector<DeviceWithSocket>> runDevices;
};

#endif //DIPROXY_MONGODEVICESREPOSITORY_H
