#include <iostream>

extern const char device_bitcode[];
extern int device_bitcode_size;

int main() {
  std::cout << device_bitcode_size << "\n";
  for(int i=0; i<device_bitcode_size; ++i) {
    std::cout << device_bitcode[i] << "\n";
  }
  return 0;
}
