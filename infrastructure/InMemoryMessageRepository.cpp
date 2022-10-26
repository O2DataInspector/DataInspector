#include "infrastructure/InMemoryMessageRepository.h"
#include <iostream>
#include <algorithm>

#include <mongoc/mongoc.h>
#include <bson/bson.h>

#define MONGODB_SWITCH
//#undef MONGODB_SWITCH
#ifdef MONGODB_SWITCH
#define INIT_BUFF_SIZE 20
#endif
extern int prx_dbg;

std::string InMemoryMessageRepository::addMessage(const std::string& runId, const Message& message) {

#ifndef MONGODB_SWITCH
  messageMutex.lock();
  std::cout << "MessageRepository::addMessage" << std::endl;
#endif

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
  bson_error_t error;
  bson_decimal128_t dec;
  mongoc_client_t *client;
  mongoc_collection_t *collection;

  mongoc_init();

  client = mongoc_client_new("mongodb://localhost:27017/?appname=prx");

  /* collection = mongoc_client_get_collection(client, "prx-db", analysisId.c_str()); */
  /* uncomment above line when specifying 'analysisId' header will be required */

  collection = mongoc_client_get_collection(client, "prx-db", "test");

  char *ovrflw_beg, *ovrflw_end, *ovrflw_str;
  char *field_beg, *field_end, *field_str;
  char *msg, *msg_rest;


  char **field_buff  = (char **)malloc(INIT_BUFF_SIZE * sizeof(char *));
  char **ovrflw_buff = (char **)malloc(INIT_BUFF_SIZE * sizeof(char *));

  char *err_msg;

  int ovrflw_idx = 0;

  msg = strdup(message.raw.c_str());

  /* c_str() return 'const char *' thus strdup is mandatory if we want
   * to modify message in case of overflow */
  
  while((doc = 
	bson_new_from_json((const unsigned char *) msg, -1, &error)) == NULL)
  {
	err_msg = strdup(error.message);
	if(error.domain == 1 && error.code == 2) {

		/* creationTimer's type is unsigned long int, but it's
		 * bigger than long int; there is no macro/function to
		 * append uint64 to BSON document besides decimal128.
		 */ 

		/* 
		 * overflow detected - get the "ovrflw" number from err_msg;
		 * get and remove "field":ovrflw from msg;
		 * append field and ovrflw to proper array of buffers.
		 *
		 * note that overflowed number in error message is embedded 
		 * in "...", while in JSON not.
		 */

		/* fixme: s/err_domain == 1/MACRO and s/err_code == 2/MACRO */

		ovrflw_beg = strchr(err_msg, '"');
		ovrflw_end = strchr(ovrflw_beg + 1, '"');

		*ovrflw_end = '\0';
		ovrflw_str = strdup(ovrflw_beg + 1);
		*ovrflw_end = '"';

		ovrflw_beg = strstr(msg, ovrflw_str);
		size_t len = strlen(ovrflw_str);
		ovrflw_end = ovrflw_beg + (len - 1);

		field_end = ovrflw_beg - 2;

		*field_end = '\0';
		field_beg = strrchr(msg, '"');
		field_str = strdup(field_beg + 1);
		*field_end = '"';
		/*
		 * v field_beg  field_end v v ovrflw_beg  ovrflw_end v   
		 * "......................":1234..................5678   
		 *  ^------field_str-----^  ^-------ovrflw_str-------^   
		 */

		if(prx_dbg) 
		   std::cerr << "WARNING: overflow detected in:" << std::endl
			     << "\t " << field_str << ":" << ovrflw_str << std::endl
			     << "\t automatically adding correct value." << std::endl;

		field_buff[ovrflw_idx] = field_str;
		ovrflw_buff[ovrflw_idx] = ovrflw_str;
		ovrflw_idx++;
		/* fixme: add realloc in case of running out of buffer */

		msg_rest = strdup(ovrflw_end + 1);

		     if (field_beg[-1] == ',' && ovrflw_end[1] == ',')
		{
			strcpy(field_beg-1, msg_rest);
			/* replace leading comma with following one */
		}
		else if (field_beg[-1] == '{' && ovrflw_end[1] == ',')
		{
			strcpy(field_beg, msg_rest+1);
			/* omit following comma */
		}
		else if (field_beg[-1] == ',' && ovrflw_end[1] == '}')
		{
			strcpy(field_beg-1, msg_rest);
			/* replace leading comma with rest of JSON. 
			 * note: not necessarily end of JSON. Can be
 			 * end of object. 
 			 */
		}
		else if (field_beg[-1] == '{' && ovrflw_end[1] == '}')
		{
			strcpy(field_beg, msg_rest);
			/* only one entry in object, not sure if
 			 * it is handled correctly
 			 */
		}
		free(msg_rest);
	}
	free(err_msg);
  }

  while (ovrflw_idx > 0)
  {
	ovrflw_idx--;
	bson_decimal128_from_string(ovrflw_buff[ovrflw_idx], &dec);
	/* ovrflw_num = atoul(ovrflw_buff[ovrflw_idx]); */
	/* BSON_APPEND_INT64(doc, field_buff[ovrflw_idx], ovrflw_num); */
	/* quirk */
	//BSON_APPEND_UTF8(doc, field_buff[ovrflw_idx], ovrflw_buff[ovrflw_idx]);
	BSON_APPEND_DECIMAL128(doc, field_buff[ovrflw_idx], &dec);
	free(field_buff[ovrflw_idx]);
	free(ovrflw_buff[ovrflw_idx]);
	/* moze dodac liste wlasnosci z overflow i trzymac ja jako wlasnosc __OVERFLOW__ ? */
  }

  free(field_buff);
  free(ovrflw_buff);

  BSON_APPEND_BOOL(doc, "read", false);
  /* fixme: before appending: check if read is present in document */

  if(!mongoc_collection_insert_one(
     collection, doc, NULL, NULL, &error))
  {
	std::cerr << error.message << std::endl;
  }
  bson_destroy(doc);
  free(msg);
  mongoc_collection_destroy(collection);
  mongoc_client_destroy(client);
  mongoc_cleanup();
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
/* fixme: handle case when count is not given. error munmap_check: */
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
  bson_error_t error;
  bson_t *pipeline;
  bson_t *query;
  bson_t *update;
  const bson_t *doc;
  const bson_value_t *value;
  bson_oid_t oid;
  bson_iter_t iter;
  mongoc_client_t *client;
  mongoc_cursor_t *cursor;
  mongoc_collection_t *collection;

  struct Message messages[count];
  std::vector<Message> response{};
  int msg_id = 0;

  mongoc_init();

  client = mongoc_client_new("mongodb://localhost:27017/?appname=prx");

  /* collection = mongoc_client_get_collection(client, "prx-db", analysisId.c_str()); */
  /* uncomment above line when specifying 'analysisId' header will be required */

  collection = mongoc_client_get_collection(client, "prx-db", "test");

  char *str;
  /* fixme: add constraints to rule out two unread messages with same startTime */
/*
 * below is better solution - instead of invoking (erronous as we 
 * already saw) bson_new_from_json, make the objects explicitly. It should
 * also be faster - you have more control over which calls are invoked instead
 * of generic macros; it's critical as function receiving messages will be 
 * invoked furiously.
 *
 * the best solution is to only use the functions, without any macros. It should
 * have the minimal invokation as possible.
 *
 */
 /*
  bson_t *match_cmd = BCON_NEW(
				"$match",
				   "{",
					"read",
					BCON_BOOL(false),
				   "}"
			      );

  bson_t *sort_cmd = BCON_NEW(
				"$sort",
				   "{",
					"startTime",
					BCON_INT32(1),
				   "}"
			     );

  bson_t *limit_cmd = BCON_NEW("$limit",
				BCON_INT32(count));

  * fixme: you have to glue this docs together into one array
  * fixme2: in BSON there is no array. Probably you have to convert it into
  * one doc, and order of array is determined by keys in doc "0", "1", ...
  */ 

/*
 * ponizszy BSON sie nie tworzy - najwidoczniej nie ma mozliwosci stworzenia
 * dokumentu zaczynajacym sie od "{" lub "[". Calkiem prawdopodobne, ze bug.
 * edit: "{" lub "[" nie jest dozwolone w BSON'ie, tylko w JSON'ie. API nie
 * pozwala na appendowanie docow oraz tablic bez klucza, czyli takie tablice
 * 'anonimowe' musisz tlumaczyc na doca z kluczami "0", "1", "2", ... itd.
  */
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
						"}",
				"}",
			"2",
				"{",
					"$limit",
					BCON_INT32(count),
				"}"
	     	     );

  printf("json as canonical BSON:\n%s\n", 
		bson_as_canonical_extended_json(pipeline, NULL));
  /*std::cout << bson_as_canonical_extended_json(pipeline, NULL) 
 	      << std::endl;*/
  cursor = mongoc_collection_aggregate(
	collection, MONGOC_QUERY_NONE, pipeline, NULL, NULL);
  while(mongoc_cursor_next(cursor, &doc))
  {
	/* char *str = bson_as_canonical_extended_json(doc, NULL); */
	str = bson_as_relaxed_extended_json(doc, NULL);
	printf("returned json from db:\n%s\n", str);
	messages[msg_id].raw = str;
	response.emplace_back(messages[msg_id]);
	msg_id++;
	if(bson_iter_init(&iter, doc)
	&& bson_iter_find(&iter, "_id") 
	&& BSON_ITER_HOLDS_OID(&iter))
	{
		if(prx_dbg)
			printf("found key: %s\n", bson_iter_key(&iter));
		value = bson_iter_value(&iter);
		bson_oid_copy(&(value->value.v_oid), &oid);
		if(prx_dbg)
		{
			char str[25];
			bson_oid_to_string(&oid, str);
			printf("found oid: %s\n", str);
		}
	}
	else
	{
		std::cout << "error while initializing the BSON iterator" 
			  << std::endl;
	}

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
	free(str);
  }

  bson_destroy(pipeline);
  mongoc_collection_destroy(collection);
  mongoc_client_destroy(client);
  mongoc_cleanup();
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
