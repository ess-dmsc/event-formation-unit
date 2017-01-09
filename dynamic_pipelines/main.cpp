#include <Pipeline.h>
#include <dlfcn.h>
#include <iostream>

int main(int argc, char *argv[]) {
  Pipeline nmx("./nmx.inst");
  Pipeline beer("./beer.inst");

  std::cout << "calling receive function nmx\n";
  for (int i = 0; i < 10; i++) {
    std::cout << i << " " << nmx.Rx(i) << "\n";
  }
  std::cout << "calling receive function beer\n";
  for (int i = 0; i < 10; i++) {
    std::cout << i << " " << beer.Rx(i) << "\n";
  }
  return 0;
}
