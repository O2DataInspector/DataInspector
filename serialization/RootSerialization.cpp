#include "serialization/RootSerialization.h"

/// SOURCE: ALICE O2

class RootDeserializationHelper : public TMessage
{
public:
  using TMessage::TMessage;
  RootDeserializationHelper(void* buf, Int_t len) : TMessage(buf, len) { ResetBit(kIsOwner); }
};

std::unique_ptr<TObject> RootSerialization::deserialize(uint8_t* data, int32_t size) {
  TClass* tgtClass = TClass::GetClass(typeid(TObject));
  if (tgtClass == nullptr) {
    throw std::runtime_error("class is not ROOT-serializable: " + std::string{typeid(TObject).name()});
  }

  RootDeserializationHelper tm(data, size);
  TClass* serializedClass = tm.GetClass();
  if (serializedClass == nullptr) {
    throw std::runtime_error("can not read class info from buffer");
  }
  if (tgtClass != serializedClass && serializedClass->GetBaseClass(tgtClass) == nullptr) {
    throw std::runtime_error("can not convert serialized class" + std::string{tm.GetClass()->GetName()} + " into target class " + std::string{tgtClass->GetName()});
  }
  return std::unique_ptr<TObject>(reinterpret_cast<TObject*>(tm.ReadObjectAny(serializedClass)));
}