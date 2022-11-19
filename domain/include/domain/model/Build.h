#ifndef DIPROXY_BUILD_H
#define DIPROXY_BUILD_H

#include <string>
#include <vector>

struct Build {
  std::string id;
  std::string name;
  std::string url;
  std::string branch;
  std::string path;
};

#endif //DIPROXY_BUILD_H
