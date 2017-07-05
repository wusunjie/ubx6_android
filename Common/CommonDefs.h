#ifndef _COMMON_DEFS_H
#define _COMMON_DEFS_H

// Generic helper definitions for shared library support
#if defined _WIN32 || defined __CYGWIN__
  #define MERBOK_GPS_HELPER_DLL_IMPORT __declspec(dllimport)
  #define MERBOK_GPS_HELPER_DLL_EXPORT __declspec(dllexport)
  #define MERBOK_GPS_HELPER_DLL_LOCAL
#else
  #if __GNUC__ >= 4
    #define MERBOK_GPS_HELPER_DLL_IMPORT __attribute__ ((visibility ("default")))
    #define MERBOK_GPS_HELPER_DLL_EXPORT __attribute__ ((visibility ("default")))
    #define MERBOK_GPS_HELPER_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define MERBOK_GPS_HELPER_DLL_IMPORT
    #define MERBOK_GPS_HELPER_DLL_EXPORT
    #define MERBOK_GPS_HELPER_DLL_LOCAL
  #endif
#endif

// Now we use the generic helper definitions above to define MERBOK_GPS_API and MERBOK_GPS_LOCAL.
// MERBOK_GPS_API is used for the public API symbols. It either DLL imports or DLL exports (or does nothing for static build)
// MERBOK_GPS_LOCAL is used for non-api symbols.

#ifdef MERBOK_GPS_DLL // defined if MERBOK_GPS is compiled as a DLL
  #ifdef MERBOK_GPS_DLL_EXPORTS // defined if we are building the MERBOK_GPS DLL (instead of using it)
    #define MERBOK_GPS_API MERBOK_GPS_HELPER_DLL_EXPORT
  #else
    #define MERBOK_GPS_API FOX_HELPER_DLL_IMPORT
  #endif // MERBOK_GPS_DLL_EXPORTS
  #define MERBOK_GPS_LOCAL MERBOK_GPS_HELPER_DLL_LOCAL
#else // MERBOK_GPS_DLL is not defined: this means MERBOK_GPS is a static lib.
  #define MERBOK_GPS_API
  #define MERBOK_GPS_LOCAL
#endif // MERBOK_GPS_DLL

#ifdef __cplusplus
  #define MERBOK_EXTERN_C extern "C"
  #else
  #define MERBOK_EXTERN_C
#endif


#endif