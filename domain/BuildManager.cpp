#include "domain/BuildManager.h"
#include <iostream>

BuildManager::BuildManager(const std::string& scriptPath, AnalysisRepository &analysisRepository) : scriptPath(scriptPath), analysisRepository(analysisRepository), threadPool(2), ioContext(),
                                                                     work(ioContext) {
  threadPool.addJob([this]() {
    ioContext.run();
  });

  threadPool.addJob([this]() {
    ioContext.run();
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

void BuildManager::build(const std::string &analysisId, const std::string &path) {
  auto* ctx = new BuildContext{
    .pipe = boost::process::async_pipe(ioContext)
  };

  ctx->process = boost::process::child(
          boost::process::search_path("bash"),
          scriptPath,
          path,
          analysisId,
          boost::process::std_out > ctx->pipe,
          ioContext,
          boost::process::on_exit([this, analysisId](int e, std::error_code ec) {
            std::cout << "BUILD FINISHED: " << e << std::endl;

            if(e == 0) {
              analysisRepository.updateStatus(analysisId, Analysis::BuildStatus::OK);
            } else {
              analysisRepository.updateStatus(analysisId, Analysis::BuildStatus::ERROR);
            }

            remove(analysisId);
          }));

  readUntilEOF(ctx, analysisId);

  ctxsMutex.lock();
  ctxs[analysisId] = ctx;
  ctxsMutex.unlock();
}

void BuildManager::remove(const std::string &analysisId) {
  ctxsMutex.lock();
  delete ctxs[analysisId];
  ctxs.erase(analysisId);
  ctxsMutex.unlock();
}
