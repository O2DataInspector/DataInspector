#include "infrastructure/InMemoryMessageRepository.h"
#include "api/DataEndpoint.h"
#include <iostream>
#include <algorithm>

#include <mongoc/mongoc.h>
#include <bson/bson.h>
#include <string.h>

#define MONGODB_SWITCH
//#undef MONGODB_SWITCH
#ifdef MONGODB_SWITCH
#define INIT_BUFF_SIZE 20
#endif

extern mongoc_client_t *client;

std::string InMemoryMessageRepository::addMessage(const std::string& runId, const Message& message) {

#ifndef MONGODB_SWITCH
  messageMutex.lock();
#endif
  std::cout << "MessageRepository::addMessage" << std::endl;

#ifndef MONGODB_SWITCH
  if(messages.count(runId) == 0)
    messages[runId] = std::deque<Message>{};

  auto id = message.id;
  if(id.empty())
    id = std::to_string(count++);

  messages[runId].emplace_back(message);
  (--messages[runId].end())->id = id;

  messageMutex.unlock();

  return id;
#endif

#ifdef MONGODB_SWITCH
  bson_t *doc;
  bson_oid_t oid;
  bson_error_t error;
  bson_decimal128_t dec;
  mongoc_collection_t *collection;

  char *orig, *json;
  char *orig_idx, *json_idx;
  char creat[25], str_oid[25];

  /* collection = mongoc_client_get_collection(client, "prx-db", runId.c_str()); */
  /* uncomment line above when specifying 'runId' header will be required */

  collection = mongoc_client_get_collection(client, "prx-db", "test");

  orig = strdup(toJson(message).c_str());
  json = strdup(toJson(message).c_str());
  orig_idx = strstr(orig, "creationTimer");
  json_idx = strstr(json, "creationTimer");

  sprintf(creat, "%llu", message.creationTimer);

  strcpy(json_idx-1-1, orig_idx+15+strlen(creat));

  /* strlen("creationTimer':") == 15 */
  /* fixme: handle cases when creationTimer is first or last prop */

  doc = bson_new_from_json((const unsigned char *) json, -1, &error);
  if(doc == NULL)
	fprintf(stderr, "bson_new_from_json: %s\n", error.message);

  bson_decimal128_from_string(creat, &dec);
  BSON_APPEND_DECIMAL128(doc, "creationTimer", &dec);
  BSON_APPEND_BOOL(doc, "read", false);

  bson_oid_init(&oid, NULL);
  BSON_APPEND_OID(doc, "_id", &oid);

  if(!mongoc_collection_insert_one(
     collection, doc, NULL, NULL, &error))
  {
	fprintf(stderr, "%s\n", error.message);
  }
  bson_destroy(doc);
  free(orig);
  free(json);
  mongoc_collection_destroy(collection);

  bson_oid_to_string(&oid, str_oid);
  std::string id = str_oid;
  return id;
#endif
}

Message InMemoryMessageRepository::getMessage(const std::string& id) {
  messageMutex.lock();
  std::cout << "MessageRepository::getMessage" << std::endl;

  for(auto& run : messages) {
    for(auto& message : run.second) {
      if(message.id == id) {
        auto response = message;
        messageMutex.unlock();
        return response;
      }
    }
  }

  messageMutex.unlock();
  throw std::runtime_error("Id not found");
}

