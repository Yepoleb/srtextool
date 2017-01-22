#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include <unordered_map>

#include "common.hpp"

struct DDSHeader;
struct PegEntry;

const uint32_t FOURCC_GEKV = MAKEFOURCC('G', 'E', 'K', 'V');

enum class TextureFormat // uint16_t
{
    PC_UNKNOWN = 0,
    PC_DXT1 = 400,
    PC_DXT3,
    PC_DXT5,
    PC_565,
    PC_1555,
    PC_4444,
    PC_888,
    PC_8888,
    PC_16_DUDV, // V8U8
    PC_16_DOT3_COMPRESSED, // CxV8U8
    PC_A8
};

const uint16_t BM_F_ALPHA = 0x1; // bitmap has alpha
const uint16_t BM_F_NONPOW2 = 0x2; // bitmap is not power of 2
const uint16_t BM_F_ALPHA_TEST = 0x4;
const uint16_t BM_F_CUBE_MAP = 0x8; // bitmap is a cube map, react appropriately on load.
const uint16_t BM_F_INTERLEAVED_MIPS = 0x10; // bitmap contains interleaved mips (they exist inside of the NEXT bitmap)
const uint16_t BM_F_INTERLEAVED_DATA = 0x20; // bitmap contains interleaved mips from the previous bitmap
const uint16_t BM_F_DEBUG_DATA_COPIED = 0x40; // used by the peg assembler only.
const uint16_t BM_F_DYNAMIC = 0x80; // bitmap was loaded dynamically (not from a peg) (runtime only)
const uint16_t BM_F_ANIM_SHEET = 0x100; // bitmap animation frames are stored in one bitmap spaced sequentially left to right
const uint16_t BM_F_LINEAR_COLOR_SPACE = 0x200; // bitmap is NOT stored in SRGB space, it is linear (not gamma corrected)
const uint16_t BM_F_HIGH_MIP = 0x400; // bitmap is a separately streamed high mip
const uint16_t BM_F_HIGH_MIP_ELIGIBLE = 0x800; // bitmap is eligible for linking up with a high mip (runtime only flag)
const uint16_t BM_F_LINKED_TO_HIGH_MIP = 0x1000; // bitmap is currently linked to a high mip (runtime only flag)
const uint16_t BM_F_PERM_REGISTERED = 0x2000; // bitmap is permanently registered. used on the PC so d3d becomes the permanent owner of the texture memory.

extern const std::unordered_map<TextureFormat, const char*> FORMAT_NAMES;
extern const std::vector<const char*> ENTRY_FLAG_NAMES;

std::string get_entry_flag_names(uint16_t flags);
uint32_t calc_compressed_size(uint32_t width, uint32_t height, uint32_t blocksize);

const size_t PEGENTRY_BINSIZE = 72;
struct PegEntry
{
    void read(std::istream& stream);
    void write(std::ostream& stream) const;
    void update_dds(const DDSHeader& header);
    DDSHeader to_dds() const;

    int64_t offset = 0; // File position of texture data
    uint16_t width = 0; // Width of texture
    uint16_t height = 0; // Height of texture
    TextureFormat bm_fmt = TextureFormat::PC_UNKNOWN; // Texture format (see TextureFormat enum).
    uint16_t pal_fmt = 0; // Not used by engine. Always 0.
    uint16_t anim_tiles_width = 1; // For animated textures using an anim sheet BM_F_ANIM_SHEET
    uint16_t anim_tiles_height = 1;
    uint16_t num_frames = 1; // Not used by engine. Always 1.
    uint16_t flags = 0; // Various flags for texture. (See BM_F_* constants).
    std::string filename; // Originally a char* to the filename. Always 0 on disk.
    uint16_t pal_size = 0; // Not used by engine. Always 0.
    uint8_t fps = 1; // Not used by engine. Always 1.
    uint8_t mip_levels = 1; // Number of mipmaps in texture + 1 for the base image.
    uint32_t data_size = 0; // Size of the texture data.
    uint64_t next; // Pointer to the next entry. Not written to disk.
    uint64_t prev; // Pointer to previous entry.
    uint32_t cache[2] = {}; // Generic texture caching data, used differently on different platforms
    // 8 bytes padding
};

const size_t PEGHEADER_BINSIZE = 24;
struct PegHeader
{
    void read(std::istream& stream);
    void write(std::ostream& stream) const;
    size_t size() const;
    size_t entry_index(const std::string& name) const;
    void add_entry(PegEntry entry);
    bool remove_entry(const std::string& name);

    uint32_t signature = FOURCC_GEKV; // Always GEKV
    int16_t version = 13; // 13 for SRTT and SRIV
    int16_t platform = 0; // 0 = PC
    uint32_t dir_block_size = 0; // Size of the header file.
    uint32_t data_block_size = 0; // Size of the data file.
    uint16_t num_bitmaps = 0; // Number of bitmaps in container.
    uint16_t flags = 0; // Various flags. Always 0?
    uint16_t total_entries = 0; // Number of entries in container. Same as num_bitmaps.
    uint16_t alignment = 16; // Always 16 for the PC.
    std::vector<PegEntry> entries;
};
