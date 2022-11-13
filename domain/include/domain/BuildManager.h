#ifndef DIPROXY_BUILDMANAGER_H
#define DIPROXY_BUILDMANAGER_H

#include "utils/ThreadPool.h"
#include "domain/AnalysisRepository.h"
#include <boost/asio.hpp>
#include <boost/process.hpp>

class BuildManager {
public:
  BuildManager(const std::string& scriptPath, AnalysisRepository& analysisRepository);
  void build(const std::string& analysisId, const std::string& url, const std::string& branch);

private:
  std::string scriptPath;
  AnalysisRepository& analysisRepository;

  //TODO: remove queue when parallel build will be supported
  struct BuildTask {
    std::string analysisId;
    std::string url;
    std::string branch;
  };
  std::deque<BuildTask> buildQueue;
  std::mutex buildQueueMutex;
  std::condition_variable cvBuildQueue;

  ThreadPool threadPool;
  boost::asio::io_context ioContext;

  struct BuildContext {
    std::string buff;
    boost::process::async_pipe pipe;
    boost::process::child process;
  };

  void startBuild(const BuildTask& task);
  void readUntilEOF(BuildContext* ctx, const std::string& analysisId);

  std::mutex ctxsMutex;
  std::unordered_map<std::string, BuildContext*> ctxs;
};

#endif //DIPROXY_BUILDMANAGER_H
