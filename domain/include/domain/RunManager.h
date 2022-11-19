#ifndef DIPROXY_RUNMANAGER_H
#define DIPROXY_RUNMANAGER_H

#include "utils/ThreadPool.h"
#include <string>
#include "domain/model/Build.h"
#include "domain/model/Run.h"
#include "domain/DevicesRepository.h"
#include <boost/asio.hpp>
#include <boost/process.hpp>
#include "domain/RunRepository.h"

class RunManager {
public:
  RunManager(const std::string& scriptPath, DevicesRepository& devicesRepository, RunRepository& runRepository);
  void start(const Run& run, const Build& build, const std::string& dataset);
  void stop(const std::string& runId);

private:
  void remove(const std::string& runId);

  std::string scriptPath;
  DevicesRepository& devicesRepository;
  RunRepository& runRepository;

  ThreadPool threadPool;

  boost::asio::io_context ioContext;
  boost::asio::io_context::work work;

  std::mutex processesMutex;
  std::unordered_map<std::string, boost::process::child*> processes;
};

#endif //DIPROXY_RUNMANAGER_H
