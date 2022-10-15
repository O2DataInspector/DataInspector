#ifndef DIPROXY_BUILDMANAGER_H
#define DIPROXY_BUILDMANAGER_H

#include "utils/ThreadPool.h"
#include "domain/AnalysisRepository.h"
#include <boost/asio.hpp>
#include <boost/process.hpp>

class BuildManager {
public:
  BuildManager(const std::string& scriptPath, AnalysisRepository& analysisRepository);
  void build(const std::string& analysisId, const std::string& path);

private:
  void remove(const std::string& analysisId);

  std::string scriptPath;
  AnalysisRepository& analysisRepository;

  ThreadPool threadPool;
  boost::asio::io_context ioContext;
  boost::asio::io_context::work work;

  struct BuildContext {
    std::string buff;
    boost::process::async_pipe pipe;
    boost::process::child process;
  };

  void readUntilEOF(BuildContext* ctx, const std::string& analysisId);

  std::mutex ctxsMutex;
  std::unordered_map<std::string, BuildContext*> ctxs;
};

#endif //DIPROXY_BUILDMANAGER_H
