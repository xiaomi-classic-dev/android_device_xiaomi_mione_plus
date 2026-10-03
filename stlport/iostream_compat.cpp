#include "stlport_prefix.h"
#include <cstdio>

// Nougat makes FILE opaque; provide the accessor through the public API.
#define _STLP_STDIO_FILE_H
_STLP_BEGIN_NAMESPACE
inline int _FILE_fd(const FILE *stream) {
    return ::fileno(const_cast<FILE *>(stream));
}
_STLP_END_NAMESPACE

#include "iostream.cpp"
