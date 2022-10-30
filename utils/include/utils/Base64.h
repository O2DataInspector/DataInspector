#ifndef DIPROXY_BASE64_H
#define DIPROXY_BASE64_H

#include <string>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>

namespace Base64 {
  std::string decode(const std::string& data) {
    using namespace boost::archive::iterators;
    using It = transform_width<binary_from_base64<std::string::const_iterator>, 8, 6>;
    auto paddingSize = std::count(data.begin(), data.end(), '=');
    auto decoded = std::string(It(std::begin(data)), It(std::end(data)));
    return std::string{decoded.begin(), decoded.end() - paddingSize};
  }
}

#endif //DIPROXY_BASE64_H