std::vector<Message> InMemoryMessageRepository::getOldestMessages(const std::string& runId, int count) {
/* fixme: handle case when count is not given. error stoi: */
#ifndef MONGODB_SWITCH
  std::vector<Message> response{};
  messageMutex.lock();
#endif
  std::cout << "MessageRepository::getOldestMessages" << std::endl;

#ifndef MONGODB_SWITCH
  auto& analysisMessages = messages[runId];

  if(analysisMessages.empty()) {
    std::cout << "EMPTY" << std::endl;
    messageMutex.unlock();
    return response;
  }

  uint64_t realCount = std::min((uint64_t) analysisMessages.size(), (uint64_t) count);
  for(int i=0; i<realCount; i++) {
    response.emplace_back(analysisMessages.front());
    analysisMessages.pop_front();
  }

  std::cout << "SIZE - " << response.size() << std::endl;

  messageMutex.unlock();

  return response;
#endif

#ifdef MONGODB_SWITCH
  bson_oid_t oid;
  bson_iter_t iter;
  bson_error_t error;
  bson_t *query;
  bson_t *update;
  bson_t *pipeline;
  bson_t *bson_payload;
  bson_decimal128_t dec128;
  const bson_t *doc;
  const bson_value_t *value;
  mongoc_cursor_t *cursor;
  mongoc_collection_t *collection;

  struct Message messages[count];
  std::vector<Message> response{};

  const uint8_t *data;
  uint32_t len;
  int idx = 0;

  char *payload;
  char creat[25], str_oid[25];

  /* collection = mongoc_client_get_collection(client, "prx-db", runId.c_str()); */
  /* uncomment line above when 'runId' header will be required */

  collection = mongoc_client_get_collection(client, "prx-db", "test");

  pipeline = BCON_NEW(
    "0",
      "{",
         "$match",
                "{",
                   "read",
                   BCON_BOOL(false),
                "}",
      "}",
    "1",
      "{",
         "$sort",
               "{",
                  "startTime",
                  BCON_INT32(1),
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
  /* fixme: replace BCON_NEW with proper function invokations */

  cursor = mongoc_collection_aggregate(
	collection, MONGOC_QUERY_NONE, pipeline, NULL, NULL);

  while(mongoc_cursor_next(cursor, &doc))
  {
	bson_iter_init_find(&iter, doc, "creationTimer");
	bson_iter_decimal128(&iter, &dec128);
	bson_decimal128_to_string(&dec128, creat);
	messages[idx].creationTimer = strtoul(creat, NULL, 10);

	bson_iter_init_find(&iter, doc, "sender");
	messages[idx].sender = bson_iter_utf8(&iter, &len);

	bson_iter_init_find(&iter, doc, "duration");
	messages[idx].duration = bson_iter_int32(&iter);

  	bson_iter_init_find(&iter, doc, "startTime");
	messages[idx].startTime = bson_iter_int32(&iter);

  	bson_iter_init_find(&iter, doc, "origin");
	messages[idx].origin = bson_iter_utf8(&iter, &len);

  	bson_iter_init_find(&iter, doc, "description");
	messages[idx].description = bson_iter_utf8(&iter, &len);

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
	messages[idx].payloadSerialization = bson_iter_utf8(&iter, &len);

  	bson_iter_init_find(&iter, doc, "payloadSplitIndex");
	messages[idx].payloadSplitIndex = bson_iter_int32(&iter);

  	bson_iter_init_find(&iter, doc, "payload");
	bson_iter_document(&iter, &len, &data);
	bson_payload = bson_new_from_data(data, (size_t) len);
	payload = bson_as_relaxed_extended_json(bson_payload, NULL);
	messages[idx].payload = payload;
	free(payload);
	bson_destroy(bson_payload);

	/* without init there are multiple errors */
	if(bson_iter_init(&iter, doc)
	&& bson_iter_find(&iter, "_id")
	&& BSON_ITER_HOLDS_OID(&iter))
	{
		value = bson_iter_value(&iter);
		bson_oid_copy(&(value->value.v_oid), &oid);
	}
	else
		fprintf(stderr, "%s\n",
			"error while searching _id in BSON");

	bson_oid_to_string(&oid, str_oid);
	messages[idx].id = str_oid;

	response.emplace_back(messages[idx]);
	idx++;

	query = BCON_NEW("_id", BCON_OID(&oid));

	update = BCON_NEW("$set", 
			        "{",
				   "read",
				   BCON_BOOL(true),
				"}"
			);

	if(!mongoc_collection_update_one(
			collection, query, update, NULL, NULL, &error))
	{
		fprintf(stderr, "%s\n", error.message);
	}
  }
  bson_destroy(pipeline);
  mongoc_collection_destroy(collection);

  return response;
#endif
}

std::vector<std::string> InMemoryMessageRepository::newerMessages(const std::string& runId, uint64_t time, const std::vector<std::string>& devices, int count) {
  std::vector<std::string> response{};
  messageMutex.lock();
  std::cout << "MessageRepository::newerMessages" << std::endl;

  auto& analysisMessages = messages[runId];

  if(analysisMessages.empty()) {
    std::cout << "EMPTY" << std::endl;
    messageMutex.unlock();
    return response;
  }

  uint64_t realCount = std::min((uint64_t) analysisMessages.size(), (uint64_t) count);
  for(auto& msg : analysisMessages) {
    if(msg.creationTimer > time && std::find(devices.begin(), devices.end(), msg.sender) != devices.end()) {
      response.push_back(msg.id);
      realCount--;

      if(realCount == 0)
        break;
    }
  }

  messageMutex.unlock();

  return response;
}
