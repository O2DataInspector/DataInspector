#ifndef DIPROXY_ROOTSERIALIZATION_H
#define DIPROXY_ROOTSERIALIZATION_H

#include "TMessage.h"
#include "TBufferJSON.h"

namespace RootSerialization {
  std::unique_ptr<TObject> deserialize(uint8_t* data, int32_t size);
}

#endif //DIPROXY_ROOTSERIALIZATION_H
