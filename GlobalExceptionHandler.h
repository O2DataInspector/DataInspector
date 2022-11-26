#ifndef DIPROXY_EXCEPTIONHANDLER_H
#define DIPROXY_EXCEPTIONHANDLER_H

#include <vector>
#include <exception>
#include "httplib.h"
#include "serialization/RootSerialization.h"

class GlobalExceptionHandler {
public:
  static void handle(const httplib::Request& input, httplib::Response& output) {
    try {
      std::rethrow_exception(std::current_exception());
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
