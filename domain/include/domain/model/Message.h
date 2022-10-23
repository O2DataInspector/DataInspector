#ifndef DIPROXY_MESSAGE_H
#define DIPROXY_MESSAGE_H

#include <string>
#include <optional>

struct Message {
  std::string id;

  /// ALWAYS
  std::string sender;

  /// DataProcessingHeader
  uint64_t startTime;
  uint64_t duration;
  uint64_t creationTimer;
  std::string origin;
  std::string description;
  uint32_t subSpecification;
  uint32_t firstTForbit;
  uint32_t tfCounter;
  uint32_t runNumber;
  uint64_t payloadSize;
  uint32_t splitPayloadParts;
  std::string payloadSerialization;
  uint32_t payloadSplitIndex;
  std::string payload;
  std::optional<std::string> payloadEndianness;

  /// OutputObjHeader
  std::optional<uint32_t> taskHash;
};

#endif //DIPROXY_MESSAGE_H
