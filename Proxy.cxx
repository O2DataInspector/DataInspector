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

/* ENDPOINTS */
constexpr char RAW_DATA_ENDPOINT[] = "/raw-data";
constexpr char AVAILABLE_DEVICES_ENDPOINT[] = "/available-devices";
constexpr char INSPECTED_DATA_ENDPOINT[]    = "/inspected-data";
constexpr char STOP_INSPECTION_ENDPOINT[]   = "/stop";
constexpr char SELECT_DEVICES_ENDPOINT[]    = "/select-devices";
constexpr char SELECT_ALL_ENDPOINT[]        = "/select-all";


int main(int argc, char* argv[]) {
    httplib::Server handle;

    //infrastructure
    InMemoryMessageRepository messageRepository;
    InMemoryDevicesRepository devicesRepository;

    //services
    MessageService messageService{messageRepository};
    DevicesService devicesService{devicesRepository};
    std::thread socketManagerThread([&devicesRepository, &messageRepository]() -> void{
      SocketManagerService socketManagerService{8081, messageRepository, devicesRepository};
    });

    //api
    DataEndpoint dataEndpoint{messageService};
    DevicesEndpoint devicesEndpoint{devicesService};

    handle.Get(AVAILABLE_DEVICES_ENDPOINT, [&devicesEndpoint](const httplib::Request& input, httplib::Response& output) {
      devicesEndpoint.getDevices(input, output);
    });

    handle.Get(INSPECTED_DATA_ENDPOINT, [&dataEndpoint](const httplib::Request& input, httplib::Response& output) {
      dataEndpoint.getMessages(input, output);
    });

    handle.Get(STOP_INSPECTION_ENDPOINT, [&devicesEndpoint](const httplib::Request& input, httplib::Response& output) {
      devicesEndpoint.unselectAll(input, output);
    });

    handle.Get(SELECT_DEVICES_ENDPOINT, [&devicesEndpoint](const httplib::Request& input, httplib::Response& output) {
      devicesEndpoint.selectDevices(input, output);
    });

    handle.Get(SELECT_ALL_ENDPOINT, [&devicesEndpoint](const httplib::Request& input, httplib::Response& output) {
      devicesEndpoint.selectAll(input, output);
    });

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

    auto address = "127.0.0.1";
    auto port = 8082;
    std::cout << "STARTING PROXY ON " << address << ":" << port << std::endl;
    handle.listen(address, port);

    socketManagerThread.join();
}
