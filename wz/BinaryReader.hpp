#ifndef __BINARY_READER_HPP_
#define __BINARY_READER_HPP_

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <vector>
#include <string>

namespace wz {
  class BinaryReader {
  protected:
    size_t size;
  private:
    std::ifstream f;

  public:
    BinaryReader();
    virtual ~BinaryReader();
    BinaryReader(const BinaryReader&) = delete;
    BinaryReader& operator=(const BinaryReader&) = delete;

    BinaryReader(BinaryReader&&) noexcept;
    BinaryReader& operator=(BinaryReader&&) noexcept;

    void open(const std::string& path);
    void close();
    std::vector<uint8_t> read(size_t len = 1);
    std::string readString();
    std::string readString(size_t len);
    bool readBoolean();
    int8_t readInt8();
    uint8_t readUInt8();

    int16_t readInt16LE();
    uint16_t readUInt16LE();
    int16_t readInt16BE();
    uint16_t readUInt16BE();

    int32_t readInt32LE();
    uint32_t readUInt32LE();
    int32_t readInt32BE();
    uint32_t readUInt32BE();

    int64_t readBigInt64LE();
    uint64_t readBigUInt64LE();
    int64_t readBigInt64BE();
    uint64_t readBigUInt64BE();

    float readFloadLE();
    float readFloadBE();
    double readDoubleLE();
    double readDoubleBE();

    BinaryReader& seek(size_t pos);
    size_t tell();
    void clear();
  };
}

#endif
