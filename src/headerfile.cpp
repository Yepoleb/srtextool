#include <stdint.h>
#include <stddef.h>
#include <string>
#include <vector>
#include <utility> // std::move
#include <algorithm> // std::max
#include <unordered_map>

#include "ddsfile.hpp"
#include "byteio.hpp"
#include "errors.hpp"
#include "headerfile.hpp"

const std::unordered_map<TextureFormat, const char*> FORMAT_NAMES = {
    {TextureFormat::PC_UNKNOWN, "UNKNOWN"},
    {TextureFormat::PC_DXT1, "DXT1"},
    {TextureFormat::PC_DXT3, "DXT3"},
    {TextureFormat::PC_DXT5, "DXT5"},
    {TextureFormat::PC_565, "R5G6B5"},
    {TextureFormat::PC_1555, "A1R5G5B5"},
    {TextureFormat::PC_4444, "A4R4G4B4"},
    {TextureFormat::PC_888, "R8G8B8"},
    {TextureFormat::PC_8888, "A8R8G8B8"},
    {TextureFormat::PC_16_DUDV, "V8U8"},
    {TextureFormat::PC_16_DOT3_COMPRESSED, "CxV8U8"},
    {TextureFormat::PC_A8, "A8"},
};

const std::vector<const char*> ENTRY_FLAG_NAMES = {
    "ALPHA",
    "NONPOW2",
    "ALPHA_TEST",
    "CUBE_MAP",
    "INTERLEAVED_MIPS",
    "INTERLEAVED_DATA",
    "DEBUG_DATA_COPIED",
    "DYNAMIC",
    "ANIM_SHEET",
    "LINEAR_COLOR_SPACE",
    "HIGH_MIP",
    "HIGH_MIP_ELIGIBLE",
    "LINKED_TO_HIGH_MIP",
    "PERM_REGISTERED"
};

std::string get_entry_flag_names(uint16_t flags)
{
    std::string names;
    for (size_t i = 0; i < sizeof(flags) * 8; i++) {
        int mask = 1 << i;
        if (flags & mask) {
            if (!names.empty()) {
                names += ", ";
            }
            names += ENTRY_FLAG_NAMES.at(i);
        }
    }
    return names;
}

uint32_t calc_compressed_size(uint32_t width, uint32_t height, uint32_t blocksize)
{
    uint32_t width_blocks = std::max(1u, (width + 3) / 4);
    uint32_t height_blocks = std::max(1u, (height + 3) / 4);

    return width_blocks * height_blocks * blocksize;
}



void PegHeader::read(std::istream& stream)
{
    ByteReader reader(stream);

    signature = reader.readU32();
    version = reader.readS16();
    platform = reader.readS16();
    dir_block_size = reader.readU32();
    data_block_size = reader.readU32();
    num_bitmaps = reader.readU16();
    flags = reader.readU16();
    total_entries = reader.readU16();
    alignment = reader.readU16();

    if (signature != FOURCC_GEKV) {
        throw field_error("signature", std::string(reinterpret_cast<char*>(&signature), 4));
    }

    if (version != 13) {
        throw field_error("version", std::to_string(version));
    }

    if (num_bitmaps != total_entries) {
        throw field_error("num_bitmaps", std::to_string(total_entries));
    }

    for (size_t entry_i = 0; entry_i < total_entries; entry_i++) {
        PegEntry entry;
        entry.read(stream);
        entries.push_back(std::move(entry));
    }

    for (PegEntry& entry : entries) {
        entry.filename = reader.readCString();
    }
}

void PegHeader::write(std::ostream& stream) const
{
    ByteWriter writer(stream);

    writer.writeU32(signature);
    writer.writeS16(version);
    writer.writeS16(platform);
    writer.writeU32(dir_block_size);
    writer.writeU32(data_block_size);
    writer.writeU16(num_bitmaps);
    writer.writeU16(flags);
    writer.writeU16(total_entries);
    writer.writeU16(alignment);

    if (total_entries != entries.size()) {
        throw field_error("total_entries", std::to_string(total_entries));
    }
    if (total_entries != num_bitmaps) {
        throw field_error("num_bitmaps", std::to_string(num_bitmaps));
    }

    for (const PegEntry& entry : entries) {
        entry.write(stream);
    }

    for (const PegEntry& entry : entries) {
        if (entry.filename.empty()) {
            throw field_error("filename", "empty");
        }
        writer.writeCString(entry.filename);
    }
}

