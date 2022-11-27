#include "infrastructure/MongoMessageRepository.h"
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <bson/bson.h>
#include <mongoc/mongoc.h>
#include "infrastructure/MongoOperation.h"
#include <boost/algorithm/string.hpp>

std::string MongoMessageRepository::addMessage(const std::string& runId, const Message& message) {
  std::cout << "MongoMessageRepository::addMessage" << std::endl;

  bson_t *doc;
  bson_oid_t oid;
  mongoc_collection_t *collection;
  char str_oid[25];

  MongoOperation mongoOp{
    pool,
    [&collection, &doc, &oid, &message, &runId](mongoc_client_t* client) {
      bson_error_t error;
      bson_decimal128_t dec;
      char creat[25];

      collection = mongoc_client_get_collection(client, "diProxy", "messages");

      doc = bson_new();

      bson_oid_init(&oid, NULL);
      BSON_APPEND_OID(doc, "_id", &oid);

      snprintf(creat, sizeof creat, "%llu", message.creationTimer);
      bson_decimal128_from_string(creat, &dec);
      BSON_APPEND_DECIMAL128(doc, "creationTimer", &dec);

      BSON_APPEND_UTF8(doc, "runId", runId.c_str());
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

      if(!mongoc_collection_insert_one(collection, doc, NULL, NULL, &error))
        throw MessageNotSaved{error.message};
    },
    [&doc, &collection]() {
      bson_destroy(doc);
      mongoc_collection_destroy(collection);
    }
  };

  mongoOp.exec();

  bson_oid_to_string(&oid, str_oid);
  std::string id = str_oid;
  return id;
}

Message MongoMessageRepository::getMessage(const std::string& id) {
  bson_t *query;
  const bson_t *doc;
  mongoc_cursor_t *cursor;
  mongoc_collection_t *collection;
  Message message;

  MongoOperation mongoOp{
    pool,
    [&collection, &query, &cursor, &doc, &message, &id](mongoc_client_t* client) {
      bson_oid_t oid;
      bson_iter_t iter;
      bson_decimal128_t dec128;
      char creat[25];

      collection = mongoc_client_get_collection(client, "diProxy", "messages");

      query = bson_new();
      bson_oid_init_from_string(&oid, id.c_str());
      BSON_APPEND_OID(query, "_id", &oid);

      cursor = mongoc_collection_find_with_opts(
              collection, query, NULL, NULL);

      if(mongoc_cursor_next(cursor, &doc)) {
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
      } else {
        throw MessageNotFound{"Message not found for id=" + id};
      }
    },
    [&query, &collection, &cursor]() {
      bson_destroy(query);
      mongoc_collection_destroy(collection);
      mongoc_cursor_destroy(cursor);
    }
  };

  mongoOp.exec();

  return message;
}

std::vector<Message> MongoMessageRepository::newerMessages(const std::string& runId, const std::string& messageId, const std::vector<std::string>& devices, int count) {
  std::cout << "MongoMessageRepository::newerMessages" << std::endl;
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

  char creat[25], str_oid[25];

  for (auto& device : devices) {
    if(strpbrk(device.c_str(), "$:{},'\"") != nullptr){
      throw std::runtime_error("proxy: detected MQL injection: " + device);
    }
  }

  std::string query;
  query += R"({"0":{"$match":{)";
  if(!messageId.empty()) {
    query += R"("_id":{"$gt":{"$oid":")" + messageId + R"("}},)";
  }
  query += R"("runId": ")" + runId + R"(",)";
  query += R"("$or":[)";
  for(auto it = devices.begin(); it != devices.end(); it++) {
    query += R"({"sender": ")" + *it + R"("})";
    if((it+1) != devices.end())
      query += ",";
  }
  query += "]";
  query += "}},";
  query += R"("1":{"$sort":{"_id":1}},)";
  query += R"("2":{"$limit":)" + std::to_string(count) + "}";
  query += "}";

  client = mongoc_client_pool_pop(pool);
  collection = mongoc_client_get_collection(client, "diProxy", "messages");

  pipeline = &pipe;
  bson_init_from_json(pipeline, query.c_str(), -1, NULL);
  
  cursor = mongoc_collection_aggregate(collection, MONGOC_QUERY_NONE, pipeline, NULL, NULL);

  while(mongoc_cursor_next(cursor, &doc))
  {
    Message message;

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

    bson_iter_init_find(&iter, doc, "_id");
    oid_ptr = bson_iter_oid(&iter);
    bson_oid_to_string(oid_ptr, str_oid);
    message.id = str_oid;

    response.emplace_back(message);
  }
  mongoc_collection_destroy(collection);
  mongoc_cursor_destroy(cursor);
  mongoc_client_pool_push (pool, client);

  return response;
}

