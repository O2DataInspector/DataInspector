#ifndef DIPROXY_MAPPERS_H
#define DIPROXY_MAPPERS_H

#include "domain/model/Device.h"
#include "DIMessages.h"
#include <algorithm>

std::vector<Input> toDomain(const std::vector<DIMessages::RegisterDevice::Specs::Input>& inputs) {
  std::vector<Input> result{};
  std::transform(inputs.begin(), inputs.end(), std::back_inserter(result), [](const DIMessages::RegisterDevice::Specs::Input& input) -> Input{
    return Input{
      .binding = input.binding,
      .sourceChannel = input.sourceChannel,
      .timeslice = input.timeslice,
      .origin = input.origin,
      .description = input.description,
      .subSpec = input.subSpec
    };
  });

  return result;
}

std::vector<Output> toDomain(const std::vector<DIMessages::RegisterDevice::Specs::Output>& outputs) {
  std::vector<Output> result{};
  std::transform(outputs.begin(), outputs.end(), std::back_inserter(result), [](const DIMessages::RegisterDevice::Specs::Output& output) -> Output{
    return Output{
            .binding = output.binding,
            .channel = output.channel,
            .timeslice = output.timeslice,
            .maxTimeslices = output.maxTimeslices,
            .origin = output.origin,
            .description = output.description,
            .subSpec = output.subSpec
    };
  });

  return result;
}

std::vector<Forward> toDomain(const std::vector<DIMessages::RegisterDevice::Specs::Forward>& forwards) {
  std::vector<Forward> result{};
  std::transform(forwards.begin(), forwards.end(), std::back_inserter(result), [](const DIMessages::RegisterDevice::Specs::Forward& forward) -> Forward{
    return Forward{
            .binding = forward.binding,
            .timeslice = forward.timeslice,
            .maxTimeslices = forward.maxTimeslices,
            .channel = forward.channel,
            .origin = forward.origin,
            .description = forward.description,
            .subSpec = forward.subSpec
    };
  });

  return result;
}

Device toDomain(const DIMessages::RegisterDevice& registerDevice) {
  return Device{
          .runId = registerDevice.runId,
          .name = registerDevice.name,
          .specs = Specs{
            .inputs = toDomain(registerDevice.specs.inputs),
            .outputs = toDomain(registerDevice.specs.outputs),
            .forwards = toDomain(registerDevice.specs.forwards),
            .rank = registerDevice.specs.rank,
            .nSlots = registerDevice.specs.nSlots,
            .inputTimesliceId = registerDevice.specs.inputTimesliceId,
            .maxInputTimeslices = registerDevice.specs.maxInputTimeslices
          }
  };
}

#endif //DIPROXY_MAPPERS_H
