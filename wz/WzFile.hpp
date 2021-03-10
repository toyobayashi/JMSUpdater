#ifndef __WZ_FILE_HPP_
#define __WZ_FILE_HPP_

#include <cstdint>
#include <string>
#include "WzDirectory.hpp"

// #include "WzFile.hpp"
namespace wz {

enum class WzFileParseStatus {
  PATH_IS_NULL = -1,
  ERROR_GAME_VER_HASH = -2, // Error with game version hash : The specified game version is incorrect and WzLib was unable to determine the version itself
  INVALID_WZ = -99,
  FAILED_UNKNOWN = 0x0,
  SUCCESS = 0x1
};

struct WzFile {
private:
  int16_t _version;
  uint32_t _versionHash;
  WzDirectory* _wzDir;
public:
  int32_t mapleStoryPatchVersion;
  std::string filepath;

  static std::wstring getErrorDescription(WzFileParseStatus);

  explicit WzFile(const std::string f);

  ~WzFile();

  WzFile(const WzFile&) = delete;
  WzFile& operator=(const WzFile&) = delete;

  WzFile(WzFile&& other) noexcept;
  WzFile& operator=(WzFile&& other) noexcept;
  
  WzFileParseStatus parseWzFile();

private:
  uint32_t _checkAndGetVersionHash (uint16_t wzVersionHeader, int32_t maplestoryPatchVersion);
};

}

#endif