size_t PegHeader::size() const
{
    size_t total_size = PEGHEADER_BINSIZE;
    total_size += PEGENTRY_BINSIZE * entries.size();
    for (const PegEntry& entry : entries) {
        total_size += entry.filename.size() + 1; // String terminator
    }
    return total_size;
}

size_t PegHeader::entry_index(const std::string& name) const
{
    for (size_t entry_i = 0; entry_i < total_entries; entry_i++) {
        if (entries[entry_i].filename == name) {
            return entry_i;
        }
    }
    return SIZE_MAX;
}

void PegHeader::add_entry(PegEntry entry)
{
    entries.push_back(std::move(entry));
    num_bitmaps++;
    total_entries++;
}

bool PegHeader::remove_entry(const std::string& name)
{
    for (auto iter = entries.begin(); iter < entries.end(); iter++) {
        if (iter->filename == name) {
            entries.erase(iter);
            num_bitmaps--;
            total_entries--;
            return true;
        }
    }

    return false;
}



void PegEntry::read(std::istream& stream)
{
    ByteReader reader(stream);

    offset = reader.readS64();
    width = reader.readU16();
    height = reader.readU16();
    bm_fmt = static_cast<TextureFormat>(reader.readU16());
    pal_fmt = reader.readU16();
    anim_tiles_width = reader.readU16();
    anim_tiles_height = reader.readU16();
    num_frames = reader.readU16();
    flags = reader.readU16();
    reader.readS64(); // skip filename because it's in a different location
    pal_size = reader.readU16();
    fps = reader.readU8();
    mip_levels = reader.readU8();
    data_size = reader.readU32();
    next = reader.readU64();
    prev = reader.readU64();
    cache[0] = reader.readU32();
    cache[1] = reader.readU32();
    reader.readU64(); // padding
}

void PegEntry::write(std::ostream& stream) const
{
    ByteWriter writer(stream);

    writer.writeS64(offset);
    writer.writeU16(width);
    writer.writeU16(height);
    writer.writeU16(static_cast<uint16_t>(bm_fmt));
    writer.writeU16(pal_fmt);
    writer.writeU16(anim_tiles_width);
    writer.writeU16(anim_tiles_height);
    writer.writeU16(num_frames);
    writer.writeU16(flags);
    writer.writeS64(0); // filename is written at the end
    writer.writeU16(pal_size);
    writer.writeU8(fps);
    writer.writeU8(mip_levels);
    writer.writeU32(data_size);
    writer.writeU64(next);
    writer.writeU64(prev);
    writer.writeU32(cache[0]);
    writer.writeU32(cache[1]);
    writer.writeU64(0); // padding
}

void PegEntry::update_dds(const DDSHeader& header)
{
    width = static_cast<uint16_t>(header.dwWidth);
    height = static_cast<uint16_t>(header.dwHeight);
    bm_fmt = detect_pixelformat(header.ddspf);
    if (header.dwMipMapCount > 1) {
        mip_levels = static_cast<uint8_t>(header.dwMipMapCount);
    } else {
        mip_levels = 1;
    }
}

DDSHeader PegEntry::to_dds() const
{
    DDSHeader dds_header;
    dds_header.dwHeight = height;
    dds_header.dwWidth = width;

    if (mip_levels > 1) {
        dds_header.dwFlags |= DDSD_MIPMAPCOUNT;
        dds_header.dwMipMapCount = mip_levels;
        dds_header.dwCaps |= DDSCAPS_COMPLEX | DDSCAPS_MIPMAP;
    }

    dds_header.ddspf = get_pixelformat(bm_fmt);

    // Calculate pitch
    switch (bm_fmt) {
    case TextureFormat::PC_DXT1:
        dds_header.dwFlags |= DDSD_LINEARSIZE;
        dds_header.dwPitchOrLinearSize = calc_compressed_size(width, height, 8);
        break;
    case TextureFormat::PC_DXT3:
    case TextureFormat::PC_DXT5:
        dds_header.dwFlags |= DDSD_LINEARSIZE;
        dds_header.dwPitchOrLinearSize = calc_compressed_size(width, height, 16);
        break;
    default:
        if (dds_header.ddspf.dwRGBBitCount > 0) {
            dds_header.dwFlags |= DDSD_PITCH;
            dds_header.dwPitchOrLinearSize =
                (width * dds_header.ddspf.dwRGBBitCount + 7) / 8;
        } else {
            throw field_error("format", std::to_string(static_cast<int>(bm_fmt)));
        }
    }

    return dds_header;
}
