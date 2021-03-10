#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include "WzBinaryReader.hpp"
#include <utility>
#include <cstring>

namespace wz {
  static uint8_t MAPLESTORY_USERKEY_DEFAULT[] = {
    0x13, 0x00, 0x00, 0x00, 0x52, 0x00, 0x00, 0x00, 0x2A, 0x00, 0x00, 0x00, 0x5B, 0x00, 0x00, 0x00,
    0x08, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00,
    0x06, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x43, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00,
    0xB4, 0x00, 0x00, 0x00, 0x4B, 0x00, 0x00, 0x00, 0x35, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
    0x1B, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x5F, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00,
    0x0F, 0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x1B, 0x00, 0x00, 0x00,
    0x33, 0x00, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00,
    0x52, 0x00, 0x00, 0x00, 0xDE, 0x00, 0x00, 0x00, 0xC7, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00
  };

  WzHeader::WzHeader(): ident("PKG1"), fsize(0), fstart(60), copyright("Package file v1.0 Copyright 2002 Wizet, ZMS") {}

  uint32_t WzBinaryReader::rotateLeft(uint32_t x, uint8_t n) {
    return (uint32_t)(((x) << (n)) | ((x) >> (32 - (n))));
  }

  WzBinaryReader::WzBinaryReader(): hash(0), header() {
    std::vector<uint8_t> key(32, 0);
    for (size_t i = 0; i < 128; i += 16) {
      key[i / 4] = MAPLESTORY_USERKEY_DEFAULT[i];
    }
    this->wzKey = new WzMutableKey(key);
  }

  WzBinaryReader::~WzBinaryReader() {
    if (this->wzKey != nullptr) {
      delete this->wzKey;
      this->wzKey = nullptr;
    }
  }

  WzBinaryReader::WzBinaryReader(WzBinaryReader&& other) noexcept {
    this->hash = other.hash;
    other.hash = 0;
    this->header = std::move(other.header);
    this->wzKey = other.wzKey;
    other.wzKey = nullptr;
  }

  WzBinaryReader& WzBinaryReader::operator=(WzBinaryReader&& other) noexcept {
    BinaryReader::operator=(std::forward<WzBinaryReader>(other));
    this->hash = other.hash;
    other.hash = 0;
    this->header = std::move(other.header);
    this->wzKey = other.wzKey;
    other.wzKey = nullptr;
    return *this;
  }

  std::string WzBinaryReader::readWzString() {
    int8_t smallLength = this->readInt8();

    if (smallLength == 0) {
      return "";
    }

    int32_t length;
    std::vector<uint8_t> u8arr;

    if (smallLength > 0) {
      uint16_t mask = 0xAAAA;
      if (smallLength == 127) {
        length = this->readInt32LE();
      } else {
        length = smallLength;
      }
      if (length <= 0) {
        return "";
      }

      for (int32_t i = 0; i < length; i++) {
        uint16_t encryptedChar = this->readUInt16LE();
        encryptedChar = encryptedChar ^ mask;
        encryptedChar = encryptedChar ^ (((uint16_t)(this->wzKey->at(i * 2 + 1)) << 8) + (uint16_t)(this->wzKey->at(i * 2)));
        u8arr.push_back((uint8_t)(encryptedChar & 0xff));
        u8arr.push_back((uint8_t)(encryptedChar >> 8));
        mask++;
      }

      u8arr.push_back(0);
      u8arr.push_back(0);
      wchar_t* ptr = (wchar_t*)u8arr.data();
      int len = WideCharToMultiByte(CP_ACP, 0, ptr, -1, nullptr, 0, NULL, NULL);
      if (len == 0) return "";
      char* buf = new char[len];
      memset(buf, 0, len * sizeof(char));
      WideCharToMultiByte(CP_ACP, 0, ptr, -1, buf, len, NULL, NULL);
      std::string res(buf);
      delete[] buf;
      return res;
    } else { // ASCII
      uint8_t mask = 0xAA;
      if (smallLength == -128) {
        length = this->readInt32LE();
      } else {
        length = -smallLength;
      }
      if (length <= 0) {
        return "";
      }

      for (int32_t i = 0; i < length; i++) {
        uint8_t encryptedChar = this->readUInt8();
        encryptedChar = encryptedChar ^ mask;
        encryptedChar = encryptedChar ^ this->wzKey->at(i);
        u8arr.push_back(encryptedChar);
        mask++;
      }
      return std::string(u8arr.begin(), u8arr.end());
    }
  }

  int32_t WzBinaryReader::readWzInt() {
    int8_t sb = this->readInt8();
    if (sb == -128) {
      return this->readInt32LE();
    }
    return sb;
  }

  uint32_t WzBinaryReader::readWzOffset() {
    uint32_t offset = (uint32_t) this->tell();
    offset = (offset - this->header.fstart) ^ 0xFFFFFFFFU;
    offset = offset * this->hash;
    offset -= 0x581C3F6D;
    offset = WzBinaryReader::rotateLeft(offset, offset & 0x1F);
    uint32_t encryptedOffset = this->readUInt32LE();
    offset = offset ^ encryptedOffset;
    offset = offset + this->header.fstart * 2;
    return offset;
  }
}

