#ifndef DIPROXY_HTTPUTIL_H
#define DIPROXY_HTTPUTIL_H

enum class HTTPMethod {
  GET,
  POST,
  PUT,
  DELETE_
};

std::function<void(const httplib::Request&, httplib::Response&)> withErrorHandling(const std::function<void(const httplib::Request&, httplib::Response&)>& processRequest) {
  return [processRequest](const httplib::Request& input, httplib::Response& output) {
    try {
      processRequest(input, output);
    } catch (std::exception& e) {
      std::cout << "ERROR: " << e.what() << std::endl;
      output.status = 500;
    }
  };
}

void addEndpoint(httplib::Server& handle, HTTPMethod method, const std::string& path, const std::function<void(const httplib::Request&, httplib::Response&)>& endpointFunc) {
  switch (method) {
    case HTTPMethod::GET: {
      handle.Get(path, withErrorHandling(endpointFunc));
      break;
    }
    case HTTPMethod::POST: {
      handle.Post(path, withErrorHandling(endpointFunc));
      break;
    }
    case HTTPMethod::PUT: {
      handle.Put(path, withErrorHandling(endpointFunc));
      break;
    }
    case HTTPMethod::DELETE_: {
      handle.Delete(path, withErrorHandling(endpointFunc));
      break;
    }
  }
}

#define ENDPOINT_MEMBER_FUNC(object, func) [&object](const httplib::Request& input, httplib::Response& output) { object.func(input, output); }


#endif //DIPROXY_HTTPUTIL_H
