#include <sstream>
#include <queue>

#include "httplib.h"
#include "boost/algorithm/string.hpp"

#include "api/DataEndpoint.h"
#include "api/DevicesEndpoint.h"

#include "domain/MessageService.h"
#include "domain/DevicesService.h"
#include "domain/SocketManagerService.h"

#include "infrastructure/InMemoryMessageRpository.h"
#include "infrastructure/InMemoryDevicesRepository.h"

#include "HTTPUtil.h"

/* ENDPOINTS */
constexpr char RAW_DATA_ENDPOINT[] = "/raw-data";
constexpr char AVAILABLE_DEVICES_ENDPOINT[] = "/available-devices";
constexpr char INSPECTED_DATA_ENDPOINT[]    = "/inspected-data";
constexpr char STOP_INSPECTION_ENDPOINT[]   = "/stop";
constexpr char SELECT_DEVICES_ENDPOINT[]    = "/select-devices";
constexpr char SELECT_ALL_ENDPOINT[]        = "/select-all";


int main(int argc, char* argv[]) {
  /**
   * INIT OBJECTS
   */
  /// INFRASTRUCTURE
  InMemoryMessageRepository messageRepository;
  InMemoryDevicesRepository devicesRepository;

  /// SERVICES
  MessageService messageService{messageRepository};
  DevicesService devicesService{devicesRepository};
  std::thread socketManagerThread([&devicesRepository, &messageRepository]() -> void{
    SocketManagerService socketManagerService{8081, messageRepository, devicesRepository};
  });

  /// API
  DataEndpoint dataEndpoint{messageService};
  DevicesEndpoint devicesEndpoint{devicesService};



  /**
   * INIT ENDPOINTS
   */
  httplib::Server handle;

  /// DATA
  addEndpoint(handle, HTTPMethod::GET, AVAILABLE_DEVICES_ENDPOINT, ENDPOINT_MEMBER_FUNC(devicesEndpoint, getDevices));

  /// DEVICES
  addEndpoint(handle, HTTPMethod::GET, INSPECTED_DATA_ENDPOINT, ENDPOINT_MEMBER_FUNC(dataEndpoint, getMessages));
  addEndpoint(handle, HTTPMethod::GET, STOP_INSPECTION_ENDPOINT, ENDPOINT_MEMBER_FUNC(devicesEndpoint, unselectAll));
  addEndpoint(handle, HTTPMethod::GET, SELECT_DEVICES_ENDPOINT, ENDPOINT_MEMBER_FUNC(devicesEndpoint, selectDevices));
  addEndpoint(handle, HTTPMethod::GET, SELECT_ALL_ENDPOINT, ENDPOINT_MEMBER_FUNC(devicesEndpoint, selectAll));



  /**
   * ADDITIONAL SETTINGS
   */
  handle.set_pre_routing_handler([](const httplib::Request &input, httplib::Response &output) -> httplib::Server::HandlerResponse{
    output.set_header("Access-Control-Allow-Methods", "*");
    output.set_header("Access-Control-Allow-Headers", "*");
    output.set_header("Access-Control-Allow-Origin", "*");
    return httplib::Server::HandlerResponse::Unhandled;
  });

  handle.Options(
          "/(.*)",
          [&](const httplib::Request& input, httplib::Response& output) {
            output.set_header("Content-Type", "text/html; charset=utf-8");
            output.set_header("Connection", "close");
          });



  /**
   * RUN
   */
  auto address = "127.0.0.1";
  auto port = 8082;
  std::cout << "STARTING PROXY ON " << address << ":" << port << std::endl;
  handle.listen(address, port);

  socketManagerThread.join();
}
