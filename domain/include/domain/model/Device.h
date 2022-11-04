#ifndef DIPROXY_DEVICE_H
#define DIPROXY_DEVICE_H

#include <string>
#include <vector>
#include <boost/optional.hpp>

struct Input
{
  std::string binding;
  std::string sourceChannel;
  size_t timeslice;

  boost::optional<std::string> origin;
  boost::optional<std::string> description;
  boost::optional<uint32_t> subSpec;
};

struct Output
{
  std::string binding;
  std::string channel;
  size_t timeslice;
  size_t maxTimeslices;

  std::string origin;
  std::string description;
  boost::optional<uint32_t> subSpec;
};

struct Forward
{
  std::string binding;
  size_t timeslice;
  size_t maxTimeslices;
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

  size_t rank;
  size_t nSlots;
  size_t inputTimesliceId;
  size_t maxInputTimeslices;
};

struct Device {
  std::string runId;
  std::string name;
  Specs specs;
};

#endif //DIPROXY_DEVICE_H
