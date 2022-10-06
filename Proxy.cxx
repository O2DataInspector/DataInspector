#include <cstring>
#include <sstream>
#include <queue>
#include <unordered_map>

#include "looped_task.hpp"
#include "httplib.h"
#include "DISocket.hpp"
#include "DIMessages.h"

#include "boost/algorithm/string.hpp"

/* ENDPOINTS */
constexpr char RAW_DATA_ENDPOINT[] = "/raw-data";
constexpr char AVAILABLE_DEVICES_ENDPOINT[] = "/available-devices";
constexpr char INSPECTED_DATA_ENDPOINT[]    = "/inspected-data";
constexpr char STOP_INSPECTION_ENDPOINT[]   = "/stop";
constexpr char SELECT_DEVICES_ENDPOINT[]    = "/select-devices";
constexpr char SELECT_ALL_ENDPOINT[]        = "/select-all";

struct Device {
    DIMessages::RegisterDevice deviceSpec;
    DISocket& socket;
};

using messages_queue = std::queue<DIMessage>;
using devices_list = std::vector<Device>;

void receive(
        messages_queue& messages,
        std::mutex& messages_mutex,
        devices_list& devices,
        std::mutex& devices_mutex
) {
    DIAcceptor acceptor(8081);
    acceptor.start([&messages, &messages_mutex, &devices, &devices_mutex](DISocket& socket) -> void{
        std::cout<<"CONNECTED"<<std::endl;

        auto initMsg = socket.receive();

        if(initMsg.header.type != DIMessage::Header::Type::DEVICE_ON) {
            std::cout << "WRONG MESSAGE TYPE";
            return;
        }

        auto registerDeviceMsg = initMsg.get<DIMessages::RegisterDevice>();

        std::cout << registerDeviceMsg.name << "(analysisId=" << registerDeviceMsg.analysisId << ") IS ACTIVE" << std::endl;
        devices_mutex.lock();
        devices.push_back(Device{registerDeviceMsg, socket});
        devices_mutex.unlock();

        while (true) {
          auto msg = socket.receive();

          if(msg.header.type == DIMessage::Header::Type::DEVICE_OFF) {
            std::cout << msg.payload << " IS NOT ACTIVE" << std::endl;
            return;
          } else if(msg.header.type == DIMessage::Header::Type::DATA) {
            std::cout << "MESSAGE RECEIVED" << std::endl;
            messages_mutex.lock();
            messages.push(msg);
            messages_mutex.unlock();
          }
        }
    });
}

int main(int argc, char* argv[]) {
    messages_queue messages;
    std::mutex messages_mutex;

    devices_list devices;
    std::mutex devices_mutex;

    auto client_thread = std::thread{[&messages, &messages_mutex, &devices, &devices_mutex]() -> void{ receive(messages, messages_mutex, devices, devices_mutex); }};

    httplib::Server handle;
    handle.Get(AVAILABLE_DEVICES_ENDPOINT,
               [&devices, &devices_mutex](const httplib::Request& input, httplib::Response& output) {
                    std::cout << "GET " << AVAILABLE_DEVICES_ENDPOINT << std::endl;

                    devices_mutex.lock();

                    std::vector<std::string> names{};
                    for(auto& device : devices)
                        names.push_back(device.deviceSpec.name);

                    std::string joined_names = boost::algorithm::join(names, "\n");
                    output.set_content(joined_names, "text/plain");

                    devices_mutex.unlock();
               }
    );
    handle.Get(INSPECTED_DATA_ENDPOINT,
               [&messages, &messages_mutex](const httplib::Request& input, httplib::Response& output) {
                   std::cout << "GET " << INSPECTED_DATA_ENDPOINT << std::endl;

                   if (input.has_header("devices") && input.has_header("count")) {
//                       std::vector<std::string> devices{};
//                       std::string devicesString = input.get_header_value("devices");
//                       boost::split(devices, devicesString, boost::is_any_of(","));
                       int count = std::stoi(input.get_header_value("count"));

                       std::stringstream ss;
                       messages_mutex.lock();
                       while(true) {
                           count--;
                           ss << messages.front().payload;
                           messages.pop();

                           if(messages.empty() || count < 0) break;
                           else if(strlen(messages.front().payload)) ss << ",\n";
                       }
                       messages_mutex.unlock();

                       auto data = ss.str();
                       output.set_content("[" + data + "]", "application/json");
                   }
               }
    );
    handle.Get(STOP_INSPECTION_ENDPOINT,
               [&handle](const httplib::Request& input, httplib::Response& output) {
                   handle.stop();
               }
    );
    handle.Get(SELECT_DEVICES_ENDPOINT,
               [&devices, &devices_mutex](const httplib::Request& input, httplib::Response& output) {
                   std::cout << "GET " << SELECT_DEVICES_ENDPOINT << std::endl;

                   devices_mutex.lock();

                   std::vector<std::string> devicesNames{};
                   std::string devicesString = input.get_header_value("devices");
                   boost::split(devicesNames, devicesString, boost::is_any_of(","));

                   for(auto& device : devices) {
                       if(std::find(devicesNames.begin(), devicesNames.end(), device.deviceSpec.name) == devicesNames.end()) {
                           std::cout << "TURN OFF: " << device.deviceSpec.name << std::endl;
                           device.socket.send(DIMessage{DIMessage::Header::Type::INSPECT_OFF});
                       } else {
                           std::cout << "TURN ON: " << device.deviceSpec.name << std::endl;
                           device.socket.send(DIMessage{DIMessage::Header::Type::INSPECT_ON});
                       }
                   }

                   devices_mutex.unlock();
               }
    );
    handle.Get(SELECT_ALL_ENDPOINT,
               [&devices, &devices_mutex](const httplib::Request& input, httplib::Response& output) {
                   std::cout << "GET " << SELECT_ALL_ENDPOINT << std::endl;

                   devices_mutex.lock();

                   for(auto& device : devices) {
                       std::cout << "TURN ON: " << device.deviceSpec.name << std::endl;
                       device.socket.send(DIMessage{DIMessage::Header::Type::INSPECT_ON});
                   }

                   devices_mutex.unlock();
               }
    );
    handle.set_pre_routing_handler([](const httplib::Request &input, httplib::Response &output) -> httplib::SSLServer::HandlerResponse{
        output.set_header("Access-Control-Allow-Methods", "*");
        output.set_header("Access-Control-Allow-Headers", "*");
        output.set_header("Access-Control-Allow-Origin", "*");
        return httplib::Server::HandlerResponse::Unhandled;
    });
    handle.Options("/(.*)",
                   [&](const httplib::Request& input, httplib::Response& output) {
                       output.set_header("Content-Type", "text/html; charset=utf-8");
                       output.set_header("Connection", "close");
                   });

    auto address = "127.0.0.1";
    auto port = 8082;
    std::cout << "STARTING PROXY ON " << address << ":" << port << std::endl;
    handle.listen(address, port);
    client_thread.join();
}
