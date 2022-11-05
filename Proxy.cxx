#include <sstream>
#include <queue>

#include "httplib.h"
#include "boost/algorithm/string.hpp"

#include "api/DataEndpoint.h"
#include "api/DevicesEndpoint.h"
#include "api/AnalysisEndpoint.h"

#include "domain/MessageService.h"
#include "domain/DevicesService.h"
#include "domain/SocketManagerService.h"
#include "domain/AnalysisService.h"
#include "domain/BuildManager.h"
#include "domain/RunManager.h"

#include "infrastructure/MongoMessageRepository.h"
#include "infrastructure/InMemoryDevicesRepository.h"
#include "infrastructure/MongoAnalysisRepository.h"
#include "infrastructure/InMemoryRunRepository.h"

#include "HTTPUtil.h"
#include "mongoc.h"

/* ENDPOINTS */
constexpr char AVAILABLE_DEVICES_ENDPOINT[] = "/available-devices";
constexpr char STOP_INSPECTION_ENDPOINT[]   = "/stop";
constexpr char SELECT_DEVICES_ENDPOINT[]    = "/select-devices";
constexpr char SELECT_ALL_ENDPOINT[]        = "/select-all";
constexpr char IMPORT_ANALYSIS_ENDPOINT[]   = "/analysis/import";
constexpr char ANALYSIS_STATUS_ENDPOINT[]   = "/analysis/status";
constexpr char LIST_WORKFLOWS_ENDPOINT[]    = "/analysis/workflows";
constexpr char START_ANALYSIS_ENDPOINT[]    = "/analysis/start";
constexpr char STOP_ANALYSIS_ENDPOINT[]     = "/analysis/stop";

constexpr char NEWER_MESSAGES_ENDPOINT[]    = "/messages/newer";
constexpr char GET_MESSAGE_ENDPOINT[]       = "/messages";

mongoc_client_pool_t *pool;

int main(int argc, char* argv[]) {
  if(argc < 3) {
    std::cout << "Usage: proxy <build-script> <execute-script>" << std::endl;
    exit(1);
  }

  auto buildScriptPath = argv[1];
  auto executeScriptPath = argv[2];

  mongoc_init();
  //mongoc_uri_t *uri = mongoc_uri_new("mongodb://localhost:27017/?appname=prx");
  mongoc_uri_t *uri = mongoc_uri_new(std::getenv("MONGO_URL"));
  pool = mongoc_client_pool_new (uri);

  /**
   * INIT OBJECTS
   */
  /// INFRASTRUCTURE
  MongoMessageRepository messageRepository{pool};
  InMemoryDevicesRepository devicesRepository;
  MongoAnalysisRepository analysisRepository{pool};
  InMemoryRunRepository runRepository;

  /// SERVICES
  // BuildManager buildManager{buildScriptPath, analysisRepository};
  // RunManager runManager{executeScriptPath, devicesRepository};
  MessageService messageService{messageRepository};
  DevicesService devicesService{devicesRepository};
  // AnalysisService analysisService{buildManager, runManager, analysisRepository, runRepository};
  SocketManagerService socketManagerService{8081, 2, messageRepository, devicesRepository};
  socketManagerService.start();

  /// API
  DataEndpoint dataEndpoint{messageService};
  DevicesEndpoint devicesEndpoint{devicesService};
  // AnalysisEndpoint analysisEndpoint{analysisService};


  /**
   * INIT ENDPOINTS
   */
  httplib::Server handle;

  /// DATA
  addEndpoint<Response::MessageHeaderList>(handle, HTTPMethod::GET, NEWER_MESSAGES_ENDPOINT, ENDPOINT_MEMBER_FUNC(dataEndpoint, newerMessages));
  addEndpoint<Message>(handle, HTTPMethod::GET, GET_MESSAGE_ENDPOINT, ENDPOINT_MEMBER_FUNC(dataEndpoint, getMessage));

  /// DEVICES
  addEndpoint<Response::DeviceList>(handle, HTTPMethod::GET, AVAILABLE_DEVICES_ENDPOINT, ENDPOINT_MEMBER_FUNC(devicesEndpoint, getDevices));
  addEndpoint<void>(handle, HTTPMethod::GET, STOP_INSPECTION_ENDPOINT, ENDPOINT_MEMBER_FUNC(devicesEndpoint, unselectAll));
  addEndpoint<void>(handle, HTTPMethod::GET, SELECT_DEVICES_ENDPOINT, ENDPOINT_MEMBER_FUNC(devicesEndpoint, selectDevices));
  addEndpoint<void>(handle, HTTPMethod::GET, SELECT_ALL_ENDPOINT, ENDPOINT_MEMBER_FUNC(devicesEndpoint, selectAll));

  /// ANALYSIS
//  addEndpoint(handle, HTTPMethod::POST, IMPORT_ANALYSIS_ENDPOINT, ENDPOINT_MEMBER_FUNC(analysisEndpoint, importAnalysis));
//  addEndpoint(handle, HTTPMethod::GET, ANALYSIS_STATUS_ENDPOINT, ENDPOINT_MEMBER_FUNC(analysisEndpoint, getBuildStatus));
//  addEndpoint(handle, HTTPMethod::GET, LIST_WORKFLOWS_ENDPOINT, ENDPOINT_MEMBER_FUNC(analysisEndpoint, listWorkflows));
//  addEndpoint(handle, HTTPMethod::POST, START_ANALYSIS_ENDPOINT, ENDPOINT_MEMBER_FUNC(analysisEndpoint, startRun));
//  addEndpoint(handle, HTTPMethod::POST, STOP_ANALYSIS_ENDPOINT, ENDPOINT_MEMBER_FUNC(analysisEndpoint, stopRun));



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
  auto address = "0.0.0.0";
  auto port = 8082;
  std::cout << "STARTING PROXY ON " << address << ":" << port << std::endl;
  handle.listen(address, port);

  mongoc_uri_destroy(uri);
  mongoc_client_pool_destroy(pool);
  mongoc_cleanup();
}
