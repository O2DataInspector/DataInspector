#include "infrastructure/MongoAnalysisRepository.h"
#include <stdexcept>
#include <iostream>

std::string MongoAnalysisRepository::save(const Analysis& analysis) {
  bson_t *doc;
  bson_t child;
  bson_oid_t oid;
  bson_error_t error;
  mongoc_client_t *client;
  mongoc_collection_t *collection;
  char str_oid[25];
  char key[200];
  int i = 0;
  std::vector<std::string> logs_copy = analysis.logs;

  client = mongoc_client_pool_pop(pool);
  collection = mongoc_client_get_collection(client, "diProxy", "analyses");
  doc = bson_new();

  bson_oid_init(&oid, NULL);
  BSON_APPEND_OID(doc, "_id", &oid);

  BSON_APPEND_UTF8(doc, "url", analysis.url.c_str());
  BSON_APPEND_UTF8(doc, "name", analysis.name.c_str());
  BSON_APPEND_UTF8(doc, "branch", analysis.branch.c_str());
  BSON_APPEND_INT32(doc, "buildStatus", (int) analysis.buildStatus);
  BSON_APPEND_ARRAY_BEGIN(doc, "logs", &child);
  while(logs_copy.size() != 0)
  {
    snprintf(key, sizeof key, "%d", i);
    BSON_APPEND_UTF8(&child, key, logs_copy.back().c_str());
    logs_copy.pop_back();
    i++;
  }
  bson_append_array_end(doc, &child);
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
  bson_iter_t child;
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

    bson_iter_init_find(&iter, doc, "buildStatus");
    analysis.buildStatus = (Analysis::BuildStatus) bson_iter_int32(&iter);

    std::vector<std::string> queue;

    bson_iter_init_find (&iter, doc, "logs");
    bson_iter_recurse (&iter, &child);
    while (bson_iter_next (&child)) {
      std::string tmp = bson_iter_utf8(&child, NULL);
      queue.emplace_back(tmp);
    }
    analysis.logs = queue;

    bson_iter_init_find(&iter, doc, "url");
    analysis.url = bson_iter_utf8(&iter, NULL);

    bson_iter_init_find(&iter, doc, "name");
    analysis.name = bson_iter_utf8(&iter, NULL);

    bson_iter_init_find(&iter, doc, "branch");
    analysis.branch = bson_iter_utf8(&iter, NULL);

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
  bson_iter_t child;
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

	bson_iter_init_find(&iter, doc, "buildStatus");
	analysis.buildStatus = (Analysis::BuildStatus) bson_iter_int32(&iter);

	std::vector<std::string> queue;

        bson_iter_init_find (&iter, doc, "logs");
        bson_iter_recurse (&iter, &child);
   	while (bson_iter_next (&child)) {
      		std::string tmp = bson_iter_utf8(&child, NULL);
      		queue.emplace_back(tmp);
   	}
        analysis.logs = queue;

	bson_iter_init_find(&iter, doc, "url");
	analysis.url = bson_iter_utf8(&iter, NULL);

	bson_iter_init_find(&iter, doc, "name");
	analysis.name = bson_iter_utf8(&iter, NULL);

  bson_iter_init_find(&iter, doc, "branch");
  analysis.branch = bson_iter_utf8(&iter, NULL);
  }
  bson_destroy(query);
  mongoc_cursor_destroy(cursor);
  mongoc_collection_destroy(collection);
  mongoc_client_pool_push (pool, client);
  return analysis;
}

void MongoAnalysisRepository::appendLogs(const std::string& analysisId, const std::vector<std::string>& logs) {
  std::cout << "MongoAnalysisRepository::appendLogs - NOT IMPLEMENTED" << std::endl;
//  bson_oid_t oid;
//  bson_iter_t iter;
//  bson_iter_t child_iter;
//  bson_error_t error;
//  bson_t *query;
//  bson_t child;
//  const bson_t *doc;
//  bson_t *docc;
//  mongoc_cursor_t *cursor;
//  mongoc_client_t *client;
//  mongoc_collection_t *collection;
//  std::vector<std::string> logs_copy = logs;
//  std::string tmp;
//  int i = 0;
//  char key[200];
//
//  client = mongoc_client_pool_pop (pool);
//  collection = mongoc_client_get_collection(client, "diProxy", "analyses");
//
//  bson_oid_init_from_string(&oid, analysisId.c_str());
//  query = BCON_NEW("_id", BCON_OID(&oid));
//
//  Analysis analysis = MongoAnalysisRepository::get(analysisId);
//
//  cursor = mongoc_collection_find_with_opts(
//	collection, query, NULL, NULL);
//
//  if(mongoc_cursor_next(cursor, &doc))
//  {
//  	bson_iter_init_find (&iter, doc, "logs");
//  	bson_iter_recurse (&iter, &child_iter);
//  	while (bson_iter_next (&child_iter)) {
//      		tmp = bson_iter_utf8(&child_iter, NULL);
//      		logs_copy.emplace_back(tmp);
//  	}
//  	docc = bson_new();
//  	BSON_APPEND_OID(docc, "_id", &oid);
//
//  	BSON_APPEND_UTF8(docc, "url", analysis.url.c_str());
//  	BSON_APPEND_UTF8(docc, "name", analysis.name.c_str());
//    BSON_APPEND_UTF8(docc, "branch", analysis.branch.c_str());
//  	BSON_APPEND_INT32(docc, "buildStatus", (int) analysis.buildStatus);
//  	BSON_APPEND_ARRAY_BEGIN(docc, "logs", &child);
//  	while(logs_copy.size() != 0)
//  	{
//    		snprintf(key, sizeof key, "%d", i);
//    		BSON_APPEND_UTF8(&child, key, logs_copy.back().c_str());
//    		logs_copy.pop_back();
//    		i++;
//  	}
//  	bson_append_array_end(docc, &child);
//  	if(!mongoc_collection_insert_one(
//     	    collection, docc, NULL, NULL, &error))
//	/* fixme: update instead of insert */
//  	{
//		fprintf(stderr, "%s\n", error.message);
//  	}
//        bson_destroy(docc);
//  }
//  else
//  {
//	fprintf(stderr, "haven't found proper id\n");
//  }
//  bson_destroy(query);
//  mongoc_cursor_destroy(cursor);
//  mongoc_client_pool_push (pool, client);
}

void MongoAnalysisRepository::updateStatus(const std::string& analysisId, Analysis::BuildStatus status) {
  bson_oid_t oid;
  bson_error_t error;
  bson_t *query;
  bson_t *update;
  mongoc_client_t *client;
  mongoc_collection_t *collection;

  client = mongoc_client_pool_pop (pool);
  collection = mongoc_client_get_collection(client, "diProxy", "analyses");

  bson_oid_init_from_string(&oid, analysisId.c_str());
  query = BCON_NEW("_id", BCON_OID(&oid));

  update = BCON_NEW("$set", 
  		          "{",
			     "buildStatus",
			     BCON_INT32((int)status),
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
