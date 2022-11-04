#ifndef DIPROXY_ARROWSERIALIZATION_H
#define DIPROXY_ARROWSERIALIZATION_H

#include <memory>
#include "rapidjson/document.h"
#include "arrow/table.h"

namespace ArrowSerialization {
std::shared_ptr<arrow::Table> toTable(uint8_t* data, size_t size);
  rapidjson::Document toJson(std::shared_ptr<arrow::Table>& table);
}

#endif //DIPROXY_ARROWSERIALIZATION_H
