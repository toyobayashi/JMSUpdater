#ifndef __WZ_DIRECTORY_HPP_
#define __WZ_DIRECTORY_HPP_

#include <cstddef>
#include <cstdint>
#include <list>
#include "WzBinaryReader.hpp"

namespace wz {
  struct WzImage {
    uint32_t offset;
  };

  struct WzDirectory {
    int32_t blockSize;
    int32_t checksum;
    uint32_t offset;
    std::list<WzImage*> images;
    std::list<WzDirectory*> subDirs;
    WzBinaryReader* reader;

    explicit WzDirectory(WzBinaryReader* r);

    ~WzDirectory();

    bool parseDirectory();

    std::list<WzImage*> getChildImages() const;
  };
}

#endif
