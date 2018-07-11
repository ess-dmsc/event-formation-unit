
#include <string>
#include <vector>
#include <net/if.h>

class HwCheck {
public:
  static const int myflags = IFF_UP | IFF_RUNNING;

  HwCheck(int mtu) : minimumMtu(mtu) {};

  /// Seelct interfaces to check
  bool checkMTU(std::vector<std::string> ignore);
private:
  /// Check a single interface
  bool checkMTU(const char * interface);

  ///
  void debugPrint(struct ifaddrs * ifa);


  int minimumMtu{9000};
};
