#include <stdint.h>
#include <stddef.h>

#include "common.hpp"
#include "byteio.hpp"
#include "errors.hpp"
#include "headerfile.hpp"
#include "ddsfile.hpp"

// Documentation of DDS format:
// https://msdn.microsoft.com/en-us/library/windows/desktop/bb943982.aspx
// https://msdn.microsoft.com/en-us/library/windows/desktop/bb943984.aspx

const DDSPixelformat PIXELFORMATS[] = {
    // DXT1
    {32, DDPF_FOURCC, MAKEFOURCC('D', 'X', 'T', '1'), 0, 0, 0, 0, 0},
    // DXT3
    {32, DDPF_FOURCC, MAKEFOURCC('D', 'X', 'T', '3'), 0, 0, 0, 0, 0},
    // DXT5
    {32, DDPF_FOURCC, MAKEFOURCC('D', 'X', 'T', '5'), 0, 0, 0, 0, 0},
    // R5G6B5
    {32, DDPF_RGB, 0, 16, 0xf800, 0x07e0, 0x001f, 0x0000},
    // R5G5B5
    {32, DDPF_RGBA, 0, 16, 0x7c00, 0x03e0, 0x001f, 0x8000},
    // A4R4G4B4
    {32, DDPF_RGBA, 0, 16, 0x0f00, 0x00f0, 0x000f, 0xf000},
    // 8G8B8
    {32, DDPF_RGB, 0, 24, 0xff0000, 0x00ff00, 0x0000ff, 0x000000},
    // A8R8G8B8
    {32, DDPF_RGBA, 0, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000},
    // V8U8
    {32, DDPF_BUMPDUDV, 0, 16, 0x00ff, 0xff00, 0x0000, 0x0000},
    // CxV8U8
    {32, DDPF_FOURCC, 117, 16, 0x00ff, 0xff00, 0x0000, 0x0000},
    // A8
    {32, DDPF_ALPHA, 0, 8, 0x00, 0x00, 0x00, 0xff}
};

const size_t PIXELFORMATS_SIZE = ARRAYSIZE(PIXELFORMATS);

DDSPixelformat get_pixelformat(TextureFormat fmt)
{
    // Enums start at 400
    int index = static_cast<int>(fmt) - 400;
    if (!(0 <= index && index < static_cast<int>(PIXELFORMATS_SIZE))) {
        throw field_error("format", std::to_string(static_cast<int>(fmt)));
    }

    return PIXELFORMATS[index];
}

TextureFormat detect_pixelformat(const DDSPixelformat& ddspf)
{
    for (size_t i = 0; i < PIXELFORMATS_SIZE; i++) {
        const DDSPixelformat& ddspf_ref = PIXELFORMATS[i];

        if (ddspf.dwFlags != ddspf_ref.dwFlags) continue;
        if (ddspf.dwFlags & DDPF_FOURCC) {
            if (ddspf.dwFourCC != ddspf_ref.dwFourCC) continue;
        }
        if (ddspf.dwFlags & DDPF_RGB) {
            if (ddspf.dwRGBBitCount != ddspf_ref.dwRGBBitCount) continue;
            if (ddspf.dwRBitMask != ddspf_ref.dwRBitMask) continue;
            if (ddspf.dwGBitMask != ddspf_ref.dwGBitMask) continue;
            if (ddspf.dwBBitMask != ddspf_ref.dwBBitMask) continue;
        }
        if (ddspf.dwFlags & DDPF_ALPHAPIXELS) {
            if (ddspf.dwABitMask != ddspf_ref.dwABitMask) continue;
        }

        return static_cast<TextureFormat>(i + 400);
    }

    return TextureFormat::PC_UNKNOWN;
}



void DDSPixelformat::read(std::istream& stream)
{
    ByteReader reader(stream);

    dwSize = reader.readU32();
    if (dwSize != DDS_PIXELFORMAT_SIZE) {
        throw field_error("dwSize", std::to_string(dwSize));
    }
    dwFlags = reader.readU32();
    dwFourCC = reader.readU32();
    dwRGBBitCount = reader.readU32();
    dwRBitMask = reader.readU32();
    dwGBitMask = reader.readU32();
    dwBBitMask = reader.readU32();
    dwABitMask = reader.readU32();
}

void DDSPixelformat::write(std::ostream& stream) const
{
    ByteWriter writer(stream);

    writer.writeU32(dwSize);
    writer.writeU32(dwFlags);
    writer.writeU32(dwFourCC);
    writer.writeU32(dwRGBBitCount);
    writer.writeU32(dwRBitMask);
    writer.writeU32(dwGBitMask);
    writer.writeU32(dwBBitMask);
    writer.writeU32(dwABitMask);
}



void DDSHeader::read(std::istream& stream)
{
    ByteReader reader(stream);

    dwSignature = reader.readU32();
    if (dwSignature != FOURCC_DDS) {
        throw field_error("dwSignature", std::string(reinterpret_cast<char*>(&dwSignature), 4));
    }
    dwSize = reader.readU32();
    if (dwSize != DDS_HEADER_SIZE) {
        throw field_error("dwSize", std::to_string(dwSize));
    }
    dwFlags = reader.readU32();
    dwHeight = reader.readU32();
    dwWidth = reader.readU32();
    dwPitchOrLinearSize = reader.readU32();
    dwDepth = reader.readU32();
    dwMipMapCount = reader.readU32();
    for (size_t i = 0; i < ARRAYSIZE(dwReserved1); i++) {
        dwReserved1[i] = reader.readU32();
    }
    ddspf.read(stream);
    dwCaps = reader.readU32();
    dwCaps2 = reader.readU32();
    dwCaps3 = reader.readU32();
    dwCaps4 = reader.readU32();
    dwReserved2 = reader.readU32();
}

void DDSHeader::write(std::ostream& stream) const
{
    ByteWriter writer(stream);

    writer.writeU32(dwSignature);
    writer.writeU32(dwSize);
    writer.writeU32(dwFlags);
    writer.writeU32(dwHeight);
    writer.writeU32(dwWidth);
    writer.writeU32(dwPitchOrLinearSize);
    writer.writeU32(dwDepth);
    writer.writeU32(dwMipMapCount);
    for (size_t i = 0; i < ARRAYSIZE(dwReserved1); i++) {
        writer.writeU32(dwReserved1[i]);
    }
    ddspf.write(stream);
    writer.writeU32(dwCaps);
    writer.writeU32(dwCaps2);
    writer.writeU32(dwCaps3);
    writer.writeU32(dwCaps4);
    writer.writeU32(dwReserved2);
}
