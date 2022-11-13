#include "domain/BuildManager.h"
#include <iostream>

BuildManager::BuildManager(const std::string& scriptPath, AnalysisRepository &analysisRepository) : scriptPath(scriptPath), analysisRepository(analysisRepository), threadPool(1), ioContext() {
  threadPool.addJob([this]() {
    while (true) {
      BuildTask task;
      {
        std::unique_lock lk(buildQueueMutex);
        cvBuildQueue.wait(lk, [this]{return !buildQueue.empty();});

        task = buildQueue.front();
        buildQueue.pop_front();
      }

      startBuild(task);
      ioContext.run();
    }
  });
}

void BuildManager::readUntilEOF(BuildContext* ctx, const std::string& analysisId) {
  boost::asio::async_read_until(ctx->pipe, boost::asio::dynamic_buffer(ctx->buff), '\n', [this,analysisId,ctx](const boost::system::error_code& ec, std::size_t sz) {
    if(ec.failed()) {
      std::cout << "ERROR: " << ec.message() << std::endl;
      return;
    }

    std::cout << "READ (" + std::to_string(sz) + ")" << std::endl;
    analysisRepository.appendLogs(analysisId, {ctx->buff});
    ctx->buff.clear();
    readUntilEOF(ctx, analysisId);
  });
}

void BuildManager::startBuild(const BuildTask& task) {
  analysisRepository.updateStatus(task.analysisId, Analysis::BuildStatus::IN_PROGRESS);

  auto* ctx = new BuildContext{
          .pipe = boost::process::async_pipe(ioContext)
  };

  ctx->process = boost::process::child(
          boost::process::search_path("bash"),
          scriptPath,
          task.url,
          task.branch,
          task.analysisId,
          boost::process::std_out > ctx->pipe,
          ioContext,
          boost::process::on_exit([this, task](int e, std::error_code ec) {
            std::cout << "BUILD FINISHED: " << e << std::endl;

            if(e == 0) {
              analysisRepository.updateStatus(task.analysisId, Analysis::BuildStatus::OK);
            } else {
              analysisRepository.updateStatus(task.analysisId, Analysis::BuildStatus::ERROR);
            }

            //remove current build context
            ctxsMutex.lock();
            delete ctxs[task.analysisId];
            ctxs.erase(task.analysisId);
            ctxsMutex.unlock();
          }));

  readUntilEOF(ctx, task.analysisId);

  ctxsMutex.lock();
  ctxs[task.analysisId] = ctx;
  ctxsMutex.unlock();
}

void BuildManager::build(const std::string& analysisId, const std::string& url, const std::string& branch) {
  {
    std::unique_lock lk(buildQueueMutex);
    buildQueue.push_back(BuildTask{analysisId, url, branch});
  }
  cvBuildQueue.notify_one();
}
