#pragma once

#define MAKEFOURCC(a, b, c, d) ((d << 24) | (c << 16) | (b << 8) | (a << 0))
#define ARRAYSIZE(a) (sizeof(a) / sizeof(a[0]))
