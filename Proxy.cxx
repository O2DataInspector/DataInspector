#include <sstream>
#include <queue>

#include "httplib.h"
#include "boost/algorithm/string.hpp"

#include "api/DataEndpoint.h"
#include "api/DevicesEndpoint.h"
#include "api/BuildsEndpoint.h"
#include "api/RunsEndpoint.h"

#include "domain/MessageService.h"
#include "domain/DevicesService.h"
#include "domain/SocketManagerService.h"
#include "domain/BuildService.h"
#include "domain/RunManager.h"
#include "domain/RunsService.h"

#include "infrastructure/MongoMessageRepository.h"
#include "infrastructure/MongoDevicesRepository.h"
#include "infrastructure/MongoBuildRepository.h"
#include "infrastructure/MongoRunRepository.h"

#include "HTTPUtil.h"
#include "mongoc.h"
#include "domain/BuildDetector.h"

/* ENDPOINTS */
constexpr char AVAILABLE_DEVICES_ENDPOINT[] = "/available-devices";
constexpr char STOP_INSPECTION_ENDPOINT[]   = "/stop";
constexpr char SELECT_DEVICES_ENDPOINT[]    = "/select-devices";
constexpr char SELECT_ALL_ENDPOINT[]        = "/select-all";

constexpr char LIST_ANALYSES_ENDPOINT[]     = "/builds";
constexpr char LIST_WORKFLOWS_ENDPOINT[]    = "/builds/workflows";

constexpr char START_RUN_ENDPOINT[]         = "/runs";
constexpr char STOP_RUN_ENDPOINT[]          = "/runs/stop";
constexpr char LIST_DATASETS_ENDPOINT[]     = "/runs/datasets";
constexpr char LIST_RUNS_ENDPOINT[]         = "/runs";

constexpr char NEWER_MESSAGES_ENDPOINT[]    = "/messages/newer";
constexpr char GET_MESSAGE_ENDPOINT[]       = "/messages";
constexpr char STATS_ENDPOINT[]             = "/stats";

int main(int argc, char* argv[]) {
  if(argc < 3) {
    std::cout << "Usage: proxy <execute-script> <local-builds-path>" << std::endl;
    exit(1);
  }

  auto executeScriptPath = argv[1];
  auto localBuildsPath = argv[2];

  mongoc_init();
  auto* uri = mongoc_uri_new(std::getenv("MONGO_URL"));
  auto* pool = mongoc_client_pool_new (uri);

  /**
   * INIT OBJECTS
   */
  /// INFRASTRUCTURE
  MongoMessageRepository messageRepository{pool};
  MongoDevicesRepository devicesRepository{pool};
  MongoBuildRepository buildRepository{pool};
  MongoRunRepository runRepository{pool};

  /// SERVICES
  RunManager runManager{executeScriptPath, std::getenv("DI_DATASETS"), devicesRepository, runRepository};
  RunsService runsService{runManager, runRepository, buildRepository};
  MessageService messageService{messageRepository};
  DevicesService devicesService{devicesRepository};
  BuildService analysesService{buildRepository, std::string{std::getenv("WORK_DIR")} + std::getenv("ALIBUILD_ARCH_PREFIX")};
  SocketManagerService socketManagerService{8081, 2, messageRepository, devicesRepository, runRepository};
  socketManagerService.start();

  /// API
  DataEndpoint dataEndpoint{messageService};
  DevicesEndpoint devicesEndpoint{devicesService};
  BuildsEndpoint analysesEndpoint{analysesService};
  RunsEndpoint runsEndpoint{runsService};


  /**
   * INIT ENDPOINTS
   */
  httplib::Server handle;

  /// DATA
  addEndpoint<Response::MessageHeaderList>(handle, HTTPMethod::GET, NEWER_MESSAGES_ENDPOINT, ENDPOINT_MEMBER_FUNC(dataEndpoint, newerMessages));
  addEndpoint<Message>(handle, HTTPMethod::GET, GET_MESSAGE_ENDPOINT, ENDPOINT_MEMBER_FUNC(dataEndpoint, getMessage));
  addEndpoint<Stats>(handle, HTTPMethod::GET, STATS_ENDPOINT, ENDPOINT_MEMBER_FUNC(dataEndpoint, getStats));

  /// DEVICES
  addEndpoint<Response::DeviceList>(handle, HTTPMethod::GET, AVAILABLE_DEVICES_ENDPOINT, ENDPOINT_MEMBER_FUNC(devicesEndpoint, getDevices));
  addEndpoint<void>(handle, HTTPMethod::GET, STOP_INSPECTION_ENDPOINT, ENDPOINT_MEMBER_FUNC(devicesEndpoint, unselectAll));
  addEndpoint<void>(handle, HTTPMethod::GET, SELECT_DEVICES_ENDPOINT, ENDPOINT_MEMBER_FUNC(devicesEndpoint, selectDevices));
  addEndpoint<void>(handle, HTTPMethod::GET, SELECT_ALL_ENDPOINT, ENDPOINT_MEMBER_FUNC(devicesEndpoint, selectAll));

  /// ANALYSIS
  addEndpoint<Response::BuildList>(handle, HTTPMethod::GET, LIST_ANALYSES_ENDPOINT, ENDPOINT_MEMBER_FUNC(analysesEndpoint, getAnalyses));
  addEndpoint<Response::WorkflowList>(handle, HTTPMethod::GET, LIST_WORKFLOWS_ENDPOINT, ENDPOINT_MEMBER_FUNC(analysesEndpoint, listWorkflows));


  /// RUNS
  addEndpoint<Response::RunId>(handle, HTTPMethod::POST, START_RUN_ENDPOINT, ENDPOINT_MEMBER_FUNC(runsEndpoint, start));
  addEndpoint<void>(handle, HTTPMethod::POST, STOP_RUN_ENDPOINT, ENDPOINT_MEMBER_FUNC(runsEndpoint, stop));
  addEndpoint<Response::RunsList>(handle, HTTPMethod::GET, LIST_RUNS_ENDPOINT, ENDPOINT_MEMBER_FUNC(runsEndpoint, listRuns));
  addEndpoint<Response::DatasetList>(handle, HTTPMethod::GET, LIST_DATASETS_ENDPOINT, ENDPOINT_MEMBER_FUNC(runsEndpoint, listDatasets));


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


  BuildDetector detector{buildRepository, localBuildsPath, "O2Physics/O2"};
  detector.detectBuilds();

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
