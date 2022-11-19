#include "infrastructure/MongoAnalysisRepository.h"
#include <stdexcept>

std::string MongoAnalysisRepository::save(const Analysis& analysis) {
  bson_t *doc;
  bson_oid_t oid;
  bson_error_t error;
  mongoc_client_t *client;
  mongoc_collection_t *collection;
  char str_oid[25];

  client = mongoc_client_pool_pop(pool);
  collection = mongoc_client_get_collection(client, "diProxy", "analyses");
  doc = bson_new();

  bson_oid_init(&oid, NULL);
  BSON_APPEND_OID(doc, "_id", &oid);

  BSON_APPEND_UTF8(doc, "url", analysis.url.c_str());
  BSON_APPEND_UTF8(doc, "name", analysis.name.c_str());
  BSON_APPEND_UTF8(doc, "branch", analysis.branch.c_str());
  BSON_APPEND_UTF8(doc, "path", analysis.path.c_str());
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

std::vector<Analysis> MongoAnalysisRepository::getAnalyses(int page, int count) {
  bson_iter_t iter;
  bson_t *query;
  const bson_t *doc;
  const bson_oid_t *oid_ptr;
  mongoc_cursor_t *cursor;
  mongoc_client_t *client;
  mongoc_collection_t *collection;

  client = mongoc_client_pool_pop(pool);
  collection = mongoc_client_get_collection(client, "diProxy", "analyses");

  query = bson_new();
  auto* matchHeader = bson_new();
  auto* match = bson_new();
  auto* limit = bson_new();
  auto* skip = bson_new();
  BSON_APPEND_DOCUMENT(matchHeader, "$match", match);
  BSON_APPEND_DOCUMENT(query, "0", matchHeader);
  BSON_APPEND_INT64(skip, "$skip", page*count);
  BSON_APPEND_DOCUMENT(query, "1", skip);
  BSON_APPEND_INT64(limit, "$limit", count);
  BSON_APPEND_DOCUMENT(query, "2", limit);
  cursor = mongoc_collection_aggregate(collection, MONGOC_QUERY_NONE, query, NULL, NULL);

  std::vector<Analysis> analyses;
  char str_oid[25];
  while(mongoc_cursor_next(cursor, &doc))
  {
    Analysis analysis;
    bson_iter_init_find(&iter, doc, "_id");
    oid_ptr = bson_iter_oid(&iter);
    bson_oid_to_string(oid_ptr, str_oid);
    analysis.id = str_oid;

    bson_iter_init_find(&iter, doc, "url");
    analysis.url = bson_iter_utf8(&iter, NULL);

    bson_iter_init_find(&iter, doc, "name");
    analysis.name = bson_iter_utf8(&iter, NULL);

    bson_iter_init_find(&iter, doc, "branch");
    analysis.branch = bson_iter_utf8(&iter, NULL);

    bson_iter_init_find(&iter, doc, "path");
    analysis.path = bson_iter_utf8(&iter, NULL);

    analyses.push_back(analysis);
  }
  bson_destroy(match);
  bson_destroy(matchHeader);
  bson_destroy(skip);
  bson_destroy(limit);
  bson_destroy(query);
  mongoc_cursor_destroy(cursor);
  mongoc_collection_destroy(collection);
  mongoc_client_pool_push (pool, client);

  return analyses;
}

Analysis MongoAnalysisRepository::get(const std::string& analysisId) {
  bson_oid_t oid;
  bson_iter_t iter;
  bson_t *query;
  const bson_t *doc;
  mongoc_cursor_t *cursor;
  mongoc_client_t *client;
  mongoc_collection_t *collection;
  struct Analysis analysis;

  client = mongoc_client_pool_pop (pool);
  collection = mongoc_client_get_collection(client, "diProxy", "analyses");
  
  query = bson_new();
  bson_oid_init_from_string(&oid, analysisId.c_str());
  BSON_APPEND_OID(query, "_id", &oid);

  cursor = mongoc_collection_find_with_opts(
	collection, query, NULL, NULL);

  while(mongoc_cursor_next(cursor, &doc))
  {
    analysis.id = analysisId.c_str();

    bson_iter_init_find(&iter, doc, "url");
    analysis.url = bson_iter_utf8(&iter, NULL);

    bson_iter_init_find(&iter, doc, "name");
    analysis.name = bson_iter_utf8(&iter, NULL);

    bson_iter_init_find(&iter, doc, "branch");
    analysis.branch = bson_iter_utf8(&iter, NULL);

    bson_iter_init_find(&iter, doc, "path");
    analysis.path = bson_iter_utf8(&iter, NULL);
  }
  bson_destroy(query);
  mongoc_cursor_destroy(cursor);
  mongoc_collection_destroy(collection);
  mongoc_client_pool_push (pool, client);
  return analysis;
}

std::optional<Analysis> MongoAnalysisRepository::getByName(const std::string& name) {
  const bson_oid_t* oid_ptr;
  bson_iter_t iter;
  bson_t *query;
  const bson_t *doc;
  mongoc_cursor_t *cursor;
  mongoc_client_t *client;
  mongoc_collection_t *collection;
  char str_oid[25];
  std::optional<Analysis> analysis;

  client = mongoc_client_pool_pop (pool);
  collection = mongoc_client_get_collection(client, "diProxy", "analyses");

  query = bson_new();
  BSON_APPEND_UTF8(query, "name", name.c_str());

  cursor = mongoc_collection_find_with_opts(collection, query, NULL, NULL);

  while(mongoc_cursor_next(cursor, &doc))
  {
    analysis.emplace();

    bson_iter_init_find(&iter, doc, "_id");
    oid_ptr = bson_iter_oid(&iter);
    bson_oid_to_string(oid_ptr, str_oid);
    analysis.value().id = str_oid;

    bson_iter_init_find(&iter, doc, "url");
    analysis.value().url = bson_iter_utf8(&iter, NULL);

    bson_iter_init_find(&iter, doc, "name");
    analysis.value().name = bson_iter_utf8(&iter, NULL);

    bson_iter_init_find(&iter, doc, "branch");
    analysis.value().branch = bson_iter_utf8(&iter, NULL);

    bson_iter_init_find(&iter, doc, "path");
    analysis.value().path = bson_iter_utf8(&iter, NULL);
  }
  bson_destroy(query);
  mongoc_cursor_destroy(cursor);
  mongoc_collection_destroy(collection);
  mongoc_client_pool_push (pool, client);
  return analysis;
}
