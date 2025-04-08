#ifndef SAMPVER_FILE_H
#define SAMPVER_FILE_H

#include <string>

enum class samp_ver {
    unknown = -1,

    v037r1 = 0,
    v037r31,
    v037r4,
    v03dlr1
};

class sampver {
public:
    samp_ver getSampVersion();
};

extern sampver* gSampVersion;

#endif