#include "serialization/ArrowSerialization.h"

#include "arrow/buffer.h"
#include <arrow/builder.h>
#include <arrow/memory_pool.h>
#include <arrow/record_batch.h>
#include <arrow/table.h>
#include <arrow/type_traits.h>
#include <arrow/status.h>
#include <arrow/io/memory.h>
#include <arrow/ipc/reader.h>
#include <arrow/api.h>
#include <arrow/result.h>
#include <arrow/table_builder.h>
#include <arrow/util/iterator.h>
#include <arrow/util/logging.h>
#include <arrow/visit_array_inline.h>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <iostream>

const rapidjson::Value kNullJsonSingleton = rapidjson::Value();

class JsonValueHelper {
public:
  JsonValueHelper() = default;
  JsonValueHelper(std::nullptr_t): _null(true) {};

  rapidjson::Value& getValue() {
    return value;
  }

  bool operator==(const JsonValueHelper& other) const {
    return (_null && other._null) || (value == other.value);
  }

private:
  bool _null = false;
  rapidjson::Value value;
};

/// SOURCE: https://github.com/apache/arrow/blob/master/cpp/examples/arrow/rapidjson_row_converter.cc

/// \brief Builder that holds state for a single conversion.
///
/// Implements Visit() methods for each type of Arrow Array that set the values
/// of the corresponding fields in each row.
class RowBatchBuilder {
public:
  explicit RowBatchBuilder(rapidjson::Document& doc, int64_t num_rows) : doc(doc), field_(nullptr) {
    // Reserve all of the space required up-front to avoid unnecessary resizing
    rows_.reserve(num_rows);

    for (int64_t i = 0; i < num_rows; ++i) {
      rows_.emplace_back();
      rows_[i].getValue().SetObject();
    }
  }

  /// \brief Set which field to convert.
  void SetField(const arrow::Field* field) { field_ = field; }

  /// \brief Retrieve converted rows from builder.
  std::vector<JsonValueHelper> Rows() && { return std::move(rows_); }

  // Default implementation
  arrow::Status Visit(const arrow::Array& array) {
    return arrow::Status::NotImplemented(
            "Can not convert to json document for array of type ", array.type()->ToString());
  }

  // Handles booleans, integers, floats
  template <typename ArrayType, typename DataClass = typename ArrayType::TypeClass>
  arrow::enable_if_primitive_ctype<DataClass, arrow::Status> Visit(
          const ArrayType& array) {
    assert(static_cast<int64_t>(rows_.size()) == array.length());
    for (int64_t i = 0; i < array.length(); ++i) {
      if (!array.IsNull(i)) {
        rapidjson::Value str_key(field_->name().c_str(), doc.GetAllocator());
        rows_[i].getValue().AddMember(str_key, array.Value(i), doc.GetAllocator());
      }
    }
    return arrow::Status::OK();
  }

  arrow::Status Visit(const arrow::StringArray& array) {
    assert(static_cast<int64_t>(rows_.size()) == array.length());
    for (int64_t i = 0; i < array.length(); ++i) {
      if (!array.IsNull(i)) {
        rapidjson::Value str_key(field_->name().c_str(), doc.GetAllocator());
        std::string_view value_view = array.Value(i);
        rapidjson::Value value;
        value.SetString(value_view.data(),
                        static_cast<rapidjson::SizeType>(value_view.size()),
                        doc.GetAllocator());
        rows_[i].getValue().AddMember(str_key, value, doc.GetAllocator());
      }
    }
    return arrow::Status::OK();
  }

  arrow::Status Visit(const arrow::StructArray& array) {
    const arrow::StructType* type = array.struct_type();

    assert(static_cast<int64_t>(rows_.size()) == array.length());

    RowBatchBuilder child_builder(doc, rows_.size());
    for (int i = 0; i < type->num_fields(); ++i) {
      const arrow::Field* child_field = type->field(i).get();
      child_builder.SetField(child_field);
      ARROW_RETURN_NOT_OK(arrow::VisitArrayInline(*array.field(i).get(), &child_builder));
    }
    std::vector<JsonValueHelper> rows = std::move(child_builder).Rows();

    for (int64_t i = 0; i < array.length(); ++i) {
      if (!array.IsNull(i)) {
        rapidjson::Value str_key(field_->name().c_str(), doc.GetAllocator());
        // Must copy value to new allocator
//        rapidjson::Value row_val;
//        row_val.CopyFrom(rows[i], doc.GetAllocator());
        rows_[i].getValue().AddMember(str_key, rows[i].getValue(), doc.GetAllocator());
      }
    }
    return arrow::Status::OK();
  }

