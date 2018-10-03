
#include <cinttypes>
#include <cstdio>
#include <string>
#include <sys/statvfs.h>
#include <vector>

std::vector<std::string> files{ "/tmp", "/home", "/fake" };

constexpr uint64_t MinDiskAvailable = 30 * 1000 * 1000 * 1000ULL;

int main(int argc, char * argv[])
{
   int ok = 0;
   for (auto file : files) { 
     struct statvfs fsstats;
     int ret = statvfs(file.c_str(), &fsstats);
     if (ret < 0) {
       printf("%s: ignored\n", file.c_str());
       continue;;
    }


    std::string res = "ok";
    uint64_t available = fsstats.f_bsize * fsstats.f_bavail;
    if (available < MinDiskAvailable) {
      ok++; 
     res = "failed";
    } 
    printf("%s: available: %" PRIu64 " (%s)\n", file.c_str(), available, res.c_str());
  }

  return ok;
}

