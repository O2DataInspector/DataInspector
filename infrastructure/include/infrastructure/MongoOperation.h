#ifndef DIPROXY_MONGOOPERATION_H
#define DIPROXY_MONGOOPERATION_H

#include "mongoc.h"

class MongoOperation {
public:
  MongoOperation(mongoc_client_pool_t* pool,
                 const std::function<void(mongoc_client_t* client)>& operation,
                 const std::function<void()>& cleanup
                 ) : pool(pool), operation(operation), cleanup(cleanup) {};
  void exec() {
    mongoc_client_t* client = mongoc_client_pool_pop(pool);
    operation(client);
    mongoc_client_pool_push (pool, client);
  }

  ~MongoOperation() {
    cleanup();
  }

private:
  mongoc_client_pool_t* pool;
  std::function<void(mongoc_client_t* client)> operation;
  std::function<void()> cleanup;
};

#endif //DIPROXY_MONGOOPERATION_H
