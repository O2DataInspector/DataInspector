#include <iostream>
#include "infrastructure/MongoDevicesRepository.h"
#include "domain/DIMessages.h"
#include <boost/lexical_cast.hpp>

void MongoDevicesRepository::addDevice(const Device& device, DISocket* socket) {
  runDevicesMutex.lock();
  std::cout << "DeviceRepository::addDevice" << std::endl;

  if(runDevices.count(device.runId) == 0)
    runDevices[device.runId] = std::vector<DeviceWithSocket>{};

  runDevices[device.runId].emplace_back(DeviceWithSocket{device, socket});
  runDevicesMutex.unlock();

  //SAVE IN DB
  bson_t *doc;
  bson_oid_t oid;
  bson_error_t error;
  mongoc_client_t *client;
  mongoc_collection_t *collection;

  client = mongoc_client_pool_pop(pool);
  collection = mongoc_client_get_collection(client, "diProxy", "devices");
  doc = bson_new();

  bson_oid_init(&oid, NULL);
  BSON_APPEND_OID(doc, "_id", &oid);

  BSON_APPEND_UTF8(doc, "runId", device.runId.c_str());
  BSON_APPEND_UTF8(doc, "name", device.name.c_str());
  BSON_APPEND_UTF8(doc, "rank", std::to_string(device.specs.rank).c_str());
  BSON_APPEND_UTF8(doc, "nSlots", std::to_string(device.specs.nSlots).c_str());
  BSON_APPEND_UTF8(doc, "inputTimesliceId", std::to_string(device.specs.inputTimesliceId).c_str());
  BSON_APPEND_UTF8(doc, "maxInputTimeslices", std::to_string(device.specs.maxInputTimeslices).c_str());
  if(!mongoc_collection_insert_one(
          collection, doc, NULL, NULL, &error))
  {
    fprintf(stderr, "%s\n", error.message);
  }
  bson_destroy(doc);
  mongoc_collection_destroy(collection);

  mongoc_client_pool_push (pool, client);
}

void MongoDevicesRepository::removeDevice(const std::string& runId, const std::string& deviceName, DISocket* socket) {
  runDevicesMutex.lock();
  std::cout << "DeviceRepository::removeDevice" << std::endl;

  auto& analysisDevices = runDevices[runId];

  auto it = std::find_if(analysisDevices.begin(), analysisDevices.end(), [&deviceName, socket](const DeviceWithSocket& deviceWithSocket) -> bool{
    return deviceWithSocket.device.name == deviceName && deviceWithSocket.socket == socket;
  });

  if(it != analysisDevices.end())
    analysisDevices.erase(it);

  runDevicesMutex.unlock();
}

Device MongoDevicesRepository::getDevice(const std::string& runId, const std::string& deviceName) {
  runDevicesMutex.lock();
  if(runDevices.count(runId) > 0) {
    Device device;
    for(auto& d : runDevices[runId]) {
      if(d.device.name == deviceName) {
        device = d.device;
        runDevicesMutex.unlock();
        return device;
      }
    }
  }
  runDevicesMutex.unlock();

  bson_iter_t iter;
  bson_t *query;
  const bson_t *doc;
  mongoc_cursor_t *cursor;
  mongoc_client_t *client;
  mongoc_collection_t *collection;
  Device device;

  client = mongoc_client_pool_pop(pool);
  collection = mongoc_client_get_collection(client, "diProxy", "devices");

  query = bson_new();
  BSON_APPEND_UTF8(query, "runId", runId.c_str());
  BSON_APPEND_UTF8(query, "name", deviceName.c_str());

  cursor = mongoc_collection_find_with_opts(
          collection, query, NULL, NULL);

  while(mongoc_cursor_next(cursor, &doc))
  {
    device.runId = runId;

    bson_iter_init_find(&iter, doc, "name");
    device.name = bson_iter_utf8(&iter, NULL);

    bson_iter_init_find(&iter, doc, "rank");
    device.specs.rank = boost::lexical_cast<uint64_t>(bson_iter_utf8(&iter, NULL));

    bson_iter_init_find(&iter, doc, "nSlots");
    device.specs.nSlots = boost::lexical_cast<uint64_t>(bson_iter_utf8(&iter, NULL));

    bson_iter_init_find(&iter, doc, "inputTimesliceId");
    device.specs.inputTimesliceId = boost::lexical_cast<uint64_t>(bson_iter_utf8(&iter, NULL));

    bson_iter_init_find(&iter, doc, "maxInputTimeslices");
    device.specs.maxInputTimeslices = boost::lexical_cast<uint64_t>(bson_iter_utf8(&iter, NULL));

    device.isSelected = false;
    device.specs.inputs = {};
    device.specs.outputs = {};
    device.specs.forwards = {};
  }
  bson_destroy(query);
  mongoc_cursor_destroy(cursor);
  mongoc_collection_destroy(collection);
  mongoc_client_pool_push (pool, client);
  return device;
}

