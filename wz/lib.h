#ifndef __WZVERSION_H__
#define __WZVERSION_H__

#ifdef __cplusplus
  #define EXTERN_C_START extern "C" {
  #define EXTERN_C_END }
#else
  #define EXTERN_C_START
  #define EXTERN_C_END
#endif

#ifdef _WIN32
  #ifdef WZVERSION_BUILD_DLL
  #define WZVERSION_API __declspec(dllexport)
  #else
  // #define WZVERSION_API __declspec(dllimport)
  #define WZVERSION_API
  #endif
#else
  #ifdef WZVERSION_BUILD_DLL
  #define WZVERSION_API __attribute__((visibility("default")))
  #else
  #define WZVERSION_API
  #endif
#endif

EXTERN_C_START
WZVERSION_API int get_wz_version(const char* path, char* out, int outlen);
EXTERN_C_END

#endif
