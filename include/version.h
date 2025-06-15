#ifndef VERSION_H
#define VERSION_H

// 장비 버전 정보
#define DEVICE_VERSION_MAJOR 1
#define DEVICE_VERSION_MINOR 0
#define DEVICE_VERSION_PATCH 0

// 버전 문자열 매크로
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define DEVICE_VERSION_STRING "v" TOSTRING(DEVICE_VERSION_MAJOR) "." \
                             TOSTRING(DEVICE_VERSION_MINOR) "." \
                             TOSTRING(DEVICE_VERSION_PATCH)

// 버전 정보 구조체
struct VersionInfo {
    static const int major = DEVICE_VERSION_MAJOR;
    static const int minor = DEVICE_VERSION_MINOR;
    static const int patch = DEVICE_VERSION_PATCH;
    
    static const char* getVersionString() {
        return DEVICE_VERSION_STRING;
    }
    
    static bool isCompatible(int reqMajor, int reqMinor = 0, int reqPatch = 0) {
        if (major != reqMajor) return false;
        if (minor < reqMinor) return false;
        if (minor == reqMinor && patch < reqPatch) return false;
        return true;
    }
};

#endif // VERSION_H 