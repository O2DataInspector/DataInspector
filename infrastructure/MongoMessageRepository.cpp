#include "infrastructure/MongoMessageRepository.h"
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <bson/bson.h>
#include <mongoc/mongoc.h>

extern mongoc_client_pool_t *pool;

std::string MongoMessageRepository::addMessage(const std::string& runId, const Message& message) {


  std::cout << "MongoMessageRepository::addMessage" << std::endl;

  bson_t *doc;
  bson_oid_t oid;
  bson_error_t error;
  bson_decimal128_t dec;
  mongoc_client_t *client;
  mongoc_collection_t *collection;

  char creat[25], str_oid[25];

  /* collection = mongoc_client_get_collection(client, "prx", runId.c_str()); */
  /* uncomment line above when specifying 'runId' header will be required */

  client = mongoc_client_pool_pop(pool);
  collection = mongoc_client_get_collection(client, "prx", "msg");

  doc = bson_new();

  bson_oid_init(&oid, NULL);
  BSON_APPEND_OID(doc, "_id", &oid);

  snprintf(creat, sizeof creat, "%llu", message.creationTimer);
  bson_decimal128_from_string(creat, &dec);
  BSON_APPEND_DECIMAL128(doc, "creationTimer", &dec);

  BSON_APPEND_UTF8(doc, "sender", message.sender.c_str());
  BSON_APPEND_INT32(doc, "duration", message.duration);
  BSON_APPEND_INT32(doc, "startTime", message.startTime);
  BSON_APPEND_UTF8(doc, "origin", message.origin.c_str());
  BSON_APPEND_UTF8(doc, "description", message.description.c_str());
  BSON_APPEND_INT32(doc, "subSpecification", message.subSpecification);
  BSON_APPEND_INT32(doc, "firstTForbit", message.firstTForbit);
  BSON_APPEND_INT32(doc, "tfCounter", message.tfCounter);
  BSON_APPEND_INT32(doc, "runNumber", message.runNumber);
  BSON_APPEND_INT32(doc, "payloadSize", message.payloadSize);
  BSON_APPEND_INT32(doc, "splitPayloadParts", message.splitPayloadParts);
  BSON_APPEND_UTF8(doc, "payloadSerialization", message.payloadSerialization.c_str());
  BSON_APPEND_INT32(doc, "payloadSplitIndex", message.payloadSplitIndex);
  BSON_APPEND_BOOL(doc, "read", false);
  BSON_APPEND_UTF8(doc, "payload", message.payload.c_str());

  if(!mongoc_collection_insert_one(
     collection, doc, NULL, NULL, &error))
  {
	fprintf(stderr, "%s\n", error.message);
  }
  bson_destroy(doc);
  mongoc_collection_destroy(collection);
  mongoc_client_pool_push(pool, client);

  bson_oid_to_string(&oid, str_oid);
  std::string id = str_oid;
  return id;
}

