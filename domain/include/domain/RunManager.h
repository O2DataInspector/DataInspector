#ifndef DIPROXY_RUNMANAGER_H
#define DIPROXY_RUNMANAGER_H

#include "utils/ThreadPool.h"
#include <string>
#include "domain/model/Analysis.h"
#include "domain/model/Run.h"
#include "domain/DevicesRepository.h"
#include <boost/asio.hpp>
#include <boost/process.hpp>

class RunManager {
public:
  RunManager(const std::string& scriptPath, DevicesRepository& devicesRepository);
  void start(const Run& run, const Analysis& analysis);
  void stop(const std::string& runId);

private:
  void remove(const std::string& runId);

  std::string scriptPath;
  DevicesRepository& devicesRepository;

  ThreadPool threadPool;

  boost::asio::io_context ioContext;
  boost::asio::io_context::work work;

  std::mutex processesMutex;
  std::unordered_map<std::string, boost::process::child*> processes;
};

#endif //DIPROXY_RUNMANAGER_H
