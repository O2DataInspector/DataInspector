#ifndef DIPROXY_ROOTSERIALIZATION_H
#define DIPROXY_ROOTSERIALIZATION_H

#include "TMessage.h"
#include "TBufferJSON.h"

namespace RootSerialization {
  std::unique_ptr<TObject> toObject(uint8_t* data, int32_t size);
  rapidjson::Document toJson(TObject* obj);

  struct Exception : std::runtime_error {
    Exception(const std::string& what) : std::runtime_error(what) {};
  };
}

#endif //DIPROXY_ROOTSERIALIZATION_H