Message MongoMessageRepository::getMessage(const std::string& id) {
  bson_oid_t oid;
  bson_iter_t iter;
  bson_t *query;
  const bson_t *doc;
  bson_decimal128_t dec128;
  mongoc_cursor_t *cursor;
  mongoc_client_t *client;
  mongoc_collection_t *collection;

  struct Message message;

  char creat[25];

  /* collection = mongoc_client_get_collection(client, "prx", runId.c_str()); */
  /* uncomment line above when 'runId' header will be required */

  client = mongoc_client_pool_pop(pool);
  collection = mongoc_client_get_collection(client, "prx", "msg");
  
  query = bson_new();
  bson_oid_init_from_string(&oid, id.c_str());
  BSON_APPEND_OID(query, "_id", &oid);

  cursor = mongoc_collection_find_with_opts(
	collection, query, NULL, NULL);

  /* fixme: check wether bson_iter_utf8 allocates string on heap */
  if(mongoc_cursor_next(cursor, &doc))
  {
	message.id = id.c_str();	

	bson_iter_init_find(&iter, doc, "creationTimer");
	bson_iter_decimal128(&iter, &dec128);
	bson_decimal128_to_string(&dec128, creat);
	message.creationTimer = strtoul(creat, NULL, 10);

	bson_iter_init_find(&iter, doc, "sender");
	message.sender = bson_iter_utf8(&iter, NULL);

	bson_iter_init_find(&iter, doc, "duration");
	message.duration = bson_iter_int32(&iter);

  	bson_iter_init_find(&iter, doc, "startTime");
	message.startTime = bson_iter_int32(&iter);

  	bson_iter_init_find(&iter, doc, "origin");
	message.origin = bson_iter_utf8(&iter, NULL);

  	bson_iter_init_find(&iter, doc, "description");
	message.description = bson_iter_utf8(&iter, NULL);

  	bson_iter_init_find(&iter, doc, "subSpecification");
	message.subSpecification = bson_iter_int32(&iter);

  	bson_iter_init_find(&iter, doc, "firstTForbit");
	message.firstTForbit = bson_iter_int32(&iter);

  	bson_iter_init_find(&iter, doc, "tfCounter");
	message.tfCounter = bson_iter_int32(&iter);

  	bson_iter_init_find(&iter, doc, "runNumber");
	message.runNumber = bson_iter_int32(&iter);

  	bson_iter_init_find(&iter, doc, "payloadSize");
	message.payloadSize = bson_iter_int32(&iter);

  	bson_iter_init_find(&iter, doc, "splitPayloadParts");
	message.splitPayloadParts = bson_iter_int32(&iter);

  	bson_iter_init_find(&iter, doc, "payloadSerialization");
	message.payloadSerialization = bson_iter_utf8(&iter, NULL);

  	bson_iter_init_find(&iter, doc, "payloadSplitIndex");
	message.payloadSplitIndex = bson_iter_int32(&iter);

  	bson_iter_init_find(&iter, doc, "payload");
	message.payload = bson_iter_utf8(&iter, NULL);
  }
  else
  {
 	fprintf(stderr, "proxy: mongoc_cursor_next: no id %s in DB\n",
	        id.c_str());
	message.id = "-1";
  }
  bson_destroy(query);
  mongoc_collection_destroy(collection);
  mongoc_cursor_destroy(cursor);
  mongoc_client_pool_push(pool, client);

  return message;
}

