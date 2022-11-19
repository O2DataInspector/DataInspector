#include "infrastructure/MongoRunRepository.h"
#include <iostream>

std::string MongoRunRepository::save(const Run& run) {
  bson_t *doc;
  bson_oid_t oid;
  bson_error_t error;
  mongoc_client_t *client;
  mongoc_collection_t *collection;
  char str_oid[25];

  client = mongoc_client_pool_pop(pool);
  collection = mongoc_client_get_collection(client, "diProxy", "runs");
  doc = bson_new();

  bson_oid_init(&oid, NULL);
  BSON_APPEND_OID(doc, "_id", &oid);

  BSON_APPEND_UTF8(doc, "buildId", run.buildId.c_str());
  BSON_APPEND_UTF8(doc, "workflow", run.workflow.c_str());
  BSON_APPEND_UTF8(doc, "config", run.config.c_str());
  BSON_APPEND_INT32(doc, "status", static_cast<int32_t>(run.status));
  if(!mongoc_collection_insert_one(
          collection, doc, NULL, NULL, &error))
  {
    fprintf(stderr, "%s\n", error.message);
  }
  bson_destroy(doc);
  mongoc_collection_destroy(collection);

  mongoc_client_pool_push (pool, client);
  bson_oid_to_string(&oid, str_oid);
  std::string id = str_oid;
  return id;
}

Run MongoRunRepository::get(const std::string& runId) {
  bson_oid_t oid;
  bson_iter_t iter;
  bson_t *query;
  const bson_t *doc;
  mongoc_cursor_t *cursor;
  mongoc_client_t *client;
  mongoc_collection_t *collection;
  Run run;

  client = mongoc_client_pool_pop(pool);
  collection = mongoc_client_get_collection(client, "diProxy", "runs");

  query = bson_new();
  bson_oid_init_from_string(&oid, runId.c_str());
  BSON_APPEND_OID(query, "_id", &oid);

  cursor = mongoc_collection_find_with_opts(
          collection, query, NULL, NULL);

  while(mongoc_cursor_next(cursor, &doc))
  {
    run.id = runId;

    bson_iter_init_find(&iter, doc, "buildId");
    run.buildId = bson_iter_utf8(&iter, NULL);

    bson_iter_init_find(&iter, doc, "workflow");
    run.workflow = bson_iter_utf8(&iter, NULL);

    bson_iter_init_find(&iter, doc, "config");
    run.config = bson_iter_utf8(&iter, NULL);

    bson_iter_init_find(&iter, doc, "status");
    run.status = static_cast<Run::Status>(bson_iter_int32(&iter));
  }
  bson_destroy(query);
  mongoc_cursor_destroy(cursor);
  mongoc_collection_destroy(collection);
  mongoc_client_pool_push (pool, client);
  return run;
}

std::vector<Run> MongoRunRepository::listRuns() {
  const bson_oid_t* oid;
  bson_iter_t iter;
  bson_t *query;
  const bson_t *doc;
  mongoc_cursor_t *cursor;
  mongoc_client_t *client;
  mongoc_collection_t *collection;

  client = mongoc_client_pool_pop(pool);
  collection = mongoc_client_get_collection(client, "diProxy", "runs");

  query = bson_new();
  cursor = mongoc_collection_find_with_opts(
          collection, query, NULL, NULL);

  std::vector<Run> runs;
  char str_oid[25];
  while(mongoc_cursor_next(cursor, &doc))
  {
    Run run;
    bson_iter_init_find(&iter, doc, "buildId");
    run.buildId = bson_iter_utf8(&iter, NULL);

    bson_iter_init_find(&iter, doc, "workflow");
    run.workflow = bson_iter_utf8(&iter, NULL);

    bson_iter_init_find(&iter, doc, "config");
    run.config = bson_iter_utf8(&iter, NULL);

    bson_iter_init_find(&iter, doc, "status");
    run.status = static_cast<Run::Status>(bson_iter_int32(&iter));

    bson_iter_init_find(&iter, doc, "_id");
    oid = bson_iter_oid(&iter);
    bson_oid_to_string(oid, str_oid);
    run.id = str_oid;

    runs.emplace_back(run);
  }
  bson_destroy(query);
  mongoc_cursor_destroy(cursor);
  mongoc_collection_destroy(collection);
  mongoc_client_pool_push (pool, client);

  return runs;
}

void MongoRunRepository::updateStatus(const std::string& runId, Run::Status status) {
  bson_oid_t oid;
  bson_error_t error;
  bson_t *query;
  bson_t *update;
  mongoc_client_t *client;
  mongoc_collection_t *collection;

  client = mongoc_client_pool_pop(pool);
  collection = mongoc_client_get_collection(client, "diProxy", "runs");

  bson_oid_init_from_string(&oid, runId.c_str());
  query = BCON_NEW("_id", BCON_OID(&oid));

  update = BCON_NEW("$set",
                    "{",
                    "status",
                    BCON_INT32(static_cast<int32_t>(status)),
                    "}"
  );

  if(!mongoc_collection_update_one(
          collection, query, update, NULL, NULL, &error))
  {
    fprintf(stderr, "%s\n", error.message);
  }
  bson_destroy(update);
  bson_destroy(query);
  mongoc_collection_destroy(collection);
  mongoc_client_pool_push (pool, client);
}
