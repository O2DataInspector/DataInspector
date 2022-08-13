#include <cstring>
#include <sstream>
#include <queue>
#include <unordered_map>

#include "looped_task.hpp"
#include "httplib.h"
#include "DISocket.hpp"

/* ENDPOINTS */
constexpr char RAW_DATA_ENDPOINT[] = "/raw-data";
constexpr char AVAILABLE_DEVICES_ENDPOINT[] = "/available-devices";
constexpr char INSPECTED_DATA_ENDPOINT[]    = "/inspected-data";
constexpr char STOP_INSPECTION_ENDPOINT[]   = "/stop";
constexpr char SELECT_DEVICES_ENDPOINT[]    = "/select-devices";
constexpr char SELECT_ALL_ENDPOINT[]        = "/select-all";

/* TIMEOUTS */
constexpr long DPLS_BROADCASTER_POLL_TIMEOUT = 1000;
constexpr long PROXY_POLL_TIMEOUT            = 1000;

/* CONSTANTS */
constexpr long MAX_LENGTH = 1 << 16;

using messages_queue = std::queue<DIMessage>;

void receive(messages_queue& messages, std::mutex& messages_mutex, std::vector<std::string>& devices) {
    DIAcceptor acceptor(8081);
    acceptor.start([&messages, &messages_mutex, &devices](DISocket& socket) -> void{
        std::cout<<"CONNECTED"<<std::endl;
        while (true) {
            auto msg = socket.receive();
            std::cout<<msg.payload<<std::endl;

            if(msg.header.type == DIMessage::Header::Type::REGISTER_DEVICE) {
                devices.push_back(msg.payload);
                continue;
            }

            messages_mutex.lock();
            messages.push(std::move(msg));
            messages_mutex.unlock();
        }
    });
}

int main(int argc, char* argv[]) {
    messages_queue messages;
    std::vector<std::string> devices;
    std::mutex messages_mutex;

    auto client_thread = std::thread{[&messages, &messages_mutex, &devices]() -> void{ receive(messages, messages_mutex, devices); }};

    httplib::Server handle;
    handle.Get(AVAILABLE_DEVICES_ENDPOINT,
               [](const httplib::Request& input, httplib::Response& output) {
                   std::string joined_names = "reader\ntpc-cluster-summary\nits-cluster-summary\nmerger\n";
                   output.set_content(joined_names, "text/plain");
               }
    );
    handle.Get(INSPECTED_DATA_ENDPOINT,
               [&messages, &messages_mutex](const httplib::Request& input, httplib::Response& output) {
                    std::cout<<"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"<<std::endl;
                   if (input.has_header("devices") && input.has_header("count")) {
                       //std::string devices = input.get_header_value("devices");
                       int count = std::stoi(input.get_header_value("count"));

                       std::stringstream ss;
                       messages_mutex.lock();
                       while(!messages.empty() && count > 0) {
                           count--;
                           ss << messages.front().payload;
                           messages.pop();

                           if(count > 0)
                               ss << ",\n";
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
               [](const httplib::Request& input, httplib::Response& output) {
               }
    );
    handle.Get(SELECT_ALL_ENDPOINT,
               [](const httplib::Request& input, httplib::Response& output) {
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
