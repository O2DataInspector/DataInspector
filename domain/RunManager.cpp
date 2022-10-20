#include "domain/RunManager.h"
#include <iostream>

RunManager::RunManager(const std::string& scriptPath, DevicesRepository& devicesRepository): scriptPath(scriptPath), devicesRepository(devicesRepository), threadPool(1), ioContext(), work(ioContext) {
  threadPool.addJob([this]() {
    ioContext.run();
  });
}

void RunManager::start(const Run& run, const Analysis& analysis, const std::string& config) {
  auto* process = new boost::process::child(
          boost::process::search_path("bash"),
          scriptPath,
          run.id,
          analysis.id,
          run.workflow,
          ioContext,
          boost::process::on_exit([this, run](int e, std::error_code ec) {
            std::cout << "RUN FINISHED" << std::endl;
            remove(run.id);
          }));

  processesMutex.lock();
  processes[run.id] = process;
  processesMutex.unlock();
}

void RunManager::remove(const std::string &runId) {
  processesMutex.lock();
  processes[runId]->join();
  delete processes[runId];
  processes.erase(runId);
  processesMutex.unlock();
}

void RunManager::stop(const std::string& runId) {
  processesMutex.lock();
  devicesRepository.terminate(runId);
  processesMutex.unlock();
}