std::vector<Message> MongoMessageRepository::newerMessages(const std::string& runId, const std::string& messageId, const std::vector<std::string>& devices, int count) {
  /* fixme: if count not given, return all of the content of db */
  /* fixme: add this to commit log
 * check added devices strings
 * in case when someone would like to inject code into database */
  /*
 * fixme: add this to commit log
 * overall query would look something like this:
 * (devices not included):
  pipeline = BCON_NEW(
    "0",
      "{",
        "$match",
               "{",
                 "_id",
		     "{",
                       "$gt",
                       BCON_OID(&oid),
                     "}",
	       "}",
      "}",
    "1",
      "{",
        "$sort",
	      "{",
		"_id",
		BCON_INT32(1),
	      "}",
      "}",
    "2",
      "{",
        "$limit",
        BCON_INT32(count),
      "}"
  );
  */
  bson_iter_t iter;
  bson_t *pipeline;
  bson_t pipe;
  bson_decimal128_t dec128;
  const bson_t *doc;
  const bson_oid_t *oid_ptr;
  mongoc_cursor_t *cursor;
  mongoc_client_t *client;
  mongoc_collection_t *collection;
  std::vector<Message> response{};
  std::vector<std::string> devices_copy = devices;
  struct Message messages[count];
  int idx = 0;
  size_t len = 0;

  char creat[25], str_oid[25], str_count[25];
  char *begin, *middle, *end, *braces, *sender;
  char *query;

  begin  = "{\"0\":{\"$match\":{\"_id\":{\"$gt\":{\"$oid\":\"";
  middle = "\"}},\"$or\":[";
  end    = "]}},\"1\":{\"$sort\":{\"_id\":1}},\"2\":{\"$limit\":";
  braces = "}}";
  sender = "{\"sender\":\"";

  for (auto device : devices) { 
	len += device.length() + 14;
	if(strpbrk(device.c_str(), "$:{},'\"") != NULL)
	{
		fprintf(stderr, "proxy: detected MQL injection: %s\n",
				device.c_str());
		messages[0].id = "-1";
		response.emplace_back(messages[0]);
		return response;
	}
  }

  /* collection = mongoc_client_get_collection(client, "prx", runId.c_str()); */
  /* uncomment line above when 'runId' header will be required */

  client = mongoc_client_pool_pop(pool);
  collection = mongoc_client_get_collection(client, "prx", "msg");

  snprintf(str_count, sizeof str_count, "%d", count);

  len += strlen(begin);
  len += sizeof str_oid;
  len += strlen(middle);
  len += strlen(end);
  len += sizeof str_count;
  len += strlen(braces);
  query = (char *)malloc(len * sizeof(char*));

  strcat(query, begin);
  strcat(query, messageId.c_str());
  strcat(query, middle);
  std::string back = devices_copy.back();
  devices_copy.pop_back();
  while(devices_copy.size() != 0) {
    std::string device = devices_copy.back();
    devices_copy.pop_back();
    strcat(query, sender);
    strcat(query, device.c_str());
    strcat(query, "\"},");
  }

  strcat(query, sender);
  strcat(query, back.c_str());
  strcat(query, "\"}");

  strcat(query, end);
  strcat(query, str_count);
  strcat(query, braces);

  pipeline = &pipe;
  bson_init_from_json(pipeline, query, -1, NULL);
  free(query);
  
  cursor = mongoc_collection_aggregate(
	collection, MONGOC_QUERY_NONE, pipeline, NULL, NULL);

  while(mongoc_cursor_next(cursor, &doc))
  {
    bson_iter_init_find(&iter, doc, "creationTimer");
    bson_iter_decimal128(&iter, &dec128);
    bson_decimal128_to_string(&dec128, creat);
    messages[idx].creationTimer = strtoul(creat, NULL, 10);

    bson_iter_init_find(&iter, doc, "sender");
    messages[idx].sender = bson_iter_utf8(&iter, NULL);

    bson_iter_init_find(&iter, doc, "duration");
    messages[idx].duration = bson_iter_int32(&iter);

    bson_iter_init_find(&iter, doc, "startTime");
    messages[idx].startTime = bson_iter_int32(&iter);

    bson_iter_init_find(&iter, doc, "origin");
    messages[idx].origin = bson_iter_utf8(&iter, NULL);

    bson_iter_init_find(&iter, doc, "description");
    messages[idx].description = bson_iter_utf8(&iter, NULL);

    bson_iter_init_find(&iter, doc, "subSpecification");
    messages[idx].subSpecification = bson_iter_int32(&iter);

    bson_iter_init_find(&iter, doc, "firstTForbit");
    messages[idx].firstTForbit = bson_iter_int32(&iter);

    bson_iter_init_find(&iter, doc, "tfCounter");
    messages[idx].tfCounter = bson_iter_int32(&iter);

    bson_iter_init_find(&iter, doc, "runNumber");
    messages[idx].runNumber = bson_iter_int32(&iter);

    bson_iter_init_find(&iter, doc, "payloadSize");
    messages[idx].payloadSize = bson_iter_int32(&iter);

    bson_iter_init_find(&iter, doc, "splitPayloadParts");
    messages[idx].splitPayloadParts = bson_iter_int32(&iter);

    bson_iter_init_find(&iter, doc, "payloadSerialization");
    messages[idx].payloadSerialization = bson_iter_utf8(&iter, NULL);

    bson_iter_init_find(&iter, doc, "payloadSplitIndex");
    messages[idx].payloadSplitIndex = bson_iter_int32(&iter);

    bson_iter_init_find(&iter, doc, "payload");
    messages[idx].payload = bson_iter_utf8(&iter, NULL);

    bson_iter_init_find(&iter, doc, "_id");
    oid_ptr = bson_iter_oid(&iter);
    bson_oid_to_string(oid_ptr, str_oid);
    messages[idx].id = str_oid;

    response.emplace_back(messages[idx]);
    idx++;
  }
  mongoc_collection_destroy(collection);
  mongoc_cursor_destroy(cursor);
  mongoc_client_pool_push (pool, client);

  return response;
}
