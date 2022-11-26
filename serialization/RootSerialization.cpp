#include <rapidjson/document.h>
#include "serialization/RootSerialization.h"

/// SOURCE: ALICE O2

class RootDeserializationHelper : public TMessage
{
public:
  using TMessage::TMessage;
  RootDeserializationHelper(void* buf, Int_t len) : TMessage(buf, len) { ResetBit(kIsOwner); }
};

rapidjson::Document RootSerialization::toJson(TObject* obj) {
  TString json = TBufferJSON::ToJSON(obj);
  rapidjson::Document doc;
  doc.Parse(json.Data());

  return doc;
}

std::unique_ptr<TObject> RootSerialization::toObject(uint8_t* data, int32_t size) {
  TClass* tgtClass = TClass::GetClass(typeid(TObject));
  if (tgtClass == nullptr) {
    throw Exception("class is not ROOT-serializable: " + std::string{typeid(TObject).name()});
  }

  RootDeserializationHelper tm(data, size);
  TClass* serializedClass = tm.GetClass();
  if (serializedClass == nullptr) {
    throw Exception("can not read class info from buffer");
  }
  if (tgtClass != serializedClass && serializedClass->GetBaseClass(tgtClass) == nullptr) {
    throw Exception("can not convert serialized class" + std::string{tm.GetClass()->GetName()} + " into target class " + std::string{tgtClass->GetName()});
  }
  return std::unique_ptr<TObject>(reinterpret_cast<TObject*>(tm.ReadObjectAny(serializedClass)));
}