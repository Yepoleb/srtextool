#pragma once
#include <stdint.h>
#include <stddef.h>

#include "common.hpp"

struct DDSPixelformat;
enum class TextureFormat;

const uint32_t FOURCC_DDS = MAKEFOURCC('D', 'D', 'S', ' ');
const size_t DDS_HEADER_SIZE = 124;
const size_t DDS_PIXELFORMAT_SIZE = 32;

// Pixelformat flags
// Source: Ddraw.h

const uint32_t DDPF_ALPHAPIXELS = 0x1;
const uint32_t DDPF_ALPHA = 0x2;
const uint32_t DDPF_FOURCC = 0x4;
const uint32_t DDPF_PALETTEINDEXED4 = 0x8;
const uint32_t DDPF_PALETTEINDEXEDTO8 = 0x10;
const uint32_t DDPF_PALETTEINDEXED8 = 0x20;
const uint32_t DDPF_RGB = 0x40;
const uint32_t DDPF_RGBA = 0x41;
const uint32_t DDPF_COMPRESSED = 0x80;
const uint32_t DDPF_RGBTOYUV = 0x100;
const uint32_t DDPF_YUV = 0x200;
const uint32_t DDPF_ZBUFFER = 0x400;
const uint32_t DDPF_PALETTEINDEXED1 = 0x800;
const uint32_t DDPF_PALETTEINDEXED2 = 0x1000;
const uint32_t DDPF_ZPIXELS = 0x2000;
const uint32_t DDPF_STENCILBUFFER = 0x4000;
const uint32_t DDPF_ALPHAPREMULT = 0x8000;
const uint32_t DDPF_LUMINANCE = 0x20000;
const uint32_t DDPF_BUMPLUMINANCE = 0x40000;
const uint32_t DDPF_BUMPDUDV = 0x80000;

// Header flags

const uint32_t DDSD_CAPS = 0x1;
const uint32_t DDSD_HEIGHT = 0x2;
const uint32_t DDSD_WIDTH = 0x4;
const uint32_t DDSD_PITCH = 0x8;
const uint32_t DDSD_BACKBUFFERCOUNT = 0x20;
const uint32_t DDSD_ZBUFFERBITDEPTH = 0x40;
const uint32_t DDSD_ALPHABITDEPTH = 0x80;
const uint32_t DDSD_LPSURFACE = 0x800;
const uint32_t DDSD_PIXELFORMAT = 0x1000;
const uint32_t DDSD_CKDESTOVERLAY = 0x2000;
const uint32_t DDSD_CKDESTBLT = 0x4000;
const uint32_t DDSD_CKSRCOVERLAY = 0x8000;
const uint32_t DDSD_CKSRCBLT = 0x10000;
const uint32_t DDSD_MIPMAPCOUNT = 0x20000;
const uint32_t DDSD_REFRESHRATE = 0x40000;
const uint32_t DDSD_LINEARSIZE = 0x80000;
const uint32_t DDSD_TEXTURESTAGE = 0x100000;
const uint32_t DDSD_FVF = 0x200000;
const uint32_t DDSD_SRCVBHANDLE = 0x400000;
const uint32_t DDSD_DEPTH = 0x800000;
const uint32_t DDSD_REQUIRED = 0x1007;
const uint32_t DDSD_ALL = 0xfff9ee;

// Caps flags

const uint32_t DDSCAPS_RESERVED1 = 0x1;
const uint32_t DDSCAPS_ALPHA = 0x2;
const uint32_t DDSCAPS_BACKBUFFER = 0x4;
const uint32_t DDSCAPS_COMPLEX = 0x8;
const uint32_t DDSCAPS_FLIP = 0x10;
const uint32_t DDSCAPS_FRONTBUFFER = 0x20;
const uint32_t DDSCAPS_OFFSCREENPLAIN = 0x40;
const uint32_t DDSCAPS_OVERLAY = 0x80;
const uint32_t DDSCAPS_PALETTE = 0x100;
const uint32_t DDSCAPS_PRIMARYSURFACE = 0x200;
const uint32_t DDSCAPS_RESERVED3 = 0x400;
const uint32_t DDSCAPS_SYSTEMMEMORY = 0x800;
const uint32_t DDSCAPS_TEXTURE = 0x1000;
const uint32_t DDSCAPS_3DDEVICE = 0x2000;
const uint32_t DDSCAPS_VIDEOMEMORY = 0x4000;
const uint32_t DDSCAPS_VISIBLE = 0x8000;
const uint32_t DDSCAPS_WRITEONLY = 0x10000;
const uint32_t DDSCAPS_ZBUFFER = 0x20000;
const uint32_t DDSCAPS_OWNDC = 0x40000;
const uint32_t DDSCAPS_LIVEVIDEO = 0x80000;
const uint32_t DDSCAPS_HWCODEC = 0x100000;
const uint32_t DDSCAPS_MODEX = 0x200000;
const uint32_t DDSCAPS_MIPMAP = 0x400000;
const uint32_t DDSCAPS_RESERVED2 = 0x800000;
const uint32_t DDSCAPS_ALLOCONLOAD = 0x4000000;
const uint32_t DDSCAPS_VIDEOPORT = 0x8000000;
const uint32_t DDSCAPS_LOCALVIDMEM = 0x10000000;
const uint32_t DDSCAPS_NONLOCALVIDMEM = 0x20000000;
const uint32_t DDSCAPS_STANDARDVGAMODE = 0x40000000;
const uint32_t DDSCAPS_OPTIMIZED = 0x80000000;

DDSPixelformat get_pixelformat(TextureFormat fmt);
TextureFormat detect_pixelformat(const DDSPixelformat& ddspf);

struct DDSPixelformat
{
    void read(std::istream& stream);
    void write(std::ostream& stream) const;

    uint32_t size; // Struct size, always 32
    uint32_t flags;
    uint32_t four_cc; // Four character code for format identification
    uint32_t rgb_bit_count; // Bits per pixel
    uint32_t r_bitmask;
    uint32_t g_bitmask;
    uint32_t b_bitmask;
    uint32_t a_bitmask;
};

struct DDSHeader
{
    void read(std::istream& stream);
    void write(std::ostream& stream) const;

    uint32_t signature = FOURCC_DDS; // Must be "DDS "
    uint32_t size = DDS_HEADER_SIZE; // Struct size, always 124
    uint32_t flags = DDSD_REQUIRED;
    uint32_t height = 0;
    uint32_t width = 0;
    uint32_t pitch_or_linear_size = 0;
    uint32_t depth = 0;
    uint32_t mipmap_count = 1;
    uint32_t reserved1[11] = {};
    DDSPixelformat ddspf = {};
    uint32_t caps = DDSCAPS_TEXTURE;
    uint32_t caps2 = 0;
    uint32_t caps3 = 0;
    uint32_t caps4 = 0;
    uint32_t reserved2 = 0;
};
