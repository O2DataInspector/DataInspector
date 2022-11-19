#include "domain/BuildDetector.h"
#include <boost/filesystem.hpp>
#include <iostream>

/**
 * In folder ~/alice/sw/<arch>/O2Physics:
 *  - <name1>.build
 *  - <name2>.build
 *  - ...
 *
 *  In file <name>.build:
 *   - name
 *   - url
 *   - branch
 *   - path to build
 */
void BuildDetector::detectBuilds() {
  auto detect = [this](const boost::filesystem::path& path) {
    std::cout << path << std::endl;

    if(!boost::filesystem::is_regular_file(path))
      return;

    std::cout << "EXTENSION: " << path.filename().extension() << std::endl;
    if(path.filename().extension() != ".build")
      return;

    std::string name, url, branch, buildPath;
    std::ifstream file{path};

    std::getline(file, name);
    file >> url >> branch >> buildPath;
    file.close();

    std::cout << "[O2Physcis] Detected - " << path.filename() << "(name=" << name << ", url=" << url << ", branch=" << branch << ", path=" << buildPath << ")" << std::endl;
    if(buildRepository.getByName(name).has_value()) {
      std::cout << "Already present" << std::endl;
      return;
    }

    buildRepository.save(Build{"", name, url, branch, buildPath});
  };

  boost::filesystem::recursive_directory_iterator it(basePath), end;
  while (it != end) {
    detect(it->path());
    ++it;
  }
}