std::vector<Device> MongoDevicesRepository::getDevices(const std::string& runId) {
  runDevicesMutex.lock();
  if(runDevices.count(runId) > 0) {
    std::vector<Device> devices;
    for(auto& device : runDevices[runId]) {
      devices.emplace_back(device.device);
    }
    runDevicesMutex.unlock();
    return devices;
  }
  runDevicesMutex.unlock();

  bson_iter_t iter;
  bson_t *query;
  const bson_t *doc;
  mongoc_cursor_t *cursor;
  mongoc_client_t *client;
  mongoc_collection_t *collection;

  client = mongoc_client_pool_pop(pool);
  collection = mongoc_client_get_collection(client, "diProxy", "devices");

  query = bson_new();
  BSON_APPEND_UTF8(query, "runId", runId.c_str());

  cursor = mongoc_collection_find_with_opts(collection, query, NULL, NULL);

  std::vector<Device> devices;
  while(mongoc_cursor_next(cursor, &doc))
  {
    Device device;
    device.runId = runId;

    bson_iter_init_find(&iter, doc, "name");
    device.name = bson_iter_utf8(&iter, NULL);

    bson_iter_init_find(&iter, doc, "rank");
    device.specs.rank = boost::lexical_cast<uint64_t>(bson_iter_utf8(&iter, NULL));

    bson_iter_init_find(&iter, doc, "nSlots");
    device.specs.nSlots = boost::lexical_cast<uint64_t>(bson_iter_utf8(&iter, NULL));

    bson_iter_init_find(&iter, doc, "inputTimesliceId");
    device.specs.inputTimesliceId = boost::lexical_cast<uint64_t>(bson_iter_utf8(&iter, NULL));

    bson_iter_init_find(&iter, doc, "maxInputTimeslices");
    device.specs.maxInputTimeslices = boost::lexical_cast<uint64_t>(bson_iter_utf8(&iter, NULL));

    device.isSelected = false;
    device.specs.inputs = {};
    device.specs.outputs = {};
    device.specs.forwards = {};
  }
  bson_destroy(query);
  mongoc_cursor_destroy(cursor);
  mongoc_collection_destroy(collection);
  mongoc_client_pool_push (pool, client);

  return devices;
}

void MongoDevicesRepository::terminate(const std::string& runId) {
  runDevicesMutex.lock();
  std::cout << "DeviceRepository::intercept" << std::endl;
  for(auto& deviceWithSocket : runDevices[runId]) {
    deviceWithSocket.socket->asyncSend(DIMessage{DIMessage::Header::Type::TERMINATE});
  }
  runDevicesMutex.unlock();
}

void MongoDevicesRepository::intercept(const std::string& runId) {
  runDevicesMutex.lock();
  std::cout << "DeviceRepository::intercept" << std::endl;
  for(auto& deviceWithSocket : runDevices[runId]) {
    deviceWithSocket.device.isSelected = true;
    deviceWithSocket.socket->asyncSend(DIMessage{DIMessage::Header::Type::INSPECT_ON});
  }
  runDevicesMutex.unlock();
}

void MongoDevicesRepository::intercept(const std::string& runId, const std::vector<std::string>& deviceNames) {
  runDevicesMutex.lock();
  std::cout << "DeviceRepository::intercept" << std::endl;

  for(auto& deviceWithSocket : runDevices[runId]) {
    if(std::find(deviceNames.begin(), deviceNames.end(), deviceWithSocket.device.name) == deviceNames.end()) {
      deviceWithSocket.device.isSelected = false;
      deviceWithSocket.socket->asyncSend(DIMessage{DIMessage::Header::Type::INSPECT_OFF});
      continue;
    }

    deviceWithSocket.device.isSelected = true;
    deviceWithSocket.socket->asyncSend(DIMessage{DIMessage::Header::Type::INSPECT_ON});
  }
  runDevicesMutex.unlock();
}

void MongoDevicesRepository::stopInterception(const std::string& runId, const std::vector<std::string>& deviceNames) {
  runDevicesMutex.lock();
  std::cout << "DeviceRepository::stopInterception" << std::endl;
  for(auto& deviceWithSocket : runDevices[runId]) {
    if(std::find(deviceNames.begin(), deviceNames.end(), deviceWithSocket.device.name) == deviceNames.end())
      continue;

    deviceWithSocket.device.isSelected = false;
    deviceWithSocket.socket->asyncSend(DIMessage{DIMessage::Header::Type::INSPECT_OFF});
  }
  runDevicesMutex.unlock();
}

void MongoDevicesRepository::stopInterception(const std::string& runId) {
  runDevicesMutex.lock();
  std::cout << "DeviceRepository::stopInterception" << std::endl;
  for(auto& deviceWithSocket : runDevices[runId]) {
    deviceWithSocket.device.isSelected = false;
    deviceWithSocket.socket->asyncSend(DIMessage{DIMessage::Header::Type::INSPECT_OFF});
  }
  runDevicesMutex.unlock();
}