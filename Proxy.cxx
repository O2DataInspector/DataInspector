#include <cstring>
#include <sstream>
#include <queue>
#include <unordered_map>

#include "looped_task.hpp"
#include "boost/asio.hpp"
#include "httplib.h"

using messages_queue = std::queue<std::string>;

/* ENDPOINTS */
constexpr char RAW_DATA_ENDPOINT[] = "/raw-data";

/* TIMEOUTS */
constexpr long DPLS_BROADCASTER_POLL_TIMEOUT = 1000;
constexpr long PROXY_POLL_TIMEOUT            = 1000;

/* CONSTANTS */
constexpr long MAX_LENGTH = 1 << 16;

static void fromLE(const char* le, uint64_t* n) {
    *n = ((uint64_t) ((uint8_t) le[7]) << 56);
    *n += ((uint64_t) ((uint8_t) le[6]) << 48);
    *n += ((uint64_t) ((uint8_t) le[5]) << 40);
    *n += ((uint64_t) ((uint8_t) le[4]) << 32);
    *n += ((uint64_t) ((uint8_t) le[3]) << 24);
    *n += ((uint64_t) ((uint8_t) le[2]) << 16);
    *n += ((uint64_t) ((uint8_t) le[1]) << 8);
    *n += ((uint64_t) (uint8_t) le[0]);
}

static void fromLE(const char* le, uint32_t* n) {
    *n = ((uint64_t) le[3] << 24)
    + ((uint64_t) le[2] << 16)
    + ((uint64_t) le[1] << 8)
    + (uint64_t) le[0];
}

struct DIPacket {
    struct Header {
        static const uint32_t HEADER_SIZE = 12;
        enum class Type : uint32_t {
            DATA = 1
        };

        char type[4];
        char payloadSize[8];
    };

    Header header;
    char* payload;
};

void receive(messages_queue& messages, std::mutex& messages_mutex) {
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::acceptor acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 8081));
    auto socket = acceptor.accept();

    for (;;)
    {
        DIPacket::Header header{};

        boost::system::error_code error;
        size_t length = socket.read_some(boost::asio::buffer(&header, DIPacket::Header::HEADER_SIZE), error);

        if (error == boost::asio::error::eof)
            break; // Connection closed cleanly by peer.
        else if (error)
            throw boost::system::system_error(error); // Some other error.

        uint64_t payloadSize;
        fromLE(header.payloadSize, &payloadSize);
        char payload[payloadSize];

        uint64_t read = 0;
        while(read < payloadSize) {
            length = socket.read_some(boost::asio::buffer(payload + read, payloadSize - read), error);
            read += length;
        }

        if (error == boost::asio::error::eof)
            break; // Connection closed cleanly by peer.
        else if (error)
            throw boost::system::system_error(error); // Some other error.

        messages_mutex.lock();
        messages.push(std::string{payload, payloadSize});
        messages_mutex.unlock();
    }
}

int main(int argc, char* argv[]) {
    messages_queue messages;
    std::mutex messages_mutex;

    auto client_thread = std::thread{[&messages, &messages_mutex]() -> void{ receive(messages, messages_mutex); }};

    httplib::Server handle;
    handle.Get(RAW_DATA_ENDPOINT,
               [&messages, &messages_mutex](const httplib::Request& input, httplib::Response& output) {
                   std::stringstream ss;
                   messages_mutex.lock();
                   while(!messages.empty()) {
                       ss << messages.front();
                       messages.pop();

                       if(!messages.empty())
                           ss << ",\n";
                   }
                   messages_mutex.unlock();

                   auto data = ss.str();
                   output.set_content("[" + data + "]", "application/json");
               }
    );
    handle.Options("/(.*)",
                   [&](const httplib::Request& input, httplib::Response& output) {
                       output.set_header("Access-Control-Allow-Methods", " POST, GET, OPTIONS");
                       output.set_header("Content-Type", "text/html; charset=utf-8");
                       output.set_header("Access-Control-Allow-Headers", "X-Requested-With, Content-Type, Accept");
                       output.set_header("Access-Control-Allow-Origin", "*");
                       output.set_header("Connection", "close");
                   });

    auto address = "127.0.0.1";
    auto port = 8082;
    std::cout << "STARTING PROXY ON " << address << ":" << port << std::endl;
    handle.listen(address, port);
    client_thread.join();
}
