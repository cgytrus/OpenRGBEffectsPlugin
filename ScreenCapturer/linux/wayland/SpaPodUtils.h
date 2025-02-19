#ifndef SPAPODUTILS_H
#define SPAPODUTILS_H

#include <spa/pod/builder.h>

class SpaPodUtils
{
public:
    static spa_pod* CreateFormatOptions(spa_pod_builder* builder, const struct spa_rectangle* resolution, unsigned int framerate);
};

#endif // SPAPODUTILS_H