bson_t *paramQuery(const Range<uint64_t>& value) {
  auto* query = bson_new();

  if(value.begin.has_value()) {
    bson_decimal128_t v;
    bson_decimal128_from_string(std::to_string(value.begin.value()).c_str(), &v);
    BSON_APPEND_DECIMAL128(query, "$gte", &v);
  }

  if(value.end.has_value()) {
    bson_decimal128_t v;
    bson_decimal128_from_string(std::to_string(value.end.value()).c_str(), &v);
    BSON_APPEND_DECIMAL128(query, "$lte", &v);
  }

  return query;
}

std::vector<Message> MongoMessageRepository::search(const StatsRequest& request) {
  std::vector<bson_t*> queries{};

  auto* query = bson_new();

  BSON_APPEND_UTF8(query, "runId", request.runId.c_str());
  if(request.device.has_value())
    BSON_APPEND_UTF8(query, "sender", request.device.value().c_str());
  if(request.messageOrigin.has_value())
    BSON_APPEND_UTF8(query, "origin", request.messageOrigin.value().c_str());
  if(request.description.has_value())
    BSON_APPEND_UTF8(query, "description", request.description.value().c_str());

  if(request.subSpecification.has_value())
    BSON_APPEND_INT64(query, "subSpecification", request.subSpecification.value());
  if(request.firstTForbit.has_value())
    BSON_APPEND_INT64(query, "firstTForbit", request.firstTForbit.value());
  if(request.tfCounter.has_value())
    BSON_APPEND_INT64(query, "tfCounter", request.tfCounter.value());
  if(request.runNumber.has_value())
    BSON_APPEND_INT64(query, "runNumber", request.runNumber.value());
  if(request.taskHash.has_value())
    BSON_APPEND_UTF8(query, "taskHash", request.taskHash.value().c_str());
  if(request.payloadSerialization.has_value())
    BSON_APPEND_UTF8(query, "payloadSerialization", request.payloadSerialization.value().c_str());
  if(request.payloadParts.has_value())
    BSON_APPEND_INT64(query, "payloadParts", request.payloadParts.value());
  if(request.payloadSplitIndex.has_value())
    BSON_APPEND_INT64(query, "payloadSplitIndex", request.payloadSplitIndex.value());

  if(request.StartTimeRange.has_value()) {
    auto* q = paramQuery(request.StartTimeRange.value());
    BSON_APPEND_DOCUMENT(query, "startTime", q);
    queries.push_back(q);
  }
  if(request.CreationTimeRange.has_value()) {
    auto* q = paramQuery(request.CreationTimeRange.value());
    BSON_APPEND_DOCUMENT(query, "creationTimer", q);
    queries.push_back(q);
  }
  if(request.DurationRange.has_value()) {
    auto* q = paramQuery(request.DurationRange.value());
    BSON_APPEND_DOCUMENT(query, "duration", q);
    queries.push_back(q);
  }
  if(request.PayloadSizeRange.has_value()) {
    auto* q = paramQuery(request.PayloadSizeRange.value());
    BSON_APPEND_DOCUMENT(query, "payloadSize", q);
    queries.push_back(q);
  }

  bson_t* opts = NULL;
  if(request.count.has_value())
    opts = BCON_NEW ("limit", BCON_INT64(request.count.value()), "sort", "{", "_id", BCON_INT32(-1), "}");

  size_t sz;
  std::cout << bson_as_canonical_extended_json(query, &sz) << std::endl;

  bson_iter_t iter;
  bson_decimal128_t dec128;
  const bson_t *doc;
  const bson_oid_t *oid_ptr;
  mongoc_cursor_t *cursor;
  mongoc_client_t *client;
  mongoc_collection_t *collection;
  std::vector<Message> response{};

  char creat[25], str_oid[25];

  client = mongoc_client_pool_pop(pool);
  collection = mongoc_client_get_collection(client, "diProxy", "messages");

  cursor = mongoc_collection_find_with_opts(collection, query, opts, NULL);

  while(mongoc_cursor_next(cursor, &doc))
  {
    Message message;

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

    bson_iter_init_find(&iter, doc, "_id");
    oid_ptr = bson_iter_oid(&iter);
    bson_oid_to_string(oid_ptr, str_oid);
    message.id = str_oid;

    response.emplace_back(message);
  }
  mongoc_collection_destroy(collection);
  mongoc_cursor_destroy(cursor);
  mongoc_client_pool_push (pool, client);

  bson_destroy(query);
  if(opts != NULL)
    bson_destroy(opts);
  for(bson_t* q : queries) {
    bson_destroy(q);
  }

  return response;
}
