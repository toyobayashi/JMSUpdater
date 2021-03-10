
#include <cstddef>
#include <exception>
#include <iostream>
#include "WzBinaryReader.hpp"

#include "WzFile.hpp"

namespace wz {

std::wstring WzFile::getErrorDescription(WzFileParseStatus status) {
  switch (status) {
    case WzFileParseStatus::SUCCESS:
      // return "Success";
      return L"成功";
    case WzFileParseStatus::FAILED_UNKNOWN:
      // return "Failed, in this case the causes are undetermined.";
      return L"未知错误";
    case WzFileParseStatus::INVALID_WZ:
      // return "Invalid WZ file";
      return L"不是合法的 WZ 格式文件";

    case WzFileParseStatus::PATH_IS_NULL:
      // return "Path is null";
      return L"路径为空";
    case WzFileParseStatus::ERROR_GAME_VER_HASH:
      // return "Error with game version hash : The specified game version is incorrect and WzLib was unable to determine the version itself";
      return L"版本哈希值错误，无法检测";
    default: return L"";
  }
}

WzFile::WzFile(const std::string f): _version(0), _versionHash(0), _wzDir(nullptr), mapleStoryPatchVersion(-1), filepath(f) {}

WzFile::~WzFile() {
  if (_wzDir != nullptr) {
    delete _wzDir;
    _wzDir = nullptr;
  }
}

WzFile::WzFile(WzFile&& other) noexcept {
  _version = other._version;
  other._version = 0;
  _versionHash = other._versionHash;
  other._versionHash = 0;
  mapleStoryPatchVersion = other.mapleStoryPatchVersion;
  other.mapleStoryPatchVersion = 0;
  filepath = std::move(other.filepath);
  _wzDir = other._wzDir;
  other._wzDir = nullptr;
}
WzFile& WzFile::operator=(WzFile&& other) noexcept {
  _version = other._version;
  other._version = 0;
  _versionHash = other._versionHash;
  other._versionHash = 0;
  mapleStoryPatchVersion = other.mapleStoryPatchVersion;
  other.mapleStoryPatchVersion = 0;
  filepath = std::move(other.filepath);
  _wzDir = other._wzDir;
  other._wzDir = nullptr;
  return *this;
}

WzFileParseStatus WzFile::parseWzFile() {
  if (this->filepath == "") {
    return WzFileParseStatus::PATH_IS_NULL;
  }

  WzBinaryReader* reader = new WzBinaryReader();
  try {
    reader->open(this->filepath);
  } catch (const std::ios_base::failure&) {
    delete reader;
    return WzFileParseStatus::INVALID_WZ;
  }
  WzHeader header;
  header.ident = reader->readString(4);
  if (header.ident != "PKG1") {
    delete reader;
    return WzFileParseStatus::INVALID_WZ;
  }
  header.fsize = reader->readBigUInt64LE();
  header.fstart = reader->readUInt32LE();
  header.copyright = reader->readString(header.fstart - 17);

  reader->read(1);
  reader->read(header.fstart - (uint32_t)reader->tell());
  reader->header = header;
  this->_version = reader->readInt16LE();

  int32_t MAX_PATCH_VERSION = 10000;
  for (int32_t j = 0; j < MAX_PATCH_VERSION; j++) {
    this->mapleStoryPatchVersion = j;
    this->_versionHash = this->_checkAndGetVersionHash(this->_version, this->mapleStoryPatchVersion);
    if (this->_versionHash == 0) {
      continue;
    }
    reader->hash = this->_versionHash;
    size_t position = reader->tell(); // save position to rollback to, if should parsing fail from here
    WzDirectory testDirectory(reader);
    testDirectory.offset = (uint32_t)position;
    try {
      if (!testDirectory.parseDirectory()) {
        reader->seek(position);
        continue;
      }
    } catch (const std::ios_base::failure&) {
      reader->clear();
      reader->seek(position);
      continue;
    } catch (const std::exception&) {
      reader->seek(position);
      continue;
    }

    try {
      std::list<WzImage*> childImages = testDirectory.getChildImages();
      if (childImages.size() == 0) {
        reader->seek(position);
        continue;
      }

      try {
        auto it = childImages.begin();
        WzImage* first = *it;
        reader->seek(first->offset);
        uint8_t checkByte = reader->readUInt8();
        reader->seek(position);
        switch (checkByte) {
          case 0x73:
          case 0x1b: {
            WzDirectory* directory = new WzDirectory(reader);
            directory->offset = (uint32_t)reader->tell();
            if (directory->parseDirectory()) {
              this->_wzDir = directory;
              return WzFileParseStatus::SUCCESS;
            } else {
              break;
            }
          }
          case 0x30:
          case 0x6C: // idk
          case 0xBC: // Map002.wz? KMST?
          default: {
            break;
          }
        }
        reader->seek(position); // reset
      } catch (const std::ios_base::failure&) {
        reader->clear();
        reader->seek(position);
      } catch (const std::exception&) {
        reader->seek(position);
      }
    } catch (const std::ios_base::failure&) {
      reader->clear();
      // nothing
    } catch (const std::exception&) {
      // nothing
    }
  }
  delete reader;
  return WzFileParseStatus::ERROR_GAME_VER_HASH;
}

uint32_t WzFile::_checkAndGetVersionHash (uint16_t wzVersionHeader, int32_t maplestoryPatchVersion) {
  int32_t VersionNumber = maplestoryPatchVersion;
  int32_t VersionHash = 0;
  std::string VersionNumberStr = std::to_string(VersionNumber);

  size_t l = VersionNumberStr.length();
  for (size_t i = 0; i < l; i++) {
    VersionHash = (32 * VersionHash) + (int32_t)VersionNumberStr[i] + 1;
  }

  int32_t a = (VersionHash >> 24) & 0xFF;
  int32_t b = (VersionHash >> 16) & 0xFF;
  int32_t c = (VersionHash >> 8) & 0xFF;
  int32_t d = VersionHash & 0xFF;
  int32_t DecryptedVersionNumber = (0xff ^ a ^ b ^ c ^ d);

  if ((int32_t)wzVersionHeader == DecryptedVersionNumber) return (uint32_t)VersionHash;

  return 0;
}

}

