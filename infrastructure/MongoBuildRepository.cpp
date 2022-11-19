#include "infrastructure/MongoBuildRepository.h"
#include <stdexcept>

std::string MongoBuildRepository::save(const Build& build) {
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

  BSON_APPEND_UTF8(doc, "url", build.url.c_str());
  BSON_APPEND_UTF8(doc, "name", build.name.c_str());
  BSON_APPEND_UTF8(doc, "branch", build.branch.c_str());
  BSON_APPEND_UTF8(doc, "path", build.path.c_str());
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

std::vector<Build> MongoBuildRepository::getAnalyses(int page, int count) {
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

  std::vector<Build> analyses;
  char str_oid[25];
  while(mongoc_cursor_next(cursor, &doc))
  {
    Build build;
    bson_iter_init_find(&iter, doc, "_id");
    oid_ptr = bson_iter_oid(&iter);
    bson_oid_to_string(oid_ptr, str_oid);
    build.id = str_oid;

    bson_iter_init_find(&iter, doc, "url");
    build.url = bson_iter_utf8(&iter, NULL);

    bson_iter_init_find(&iter, doc, "name");
    build.name = bson_iter_utf8(&iter, NULL);

    bson_iter_init_find(&iter, doc, "branch");
    build.branch = bson_iter_utf8(&iter, NULL);

    bson_iter_init_find(&iter, doc, "path");
    build.path = bson_iter_utf8(&iter, NULL);

    analyses.push_back(build);
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

Build MongoBuildRepository::get(const std::string& buildId) {
  bson_oid_t oid;
  bson_iter_t iter;
  bson_t *query;
  const bson_t *doc;
  mongoc_cursor_t *cursor;
  mongoc_client_t *client;
  mongoc_collection_t *collection;
  struct Build build;

  client = mongoc_client_pool_pop (pool);
  collection = mongoc_client_get_collection(client, "diProxy", "analyses");
  
  query = bson_new();
  bson_oid_init_from_string(&oid, buildId.c_str());
  BSON_APPEND_OID(query, "_id", &oid);

  cursor = mongoc_collection_find_with_opts(
	collection, query, NULL, NULL);

  while(mongoc_cursor_next(cursor, &doc))
  {
    build.id = buildId.c_str();

    bson_iter_init_find(&iter, doc, "url");
    build.url = bson_iter_utf8(&iter, NULL);

    bson_iter_init_find(&iter, doc, "name");
    build.name = bson_iter_utf8(&iter, NULL);

    bson_iter_init_find(&iter, doc, "branch");
    build.branch = bson_iter_utf8(&iter, NULL);

    bson_iter_init_find(&iter, doc, "path");
    build.path = bson_iter_utf8(&iter, NULL);
  }
  bson_destroy(query);
  mongoc_cursor_destroy(cursor);
  mongoc_collection_destroy(collection);
  mongoc_client_pool_push (pool, client);
  return build;
}

std::optional<Build> MongoBuildRepository::getByName(const std::string& name) {
  const bson_oid_t* oid_ptr;
  bson_iter_t iter;
  bson_t *query;
  const bson_t *doc;
  mongoc_cursor_t *cursor;
  mongoc_client_t *client;
  mongoc_collection_t *collection;
  char str_oid[25];
  std::optional<Build> build;

  client = mongoc_client_pool_pop (pool);
  collection = mongoc_client_get_collection(client, "diProxy", "analyses");

  query = bson_new();
  BSON_APPEND_UTF8(query, "name", name.c_str());

  cursor = mongoc_collection_find_with_opts(collection, query, NULL, NULL);

  while(mongoc_cursor_next(cursor, &doc))
  {
    build.emplace();

    bson_iter_init_find(&iter, doc, "_id");
    oid_ptr = bson_iter_oid(&iter);
    bson_oid_to_string(oid_ptr, str_oid);
    build.value().id = str_oid;

    bson_iter_init_find(&iter, doc, "url");
    build.value().url = bson_iter_utf8(&iter, NULL);

    bson_iter_init_find(&iter, doc, "name");
    build.value().name = bson_iter_utf8(&iter, NULL);

    bson_iter_init_find(&iter, doc, "branch");
    build.value().branch = bson_iter_utf8(&iter, NULL);

    bson_iter_init_find(&iter, doc, "path");
    build.value().path = bson_iter_utf8(&iter, NULL);
  }
  bson_destroy(query);
  mongoc_cursor_destroy(cursor);
  mongoc_collection_destroy(collection);
  mongoc_client_pool_push (pool, client);
  return build;
}
