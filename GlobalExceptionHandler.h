#ifndef DIPROXY_EXCEPTIONHANDLER_H
#define DIPROXY_EXCEPTIONHANDLER_H

#include <vector>
#include <exception>
#include "httplib.h"
#include "serialization/RootSerialization.h"
#include "domain/MessageRepository.h"
#include "domain/DevicesRepository.h"
#include "domain/RunRepository.h"
#include "domain/BuildRepository.h"
#include "api/RunsEndpoint.h"

class GlobalExceptionHandler {
public:
  static void handle(const httplib::Request& input, httplib::Response& output) {
    try {
      std::rethrow_exception(std::current_exception());
    } catch (const StartRunBadRequest& ex) {
      std::cout << "ERROR[StartRunBadRequest] - " << ex.what() << std::endl;
      output.status = 400;
    } catch (const BuildNotSaved& ex) {
      std::cout << "ERROR[BuildNotSaved] - " << ex.what() << std::endl;
      output.status = 500;
    } catch (const MessageNotSaved& ex) {
      std::cout << "ERROR[MessageNotSaved] - " << ex.what() << std::endl;
      output.status = 500;
    } catch (const RunNotSaved& ex) {
      std::cout << "ERROR[RunNotSaved] - " << ex.what() << std::endl;
      output.status = 500;
    } catch (const DeviceNotSaved& ex) {
      std::cout << "ERROR[DeviceNotSaved] - " << ex.what() << std::endl;
      output.status = 500;
    } catch (const BuildNotFound& ex) {
      std::cout << "ERROR[BuildNotFound] - " << ex.what() << std::endl;
      output.status = 404;
    } catch (const MessageNotFound& ex) {
      std::cout << "ERROR[MessageNotFound] - " << ex.what() << std::endl;
      output.status = 404;
    } catch (const RunNotFound& ex) {
      std::cout << "ERROR[RunNotFound] - " << ex.what() << std::endl;
      output.status = 404;
    } catch (const DeviceNotFound& ex) {
      std::cout << "ERROR[DeviceNotFound] - " << ex.what() << std::endl;
      output.status = 404;
    } catch (const RootSerialization::Exception& ex) {
      std::cout << "ERROR[RootSerialization::Exception] - " << ex.what() << std::endl;
      output.status = 500;
    } catch (const std::exception& ex) {
      std::cout << "ERROR[std::exception] - " << ex.what() << std::endl;
      output.status = 500;
    }
  }
};

#endif //DIPROXY_EXCEPTIONHANDLER_H
