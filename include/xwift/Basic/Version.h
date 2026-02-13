#ifndef XWIFT_BASIC_VERSION_H
#define XWIFT_BASIC_VERSION_H

#include <string>

namespace xwift {

class Version {
public:
  unsigned Major;
  unsigned Minor;
  unsigned Patch;

  Version(unsigned major, unsigned minor, unsigned patch)
    : Major(major), Minor(minor), Patch(patch) {}

  std::string getAsString() const {
    return std::to_string(Major) + "." + std::to_string(Minor) + "." + std::to_string(Patch);
  }
};

namespace version {
  constexpr unsigned Major = 0;
  constexpr unsigned Minor = 1;
  constexpr unsigned Patch = 0;
  
  inline std::string getXWiftVersion() {
    return Version(Major, Minor, Patch).getAsString();
  }
}

}

#endif
