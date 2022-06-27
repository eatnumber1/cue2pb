#include "util/status_macros.h"

#include <iostream>
#include <ostream>
#include <cstdlib>

namespace util {

void StatusInternalOnlyDie(const absl::Status &st) {
  std::cerr << st << std::endl;
  std::abort();
}

}  // namespace util
