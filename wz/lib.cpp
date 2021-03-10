#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include "lib.h"
#include "WzFile.hpp"
#include <cstring>

int get_wz_version (const char* path, char* out, int outlen) {
  std::string p = path;
  wz::WzFile wzFile(p);

  wz::WzFileParseStatus r = wzFile.parseWzFile();
  std::wstring msgW = wz::WzFile::getErrorDescription(r);
  std::string msg = "";
  int len = WideCharToMultiByte(CP_ACP, 0, msgW.c_str(), -1, nullptr, 0, NULL, NULL);
  if (len) {
    char* buf = new char[len];
    memset(buf, 0, len * sizeof(char));
    WideCharToMultiByte(CP_ACP, 0, msgW.c_str(), -1, buf, len, NULL, NULL);
    msg = buf;
    delete[] buf;
  }
  
  if (r != wz::WzFileParseStatus::SUCCESS) {
    if (out != NULL && outlen > 0) {
      strncpy(out, msg.c_str(), outlen - 1);
    }
    return 0;
  }
  if (out != NULL && outlen > 0) {
    msg = std::to_string(wzFile.mapleStoryPatchVersion);
    strncpy(out, msg.c_str(), outlen - 1);
  }
  return 1;
}