  arrow::Status Visit(const arrow::ListArray& array) {
    assert(static_cast<int64_t>(rows_.size()) == array.length());
    // First create rows from values
    std::shared_ptr<arrow::Array> values = array.values();
    RowBatchBuilder child_builder(doc, values->length());
    const arrow::Field* value_field = array.list_type()->value_field().get();
    std::string value_field_name = value_field->name();
    child_builder.SetField(value_field);
    ARROW_RETURN_NOT_OK(arrow::VisitArrayInline(*values.get(), &child_builder));

    std::vector<JsonValueHelper> rows = std::move(child_builder).Rows();

    int64_t values_i = 0;
    for (int64_t i = 0; i < array.length(); ++i) {
      if (array.IsNull(i)) continue;

      rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
      auto array_len = array.value_length(i);

      rapidjson::Value value;
      value.SetArray();
      value.Reserve(array_len, allocator);

      for (int64_t j = 0; j < array_len; ++j) {
        rapidjson::Value row_val;
        // Must copy value to new allocator
        row_val.CopyFrom(rows[values_i].getValue().FindMember(value_field_name.c_str())->value, allocator);
        value.PushBack(row_val, allocator);
        ++values_i;
      }

      rapidjson::Value str_key(field_->name().c_str(), allocator);
      rows_[i].getValue().AddMember(str_key, value, allocator);
    }

    return arrow::Status::OK();
  }

private:
  rapidjson::Document& doc;
  const arrow::Field* field_;
  std::vector<JsonValueHelper> rows_;
};  // RowBatchBuilder

class ArrowToDocumentConverter {
public:
  /// Convert a single batch of Arrow data into Documents
  static arrow::Result<std::vector<JsonValueHelper>> ConvertToVector(
          rapidjson::Document& doc,
          std::shared_ptr<arrow::RecordBatch> batch) {
    RowBatchBuilder builder{doc, batch->num_rows()};

    for (int i = 0; i < batch->num_columns(); ++i) {
      builder.SetField(batch->schema()->field(i).get());
      ARROW_RETURN_NOT_OK(arrow::VisitArrayInline(*batch->column(i).get(), &builder));
    }

    return std::move(builder).Rows();
  }

  /// Convert an Arrow table into an iterator of Documents
  static arrow::Iterator<JsonValueHelper> ConvertToIterator(
          rapidjson::Document& doc, std::shared_ptr<arrow::Table>& table, size_t batch_size) {
    // Use TableBatchReader to divide table into smaller batches. The batches
    // created are zero-copy slices with *at most* `batch_size` rows.
    auto batch_reader = std::make_shared<arrow::TableBatchReader>(*table);
    batch_reader->set_chunksize(batch_size);

    auto read_batch = [&doc](std::shared_ptr<arrow::RecordBatch> batch)
            -> arrow::Result<arrow::Iterator<JsonValueHelper>> {
      return arrow::MakeVectorIterator(ConvertToVector(doc, batch).ValueOrDie());
    };

    auto nested_iter = arrow::MakeMaybeMapIterator(
            read_batch, arrow::MakeIteratorFromReader(batch_reader));

    return arrow::MakeFlattenIterator(std::move(nested_iter));
  }
};  // ArrowToDocumentConverter

std::shared_ptr<arrow::Table> ArrowSerialization::toTable(uint8_t* data, size_t size) {
  auto buffer = std::make_shared<arrow::Buffer>(data, size);

  auto bufferReader = std::make_shared<arrow::io::BufferReader>(buffer);

  auto readerResult = arrow::ipc::RecordBatchStreamReader::Open(bufferReader);
  std::vector<std::shared_ptr<arrow::RecordBatch>> batches;
  auto batchReader = readerResult.ValueOrDie();
  while (true) {
    std::shared_ptr<arrow::RecordBatch> batch;
    auto next = batchReader->ReadNext(&batch);
    if (batch == nullptr) {
      break;
    }
    batches.push_back(batch);
  }

  return arrow::Table::FromRecordBatches(batches).ValueOrDie();
}

rapidjson::Document ArrowSerialization::toJson(std::shared_ptr<arrow::Table> &table) {
  rapidjson::Document doc = std::nullptr_t{};
  doc.SetObject();

  rapidjson::Value rowsValue;
  rowsValue.SetArray();

  auto jsonIter = ArrowToDocumentConverter::ConvertToIterator(doc, table, 64);
  for (arrow::Result<JsonValueHelper> rowJson : jsonIter) {
    rowsValue.PushBack(rowJson.ValueOrDie().getValue(), doc.GetAllocator());
  }

  doc.AddMember("rows", rowsValue, doc.GetAllocator());
  return doc;
}