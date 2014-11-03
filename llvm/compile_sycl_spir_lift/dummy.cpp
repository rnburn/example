#include <cstddef>

#define const_func __attribute__((const))
std::size_t const_func __attribute__((overloadable)) get_global_id(unsigned int dimindx) {
  return 0;
}
