#ifndef DIPROXY_DEVICE_H
#define DIPROXY_DEVICE_H

#include <string>
#include <vector>
#include <boost/optional.hpp>

struct Input
{
  std::string binding;
  std::string sourceChannel;
  uint64_t timeslice;

  boost::optional<std::string> origin;
  boost::optional<std::string> description;
  boost::optional<uint32_t> subSpec;
};

struct Output
{
  std::string binding;
  std::string channel;
  uint64_t timeslice;
  uint64_t maxTimeslices;

  std::string origin;
  std::string description;
  boost::optional<uint32_t> subSpec;
};

struct Forward
{
  std::string binding;
  uint64_t timeslice;
  uint64_t maxTimeslices;
  std::string channel;

  boost::optional<std::string> origin;
  boost::optional<std::string> description;
  boost::optional<uint32_t> subSpec;
};

struct Specs
{
  std::vector<Input> inputs;
  std::vector<Output> outputs;
  std::vector<Forward> forwards;

  uint64_t rank;
  uint64_t nSlots;
  uint64_t inputTimesliceId;
  uint64_t maxInputTimeslices;
};

struct Device {
  std::string runId;
  std::string name;
  bool isSelected;
  Specs specs;
};

#endif //DIPROXY_DEVICE_H
