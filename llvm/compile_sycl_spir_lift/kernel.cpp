#include <cstddef>

#define GLOBAL __attribute__((address_space(1)))

#define const_func __attribute__((const))
std::size_t const_func __attribute__((overloadable)) get_global_id(unsigned int dimindx);

float square(float x) {
  return x*x;
}

[[sycl::device]]
void sum_squared(const float GLOBAL* x, const float GLOBAL* y, float GLOBAL* result) {
  auto index = get_global_id(0);
  x += index;
  y += index;
  result += index;
  *result = square(*x) + square(*y);  
}

// int main() {
//   return 0;
// }
