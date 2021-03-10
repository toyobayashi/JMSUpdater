#ifndef __WZ_BINARY_READER_HPP_
#define __WZ_BINARY_READER_HPP_

#include "BinaryReader.hpp"
#include "WzMutableKey.hpp"

namespace wz {

  struct WzHeader {
    std::string ident;
    uint64_t fsize;
    uint32_t fstart;
    std::string copyright;

    WzHeader();
  };

  class WzBinaryReader: public BinaryReader {
  private:
    static uint32_t rotateLeft(uint32_t x, uint8_t n);
  public:
    uint32_t hash;
    WzHeader header;
    WzMutableKey* wzKey;

    explicit WzBinaryReader();

    ~WzBinaryReader();

    WzBinaryReader(const WzBinaryReader&) = delete;
    WzBinaryReader& operator=(const WzBinaryReader&) = delete;

    WzBinaryReader(WzBinaryReader&& other) noexcept;

    WzBinaryReader& operator=(WzBinaryReader&& other) noexcept;

    std::string readWzString();

    int32_t readWzInt();

    uint32_t readWzOffset();
  };
}

#endif
