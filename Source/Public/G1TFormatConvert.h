#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#pragma once

#ifndef G1T_FORMAT_CONVERT_H
#define G1T_FORMAT_CONVERT_H

///////////////////////////////
//   GENERAL SWIZZLE CODE    //
///////////////////////////////

// All swizzle code credit to Piken (DwayneR) https://github.com/fdwr
// NOTE: Not all code is used here. A lot is for reference and debugging

template <typename T, size_t N>
constexpr size_t countof(const T(&a)[N])
{
    return N;// _countof(a);
};

enum class BlockAxis
{
    LinearX,
    LinearY,
    MortonX,
    MortonY,
};

// A single swizzle item (axis and dimension length).
struct CustomSwizzleItem
{
    
    BlockAxis blockAxis;

    // Block size in terms of multiples of the smaller previous block.
    // So a 4x4 block would have a block size of 4 since there was no block before it,
    // or rather the smallest block was a 1x1 pixel. An 8x8 block following that 4x4
    // would have a size of 2.
    // Negative block size means right-to-left flow.
    int32_t blockSize;
};

uint32_t CustomSwizzle(uint32_t width, uint32_t height, uint32_t x, uint32_t y, CustomSwizzleItem const* items, size_t itemCount)
{
    uint32_t innerBlockWidth = 1;     // Accumulated block size in pixels
    uint32_t innerBlockHeight = 1;
    uint32_t outerBlockWidth = 1;     // Accumulated size in blocks
    uint32_t outerBlockHeight = 1;
    uint32_t innerPixelCount = 1;
    uint32_t outerBlockCount = 1;
    uint32_t reducedX = x;            // Coordinate after each reduction by a block
    uint32_t reducedY = y;
    uint32_t adjustedWidth = width;
    uint32_t outputX = 0;             // Output coordinate in pixels
    uint32_t outputY = 0;
    uint32_t outputOffset = 0;

    for (size_t i = 0; i < itemCount; ++i)
    {
        auto& item = items[i];
        int32_t signedBlockSize = item.blockSize; // Size in terms of a multiple to the previous (not pixels).
        bool isRtl = signedBlockSize < 0;
        int32_t blockSize = isRtl ? -signedBlockSize : signedBlockSize;

        switch (item.blockAxis)
        {
        case BlockAxis::LinearX:
        {
            uint32_t xResidual = (reducedX % blockSize);
            if (isRtl) xResidual = blockSize - xResidual - 1;
            outputX += xResidual * innerBlockWidth;
            //outputOffset += (xResidual * innerBlockWidth;
            innerBlockWidth *= blockSize;
            reducedX /= blockSize;
            innerPixelCount *= blockSize;
        }
        break;

        case BlockAxis::LinearY:
        {
            uint32_t yResidual = (reducedY % blockSize);
            if (isRtl) yResidual = blockSize - yResidual - 1;
            outputY += yResidual * innerBlockHeight;
            innerBlockHeight *= blockSize;
            //adjustedWidth *= blockSize;
            reducedY /= blockSize;
            innerPixelCount *= blockSize;
        }
        break;

        case BlockAxis::MortonX:
        {
            uint32_t xResidual = (reducedX % blockSize);
            if (isRtl) xResidual = blockSize - xResidual - 1;
            outputX += xResidual * outerBlockCount * innerBlockWidth;
            outerBlockWidth *= blockSize;
            outerBlockCount *= blockSize;
            reducedX /= blockSize;
        }
        break;

        case BlockAxis::MortonY:
        {
            uint32_t yResidual = (reducedY % blockSize);
            if (isRtl) yResidual = blockSize - yResidual - 1;
            outputX += yResidual * outerBlockCount * innerBlockWidth;
            outerBlockHeight *= blockSize;
            outerBlockCount *= blockSize;
            reducedY /= blockSize;
        }
        break;
        }
    }
    outputX += reducedX * outerBlockWidth * outerBlockHeight * innerBlockWidth;
    outputOffset += outputX % width;
    outputY += outputX / width * innerBlockHeight;
    outputOffset += (reducedY * outerBlockHeight * innerBlockHeight + outputY) * width;

    return outputOffset;
}

template<uint32_t BlockWidth, uint32_t BlockHeight>
uint32_t BlockMxNUnswizzle(uint32_t width, uint32_t height, uint32_t x, uint32_t y)
{
    static_assert((BlockWidth & (BlockWidth - 1)) == 0, "BlockWidth must be a power of two");
    static_assert((BlockHeight & (BlockHeight - 1)) == 0, "BlockHeight must be a power of two");
    constexpr uint32_t blockPixelWidth = BlockWidth;
    constexpr uint32_t blockPixelHeight = BlockHeight;
    uint32_t sourceOffset = (y / blockPixelHeight) * (width * blockPixelHeight)
        + (y & (blockPixelHeight - 1)) * blockPixelWidth
        + (x / blockPixelWidth) * (blockPixelWidth * blockPixelHeight)
        + (x & (blockPixelWidth - 1));
    return sourceOffset;
}

// Mario Kart WII
uint32_t Block4x4Unswizzle(uint32_t width, uint32_t height, uint32_t x, uint32_t y)
{
    
    return BlockMxNUnswizzle<4, 4>(width, height, x, y);
}

// Zelda Twilight Princess GameCube
uint32_t Block8x4Unswizzle(uint32_t width, uint32_t height, uint32_t x, uint32_t y)
{
    
    return BlockMxNUnswizzle<8, 4>(width, height, x, y);
}

// Predator PSP swizzle
uint32_t Block16x8Unswizzle(uint32_t width, uint32_t height, uint32_t x, uint32_t y)
{
    return BlockMxNUnswizzle<16, 8>(width, height, x, y);

    // Equivalent to calling:
    //
    // using enum CustomSwizzleItem::BlockAxis;
    // constexpr static CustomSwizzleItem items[] =
    // {
    //     {MortonX, 16},
    //     {MortonY, 8},
    // };
    // return CustomSwizzle(width, height, x, y, items, countof(items));
}

uint32_t Block16x16Unswizzle(uint32_t width, uint32_t height, uint32_t x, uint32_t y)
{
    return BlockMxNUnswizzle<16, 16>(width, height, x, y);
}

uint32_t Block8x8Unswizzle(uint32_t width, uint32_t height, uint32_t x, uint32_t y)
{
    return BlockMxNUnswizzle<8, 8>(width, height, x, y);
}

// Hyrule Warriors Wii U
uint32_t NWiiUSwizzle(uint32_t width, uint32_t height, uint32_t x, uint32_t y)
{
    auto x2 = x ^ (y & 16) ^ ((y & 32) >> 2);
    auto y2 = y ^ (x & 8);
    x = x2;
    y = y2;
    
    constexpr static CustomSwizzleItem items[] =
    {
        {BlockAxis::MortonX, 4}, // 4x1 pixel blocks
        {BlockAxis::MortonY, 2}, // 4x2 pixel blocks
        {BlockAxis::MortonX, 2}, // 8x2 pixel blocks
        {BlockAxis::MortonY, 8}, // 8x16 pixel blocks

        // Tile offset mapping:
        //
        //     x:  0  1  2  3   4  5  6  7
        //  y:    ------------------------
        //   0  | 00 01 02 03  08 09 0A 0B
        //   1  | 04 05 06 07  0C 0D 0E 0F
        //   2  | 10 11 12 13  18 19 1A 1B
        //   3  | 14 15 16 17  1C 1D 1E 1F
        //   4  | 20 21 22 23  28 29 2A 2B
        //   5  | 24 25 26 27  2C 2D 2E 2F
        //   ...
        //   15 | 74 75 76 77  7C 7D 7E 7F
    };
    return CustomSwizzle(width, height, x, y, items, countof(items));
}

// Rise of the Rōnin PC
uint32_t RoninSwizzleDXT1(uint32_t width, uint32_t height, uint32_t x, uint32_t y)
{    
    constexpr static CustomSwizzleItem items[] =
    {
        {BlockAxis::LinearX, 512}, // 512x1
        {BlockAxis::LinearY, 4},   // 512x4
        {BlockAxis::MortonY, 64},  // 512x256
    };
    return CustomSwizzle(width, height, x, y, items, countof(items));
}

// Rise of the Rōnin PC
uint32_t RoninSwizzleDXT5(uint32_t width, uint32_t height, uint32_t x, uint32_t y)
{
    constexpr static CustomSwizzleItem items[] =
    {
        {BlockAxis::LinearX, 256}, // 256x1
        {BlockAxis::LinearY, 4},   // 512x4
        {BlockAxis::MortonY, 64},  // 512x256
    };
    return CustomSwizzle(width, height, x, y, items, countof(items));
}

// Rise of the Rōnin PC raw
uint32_t D3D12_64KBSwizzleRaw(uint32_t width, uint32_t height, uint32_t x, uint32_t y, double bitsPerPixel)
{
    int blockWidth = (int)(1024 / (bitsPerPixel / 8)); // unsure on control logic
    static CustomSwizzleItem items[] =
    {
        {BlockAxis::MortonX, 128}, // 128x1
        {BlockAxis::MortonY, 128}, // 128x128
        {BlockAxis::MortonX, 16},  // 512x256
    };
    return CustomSwizzle(width, height, x, y, items, countof(items));
}

// 3DS
uint32_t D3DStandardUnswizzle(uint32_t width, uint32_t height, uint32_t x, uint32_t y, uint32_t blockPixelSize, uint32_t texelByteSize)
{
    assert(blockPixelSize == 1 || blockPixelSize == 4);
    assert((texelByteSize & (texelByteSize - 1)) == 0); // Power of two.
    assert((texelByteSize >= 8) && (texelByteSize <= 128)); // 8bpp through 128bpp.

    // 2025-03-25 https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_texture_layout
    // - 8bpp   - XYXY XYXY YYYY XXXX
    // - 16bpp  - XYXY XYXY XYYY XXX-
    // - 32bpp  - XYXY XYXY XYYY XX--
    // - 64bpp  - XYXY XYXY XXYY X---
    // - 128bpp - XYXY XYXY XXYY ----

    // 8bpp   - XYXY XYXY YYYY XXXX
    constexpr static CustomSwizzleItem items8bpp[] =
    {
        {BlockAxis::MortonX, 16}, // 16x1
        {BlockAxis::MortonY, 32}, // 16x32
        {BlockAxis::MortonX, 2},
        {BlockAxis::MortonY, 2},
        {BlockAxis::MortonX, 2},
        {BlockAxis::MortonY, 2},
        {BlockAxis::MortonX, 2},
        {BlockAxis::MortonY, 2},
        {BlockAxis::MortonX, 2},
    };
    // 16bpp  - XYXY XYXY XYYY XXX-
    constexpr static CustomSwizzleItem items16bpp[] =
    {
        {BlockAxis::MortonX, 8}, // 8x1
        {BlockAxis::MortonY, 8}, // 8x8
        {BlockAxis::MortonX, 2},
        {BlockAxis::MortonY, 2}, // 16x16
        {BlockAxis::MortonX, 2},
        {BlockAxis::MortonY, 2}, // 32x32
        {BlockAxis::MortonX, 2},
        {BlockAxis::MortonY, 2}, // 64x64
        {BlockAxis::MortonX, 2},
        {BlockAxis::MortonY, 2}, // 128x128
        {BlockAxis::MortonX, 2}, // 256x128
    };
    // 32bpp  - XYXY XYXY XYYY XX--
    constexpr static CustomSwizzleItem items32bpp[] =
    {
        {BlockAxis::MortonX, 4},
        {BlockAxis::MortonY, 8},
        {BlockAxis::MortonX, 2},
        {BlockAxis::MortonY, 2},
        {BlockAxis::MortonX, 2},
        {BlockAxis::MortonY, 2},
        {BlockAxis::MortonX, 2},
        {BlockAxis::MortonY, 2},
        {BlockAxis::MortonX, 2},
        {BlockAxis::MortonY, 2},
        {BlockAxis::MortonX, 2},
    };
    // 64bpp  - XYXY XYXY XXYY X---
    constexpr static CustomSwizzleItem items64bpp[] =
    {
        {BlockAxis::MortonX, 2},
        {BlockAxis::MortonY, 4},
        {BlockAxis::MortonX, 4},
        {BlockAxis::MortonY, 2},
        {BlockAxis::MortonX, 2},
        {BlockAxis::MortonY, 2},
        {BlockAxis::MortonX, 2},
        {BlockAxis::MortonY, 2},
        {BlockAxis::MortonX, 2},
        {BlockAxis::MortonY, 2},
        {BlockAxis::MortonX, 2},
    };
    constexpr static CustomSwizzleItem items64bppBc[] =
    {
        {BlockAxis::LinearX, 4}, // 4x1
        {BlockAxis::LinearY, 4}, // 4x4
        {BlockAxis::MortonX, 2},
        {BlockAxis::MortonY, 4},
        {BlockAxis::MortonX, 4},
        {BlockAxis::MortonY, 2},
        {BlockAxis::MortonX, 2},
        {BlockAxis::MortonY, 2},
        {BlockAxis::MortonX, 2},
        {BlockAxis::MortonY, 2},
        {BlockAxis::MortonX, 2},
        {BlockAxis::MortonY, 2},
        {BlockAxis::MortonX, 2},
    };
    // 128bpp - XYXY XYXY XXYY ----
    constexpr static CustomSwizzleItem items128bpp[] =
    {
        {BlockAxis::MortonY, 4},
        {BlockAxis::MortonX, 4},
        {BlockAxis::MortonY, 2},
        {BlockAxis::MortonX, 2},
        {BlockAxis::MortonY, 2},
        {BlockAxis::MortonX, 2},
        {BlockAxis::MortonY, 2},
        {BlockAxis::MortonX, 2},
        {BlockAxis::MortonY, 2},
        {BlockAxis::MortonX, 2},
    };
    constexpr static CustomSwizzleItem items128bppBc[] =
    {
        {BlockAxis::LinearX, 4}, // 4x1
        {BlockAxis::LinearY, 4}, // 4x4
        {BlockAxis::MortonY, 4},
        {BlockAxis::MortonX, 4},
        {BlockAxis::MortonY, 2},
        {BlockAxis::MortonX, 2},
        {BlockAxis::MortonY, 2},
        {BlockAxis::MortonX, 2},
        {BlockAxis::MortonY, 2},
        {BlockAxis::MortonX, 2},
        {BlockAxis::MortonY, 2},
        {BlockAxis::MortonX, 2},
    };

    struct
    {
        CustomSwizzleItem const* data = nullptr;
        size_t count = 0;
    } items;

    switch (texelByteSize)
    {
    default:
    case 8:   items = { items8bpp, countof(items8bpp) };    break;
    case 16:  items = { items16bpp, countof(items16bpp) };  break;
    case 32:  items = { items32bpp, countof(items32bpp) };  break;
    case 64:
        if (blockPixelSize == 4) // 4x4 block compressed.
        {
            items = { items64bppBc, countof(items64bppBc) };
        }
        else // Pixels
        {
            items = { items64bpp, countof(items64bpp) };
        }
        break;
    case 128:
        if (blockPixelSize == 4) // 4x4 block compressed.
        {
            items = { items128bppBc, countof(items128bppBc) };
        }
        else // Pixels
        {
            items = { items128bpp, countof(items128bpp) };
        }
        break;
    }

    return CustomSwizzle(width, height, x, y, items.data, items.count);
}

uint32_t Ps2Unswizzle8bpp(uint32_t width, uint32_t height, uint32_t x, uint32_t y)
{
    // From http://ps2linux.no-ip.info/playstation2-linux.com/docs/howto/display_docef7c.html?docid=75
    //
    // this function works for the following resolutions
    // Width:       any multiple of 16 smaller than or equal to 4096
    // Height:      any multiple of 4 smaller than or equal to 4096

    // the texels must be uploaded as a 32bit texture
    // width_32bit = width_8bit / 2
    // height_32bit = height_8bit / 2
    // remember to adjust the mapping coordinates when
    // using a dimension which is not a power of two

#if 0
    const uint32_t blockLocation = (y & (~0xF)) * width + (x & (~0xF)) * 2;
    const uint32_t swapSelector = (((y + 2) >> 2) & 0x1) * 4;
    const uint32_t posY = (((y & (~3)) >> 1) + (y & 1)) & 0x7;
    const uint32_t columnLocation = posY * width * 2 + ((x + swapSelector) & 0x7) * 4;
    const uint32_t byteNumber = ((y >> 1) & 1) + ((x >> 2) & 2); // 0,1,2,3
#else
    const int32_t posY = (y & 0b1111'1111'1111'1100) | ((y & 0b0000'0000'0000'0001) << 1);
    const uint32_t swapSelector = (y + 2) & 0b0000'0000'0000'0100;
    const int32_t blockLocation = (x & 0b1111'1111'1111'0000) << 1;
    const int32_t blockLocation2 = ((x + swapSelector) & 0b0000'0000'0000'0111) << 2;
    const int32_t columnLocation = posY * width;
    const int32_t byteNumber = ((y & 0b0000'0000'0000'0010) >> 1) + ((x & 0b0000'0000'0000'1000) >> 2); // 0,1,2,3
#endif

    //auto inputOffset = y * width + x;
    auto outputOffset = blockLocation + blockLocation2 + columnLocation + byteNumber;
    return outputOffset;
}

uint32_t Ps2Unswizzle4bpp(uint32_t width, uint32_t height, uint32_t x, uint32_t y)
{
    // From http://ps2linux.no-ip.info/playstation2-linux.com/docs/howto/display_docef7c.html?docid=75
    //
    // this function works for the following resolutions
    // Width:       32, 64, 96, 128, any multiple of 128 smaller than or equal to 4096
    // Height:      16, 32, 48, 64, 80, 96, 112, 128, any multiple of 128 smaller than or equal to 4096

    // the texels must be uploaded as a 32bit texture
    // width_32bit = height_4bit / 2
    // height_32bit = width_4bit / 4
    // remember to adjust the mapping coordinates when
    // using a dimension which is not a power of two

    // swizzle
    const uint32_t pageX = x & ~0x7F;
    const uint32_t pageY = y & ~0x7F;

    const uint32_t pagesHorz = (width + 127) / 128;
    const uint32_t pagesVert = (height + 127) / 128;

    const uint32_t pageNumber = (pageY / 128) * pagesHorz + (pageX / 128);

    const uint32_t page32Y = (pageNumber / pagesVert) * 32;
    const uint32_t page32X = (pageNumber % pagesVert) * 64;

    const uint32_t pageLocation = page32Y * height * 2 + page32X * 4;

    const uint32_t locX = x & 0x7F;
    const uint32_t locY = y & 0x7F;

    const uint32_t blockLocation = ((locX & (~0x1F)) >> 1) * height + (locY & (~0xF)) * 2;
    const uint32_t swapSelector = (((y + 2) >> 2) & 0x1) * 4;
    const uint32_t posY = (((y & (~3)) >> 1) + (y & 1)) & 0x7;

    const uint32_t columnLocation = posY * height * 2 + ((x + swapSelector) & 0x7) * 4;

    const uint32_t byteNumber = (x >> 3) & 3;       // 0,1,2,3
    const uint32_t nybbleParity = (y >> 1) & 1;     // 0,1            (lower/upper 4 bits)

    //const uint32_t inputOffset = y * width + x;
    //const uint8_t uPen = (((const uint8*)pInTexels)[(inputOffset >> 1)] >> ((inputOffset & 1) * 4)) & 0xF;
    uint32_t outputByteOffset = pageLocation + blockLocation + columnLocation + byteNumber;

    //if (y < 2)
    //{
    //    auto s = std::format("x:{} y:{} offset:{} pl{} bl{} cl{} bn{} np{}\r\n", x, y, outputByteOffset * 2 + nybbleParity, pageLocation, blockLocation, columnLocation, byteNumber, nybbleParity);
    //    OutputDebugStringA(s.c_str());
    //}

    return outputByteOffset * 2 + nybbleParity;
}

uint32_t Ps2Unswizzle16bpp(uint32_t width, uint32_t height, uint32_t x, uint32_t y)
{
    // this function works for the following resolutions
    // Width:       16, 32, 48, 64, any multiple of 64 smaller than or equal to 4096
    // Height:      8, 16, 24, 32, 40, 48, 56, 64, any multiple of 64 smaller than or equal to 4096

    // the texels must be uploaded as a 32bit texture
    // width_32bit = height_16bit
    // height_32bit = width_16bit / 2
    // remember to adjust the mapping coordinates when
    // using a dimension which is not a power of two

    //const uint16 uCol = ((const uint16*)pInTexels)[y * width + x];

    const uint32_t pageX = x & (~0x3F);
    const uint32_t pageY = y & (~0x3F);

    const uint32_t pagesHorz = (width + 63) / 64;
    const uint32_t pagesVert = (height + 63) / 64;

    const uint32_t pageNumber = (pageY / 64) * pagesHorz + (pageX / 64);

    const uint32_t page32Y = (pageNumber / pagesVert) * 32;
    const uint32_t page32X = (pageNumber % pagesVert) * 64;

    const uint32_t pageLocation = (page32Y * height + page32X) * 2;

    const uint32_t locX = x & 0x3F;
    const uint32_t locY = y & 0x3F;

    const uint32_t blockLocation = (locX & (~0xF)) * height + (locY & (~0x7)) * 2;
    const uint32_t columnLocation = ((y & 0x7) * height + (x & 0x7)) * 2;

    const uint32_t shortNum = (x >> 3) & 1; // 0,1

    //uint32_t inputOffset = y * width + x;
    uint32_t outputOffset = pageLocation + blockLocation + columnLocation + shortNum;
    //((short*)pSwizTexels)[pageLocation + blockLocation + columnLocation + shortNum] = uCol;
    return outputOffset;
}

uint32_t Ps4UnswizzleBC(uint32_t width, uint32_t height, uint32_t x, uint32_t y)
{
    // block:{dxt5element4x4 x:8 y:8 x:16 y:16 x:32 y:32 x:imageWidth y:imageHeight}
    constexpr static CustomSwizzleItem items[] =
    {
        {BlockAxis::LinearX, 4}, // 4x1
        {BlockAxis::LinearY, 4}, // 4x4
        {BlockAxis::MortonX, 2}, // 8x4
        {BlockAxis::MortonY, 2}, // 8x8
        {BlockAxis::MortonX, 2}, // 16x8
        {BlockAxis::MortonY, 2}, // 16x16
        {BlockAxis::MortonX, 2}, // 32x16
        {BlockAxis::MortonY, 2}, // 32x32
    };
    return CustomSwizzle(width, height, x, y, items, countof(items));
}

uint32_t Ps4UnswizzlePixel(uint32_t width, uint32_t height, uint32_t x, uint32_t y)
{
    // D:\temp\Textures\Rocket League Textures [PS4] texture RL_Example\Texture2D\CloudDistort_T.png 512x512 32-bpp
    // D:\temp\Textures\Rocket League Textures [PS4] texture RL_Example\Texture2D\Body_SweetTooth_Thumbnail.png
    // D:\temp\Textures\StarFox 3D 3DS\9735a502.texturebin.swizzled r8g8@0h 1024x512.png
    // Works if flip image upside-down first: "D:\temp\Textures\transferred\NotoSansJP-Regular.SDF.Atlas-sharedassets0.assets-590 PS4 swizzled\NotoSansJP-Regular SDF Atlas-sharedassets0.assets-590.png"
    //      Can be achieved by uint32_t sourceOffset = CustomSwizzle(width, height, width - x - 1, y, items, countof(items));
    //      uint32_t sourceX = sourceOffset % width;
    //      sourceOffset = sourceOffset - sourceX + (width - sourceX - 1);
    constexpr static CustomSwizzleItem items[] =
    {
        {BlockAxis::MortonX, 2}, // 2x1
        {BlockAxis::MortonY, 2}, // 2x2
        {BlockAxis::MortonX, 2}, // 4x2
        {BlockAxis::MortonY, 2}, // 4x4
        {BlockAxis::MortonX, 2}, // 8x4
        {BlockAxis::MortonY, 2}, // 8x8
    };
    uint32_t sourceOffset = CustomSwizzle(width, height, x, y, items, countof(items));
#if 0 // Weird upside-down NotoSansJP-Regular
    uint32_t sourceOffset = CustomSwizzle(width, height, width - x - 1, y, items, countof(items));
    uint32_t sourceX = sourceOffset % width;
    sourceOffset = sourceOffset - sourceX + (width - sourceX - 1);
#endif
    return sourceOffset;
}

uint32_t GameCubeUnswizzleBC(uint32_t width, uint32_t height, uint32_t x, uint32_t y, uint32_t blockSize)
{
    if (blockSize == 4)
    {
        // GameCube:
        // "D:\temp\Textures\Star Wars Rogue Squadron 2 Rogue Leader\Star Wars Rogue Squadron 2 rogue leader extracted fonts StarWars @20h DXT1 512x128 swizzled.bin.png"
        constexpr static CustomSwizzleItem items[] =
        {
            {BlockAxis::LinearX, 4},
            {BlockAxis::LinearY, 4},
            {BlockAxis::MortonX, 2},
            {BlockAxis::MortonY, 2},
        };

        return CustomSwizzle(width, height, x, y, items, countof(items));
    }
    else
    {
        constexpr static CustomSwizzleItem items[] =
        {
            {BlockAxis::MortonX, 2},
            {BlockAxis::MortonY, 2},
        };

        return CustomSwizzle(width, height, x, y, items, countof(items));
    }
}

uint32_t DreamcastUnswizzle(uint32_t width, uint32_t height, uint32_t x, uint32_t y)
{
    // TODO: Unreferenced function?

    constexpr static CustomSwizzleItem items[] =
    {
        {BlockAxis::MortonY, 2}, // 2x2
        {BlockAxis::MortonX, 2}, // 2x1
        {BlockAxis::MortonY, 2}, // 2x2
        {BlockAxis::MortonX, 2}, // 4x2
        {BlockAxis::MortonY, 2}, // 4x4
        {BlockAxis::MortonX, 2}, // 8x4
        {BlockAxis::MortonY, 2}, // 8x8
        {BlockAxis::MortonX, 2}, // 16x8
        {BlockAxis::MortonY, 2}, // 16x16
        {BlockAxis::MortonX, 2}, // 32x16
        {BlockAxis::MortonY, 2}, // 32x32
        //{MortonX, 2}, //
        //{MortonY, 2}, //
        //{MortonX, 2}, //
        //{MortonY, 2}, //
        //{MortonX, 2}, //
    };
    return CustomSwizzle(width, height, x, y, items, countof(items));
}

uint32_t MortonUnswizzleXFirst(uint32_t width, uint32_t height, uint32_t x, uint32_t y)
{
    uint32_t sourceOffset = 0;
    uint32_t segmentWidth = width;
    uint32_t segmentHeight = height;
    uint32_t segmentPixelCount = segmentWidth * segmentHeight;
    while (segmentWidth > 1 || segmentHeight > 1)
    {
        if (segmentHeight >= segmentWidth)
        {
            segmentHeight >>= 1;
            segmentPixelCount >>= 1;
            if (y >= segmentHeight)
            {
                y -= segmentHeight;
                sourceOffset += segmentPixelCount; // segmentWidth * segmentHeight
            }
        }
        if (segmentWidth >= segmentHeight)
        {
            segmentWidth >>= 1;
            segmentPixelCount >>= 1;
            if (x >= segmentWidth)
            {
                x -= segmentWidth;
                sourceOffset += segmentPixelCount; // segmentWidth * segmentHeight
            }
        }
    }
    return sourceOffset;
}

uint32_t MortonUnswizzleYFirst(uint32_t width, uint32_t height, uint32_t x, uint32_t y, uint32_t blockWidth = 1, uint32_t blockHeight = 1)
{
    uint32_t sourceOffset = 0;
    uint32_t segmentWidth = width;
    uint32_t segmentHeight = height;
    uint32_t segmentPixelCount = segmentWidth * segmentHeight;
    while (segmentWidth > blockWidth || segmentHeight > blockHeight)
    {
        if (segmentWidth >= segmentHeight)
        {
            segmentWidth >>= 1;
            segmentPixelCount >>= 1;
            if (x >= segmentWidth)
            {
                x -= segmentWidth;
                sourceOffset += segmentPixelCount; // segmentWidth * segmentHeight
            }
        }
        else if (segmentHeight >= segmentWidth)
        {
            segmentHeight >>= 1;
            segmentPixelCount >>= 1;
            if (y >= segmentHeight)
            {
                y -= segmentHeight;
                sourceOffset += segmentPixelCount; // segmentWidth * segmentHeight
            }
        }
    }
    sourceOffset += y * blockWidth + x;
    return sourceOffset;
}

uint32_t Xbox2001Unswizzle(uint32_t width, uint32_t height, uint32_t x, uint32_t y)
{
    return MortonUnswizzleXFirst(width, height, x, y);
}

uint32_t PsVitaDreamcastUnswizzle(uint32_t width, uint32_t height, uint32_t x, uint32_t y, uint32_t blockPixelSize)
{
    uint32_t const blockPixelWidth = blockPixelSize;
    uint32_t const blockPixelHeight = blockPixelSize;
    auto mortonOffset = MortonUnswizzleYFirst(width, height, x, y, blockPixelWidth, blockPixelHeight);

    if (blockPixelSize == 1)
        return mortonOffset;

    // Map the Morton offset back to the linear image block's pixel (as if the image had been Morton layout within each block too).
    auto pixelsPerBlock = blockPixelWidth * blockPixelHeight;
    auto blockIndex = mortonOffset / pixelsPerBlock;
    auto blockResidual = mortonOffset % pixelsPerBlock;
    auto blocksPerRow = width / blockPixelWidth;
    auto blockX = blockIndex % blocksPerRow;
    auto blockY = blockIndex / blocksPerRow;
    auto dx = blockResidual % blockPixelWidth;
    auto dy = blockResidual / blockPixelWidth;
    return ((blockY * blockPixelHeight) + dy) * width + blockX * blockPixelWidth + dx;
}


uint32_t GetXbox360TiledOffset(uint32_t blockX, uint32_t blockY, uint32_t widthInBlocks, uint32_t bytesPerBlockLog2)
{
    assert(widthInBlocks <= 8192);
    // assert(blockX < widthInBlocks); // Brittle to assume since caller may pass in widths that are not a multiple of widthInBlocks.

    // Align to 32 blocks.
    uint32_t alignedWidthInBlocks = (widthInBlocks + 31) & ~31;
    assert(blockX < alignedWidthInBlocks);
    // Top bits of coordinates.
    uint32_t macro = ((blockX >> 5) + (blockY >> 5) * (alignedWidthInBlocks >> 5)) << (bytesPerBlockLog2 + 7);
    // Lower bits of coordinates (result is 6-bit value).
    uint32_t micro = ((blockX & 7) + ((blockY & 0xE) << 2)) << bytesPerBlockLog2;
    // Mix micro/macro + add few remaining x/y bits.
    uint32_t offset = macro + ((micro & ~0xF) << 1) + (micro & 0xF) + ((blockY & 1) << 4);
    // Mix bits again.
    return (((offset & ~0x1FF) << 3) +  // upper bits (offset bits [*-9])
        ((blockY & 16) << 7) +          // next 1 bit
        ((offset & 0x1C0) << 2) +       // next 3 bits (offset bits [8-6])
        (((((blockY & 8) >> 2) + (blockX >> 3)) & 3) << 6) + // next 2 bits
        (offset & 0x3F)                 // lower 6 bits (offset bits [5-0])
        ) >> bytesPerBlockLog2;
}

uint32_t Xbox360ConvertToLinearTexture(uint32_t width, uint32_t height, uint32_t x, uint32_t y, uint32_t blockPixelSize = 1, uint32_t texelByteSize = 1)
{
    uint32_t pixelWidth = width;
    uint32_t pixelHeight = height;

#if 0
    std::vector<uint8_t> destData(data.size());
    uint32_t blockPixelSize;
    uint32_t texelBytePitch;

    switch (textureFormat)
    {
        // 8-bpp pixels
    case GraphicFormat::gray8:
    case GraphicFormat::LinearPaletteIndex8bpp:
        blockPixelSize = 1;
        texelBytePitch = 1;
        break;
        // 16-bpp pixels
    case GraphicFormat::b4g4r4x4:
    case GraphicFormat::gray8a8:
    case GraphicFormat::b5g6r5:
        blockPixelSize = 1;
        texelBytePitch = 2;
        break;
        // 32-bpp pixels
    case GraphicFormat::b8g8r8ap8:
        blockPixelSize = 1;
        texelBytePitch = 4;
        break;
        // 64-bit 4x4 blocks
    case GraphicFormat::Bc1Dxt1:
        blockPixelSize = 4;
        texelBytePitch = 8;
        break;
        // 128-bit 4x4 blocks
    case GraphicFormat::Bc2Dxt2:
    case GraphicFormat::Bc2Dxt3:
    case GraphicFormat::Bc3Dxt4:
    case GraphicFormat::Bc3Dxt5:
        blockPixelSize = 4;
        texelBytePitch = 16;
        break;
    default:
        throw std::invalid_argument("Bad texture type!");
    }
#endif

    // Width and height in number of blocks.
    // So a 256x128 DXT1 image would be 64x32 in 4x4 blocks.
    uint32_t widthInBlocks = pixelWidth / blockPixelSize;
    uint32_t heightInBlocks = pixelHeight / blockPixelSize;

    uint32_t blockY = y / blockPixelSize;
    uint32_t blockX = x / blockPixelSize;
    uint32_t blockDy = y % blockPixelSize;
    uint32_t blockDx = x % blockPixelSize;
    uint32_t blockOffset = blockY * widthInBlocks + blockX;

#if 0
    // Given
    uint32 tileX = XGAddress2DTiledX(blockOffset, widthInBlocks, texelByteSize);
    uint32 tileY = XGAddress2DTiledY(blockOffset, widthInBlocks, texelByteSize);
#elif 0
    // Given an image block offset in terms of the swizzled image,
    // compute where it would end up as linear offset. This is the
    // opposite of the normal inquiry where you have an x,y
    // coordinate and want to find the element offset in swizzled
    // memory.
    auto [tileX, tileY] = XGAddress2DTiledXY(blockOffset, widthInBlocks, texelByteSize);
    //uint32 linearSourceOffset = y * width + x; // OR blockY * widthInBlocks + blockX;
#elif 1
    uint32_t texelByteSizeLog2 = (texelByteSize >> 2) + ((texelByteSize >> 1) >> (texelByteSize >> 2));
    uint32_t tileOffset = GetXbox360TiledOffset(blockX, blockY, widthInBlocks, texelByteSizeLog2);
    uint32_t tileY = tileOffset / max(widthInBlocks, 1u);
    uint32_t tileX = tileOffset % max(widthInBlocks, 1u);
#endif
    // Compute the offset of the pixel (indices, not bytes).
    uint32_t destOffset = (tileY * blockPixelSize + blockDy) * pixelWidth + tileX * blockPixelSize + blockDx;

#if 0
    // This loop works in terms of the source and computes the destination instead.
    for (uint32 j = 0; j < heightInBlocks; j++)
    {
        for (uint32 i = 0; i < widthInBlocks; i++)
        {
            uint32 blockOffset = j * widthInBlocks + i;

            uint32 x = XGAddress2DTiledX(blockOffset, widthInBlocks, texelBytePitch);
            uint32 y = XGAddress2DTiledY(blockOffset, widthInBlocks, texelBytePitch);

            uint32 srcByteOffset = j * widthInBlocks * texelBytePitch + i * texelBytePitch;
            uint32 destByteOffset = y * widthInBlocks * texelBytePitch + x * texelBytePitch;
            //TODO: ConvertToLinearTexture apparently breaks on textures with a height of 64...
            memcpy(&destData[destByteOffset], &data[srcByteOffset], texelBytePitch);
        }
    }
#endif

    return destOffset;
}

enum class SwizzleMode
{
    Linear,
    Block4x4,
    Block8x4,
    Block8x8,
    Block16x8,
    Block16x16,
    D3DStandard8bpp,
    D3DStandard16bpp,
    D3DStandard32bpp,
    D3DStandard64bpp,
    D3DStandard64bppBc,
    D3DStandard128bpp,
    D3DStandard128bppBc,
    Ps2Unswizzle4bpp,
    Ps2Unswizzle8bpp,
    Ps2Unswizzle16bpp,
    PS4Bc,
    PS4Pixel,
    PsVitaPixel,
    PsVitaBc,
    GameCubeBc,
    Xbox2001Morton,
    Xbox360_8bpp,
    Xbox360_16bpp,
    Xbox360_32bpp,
    Xbox360_BC1,
    Xbox360_BC3,
    NWiiU,
    RoninDXT1,
    RoninDXT5,
    D3D12_64KBRaw,
    Test
};

uint32_t TestSwizzle(uint32_t width, uint32_t height, uint32_t x, uint32_t y)
{
    // dummy function for swizzle testing
    return 0;
}

uint32_t GetSwizzledOffset(SwizzleMode swizzleMode, uint32_t width, uint32_t height, uint32_t x, uint32_t y, double bitsPerPixel)
{
    switch (swizzleMode)
    {
    default:
    case SwizzleMode::Linear:              return y * width + x;
    case SwizzleMode::Block4x4:            return Block4x4Unswizzle(width, height, x, y);
    case SwizzleMode::Block8x4:            return Block8x4Unswizzle(width, height, x, y);
    case SwizzleMode::Block8x8:            return Block8x8Unswizzle(width, height, x, y);
    case SwizzleMode::Block16x8:           return Block16x8Unswizzle(width, height, x, y);
    case SwizzleMode::Block16x16:          return Block16x16Unswizzle(width, height, x, y);
    case SwizzleMode::D3DStandard8bpp:     return D3DStandardUnswizzle(width, height, x, y, 1, 8);
    case SwizzleMode::D3DStandard16bpp:    return D3DStandardUnswizzle(width, height, x, y, 1, 16);
    case SwizzleMode::D3DStandard32bpp:    return D3DStandardUnswizzle(width, height, x, y, 1, 32);
    case SwizzleMode::D3DStandard64bpp:    return D3DStandardUnswizzle(width, height, x, y, 1, 64);
    case SwizzleMode::D3DStandard64bppBc:  return D3DStandardUnswizzle(width, height, x, y, 4, 64);
    case SwizzleMode::D3DStandard128bpp:   return D3DStandardUnswizzle(width, height, x, y, 1, 128);
    case SwizzleMode::D3DStandard128bppBc: return D3DStandardUnswizzle(width, height, x, y, 4, 128);
    case SwizzleMode::Ps2Unswizzle4bpp:    return Ps2Unswizzle4bpp(width, height, x, y);
    case SwizzleMode::Ps2Unswizzle8bpp:    return Ps2Unswizzle8bpp(width, height, x, y);
    case SwizzleMode::Ps2Unswizzle16bpp:   return Ps2Unswizzle16bpp(width, height, x, y);
    case SwizzleMode::PS4Bc:               return Ps4UnswizzleBC(width, height, x, y);
    case SwizzleMode::PS4Pixel:            return Ps4UnswizzlePixel(width, height, x, y);
    case SwizzleMode::PsVitaPixel:         return PsVitaDreamcastUnswizzle(width, height, x, y, /*blockPixelSize*/ 1);
    case SwizzleMode::PsVitaBc:            return PsVitaDreamcastUnswizzle(width, height, x, y, /*blockPixelSize*/ 4);
    case SwizzleMode::GameCubeBc:          return GameCubeUnswizzleBC(width, height, x, y, /*blockPixelSize*/ 4);
    case SwizzleMode::Xbox2001Morton:      return Xbox2001Unswizzle(width, height, x, y);
    case SwizzleMode::Xbox360_8bpp:        return Xbox360ConvertToLinearTexture(width, height, x, y, /*blockPixelSize*/ 1, /*texelByteSize*/ 1);
    case SwizzleMode::Xbox360_16bpp:       return Xbox360ConvertToLinearTexture(width, height, x, y, /*blockPixelSize*/ 1, /*texelByteSize*/ 2);
    case SwizzleMode::Xbox360_32bpp:       return Xbox360ConvertToLinearTexture(width, height, x, y, /*blockPixelSize*/ 1, /*texelByteSize*/ 4);
    case SwizzleMode::Xbox360_BC1:         return Xbox360ConvertToLinearTexture(width, height, x, y, /*blockPixelSize*/ 4, /*texelByteSize*/ 8);
    case SwizzleMode::Xbox360_BC3:         return Xbox360ConvertToLinearTexture(width, height, x, y, /*blockPixelSize*/ 4, /*texelByteSize*/ 16);
    case SwizzleMode::NWiiU:               return NWiiUSwizzle(width, height, x, y);
    case SwizzleMode::RoninDXT1:           return RoninSwizzleDXT1(width, height, x, y);
    case SwizzleMode::RoninDXT5:           return RoninSwizzleDXT5(width, height, x, y);
    case SwizzleMode::D3D12_64KBRaw:       return D3D12_64KBSwizzleRaw(width, height, x, y, bitsPerPixel);
    case SwizzleMode::Test:                return TestSwizzle(width, height, x, y);
    }
}

void SwizzleMasterFunction(BYTE *src, BYTE *dst, uint32_t width, uint32_t height, double bitsPerPixel, SwizzleMode mode)
{
    uint32_t byteCount = (uint32_t)((width * height * bitsPerPixel) / 8);
    size_t bytesPerPixel = (size_t)(bitsPerPixel / 8);
    uint32_t sourcePixelCount = width * height;
    std::vector<uint32_t> imageSwizzledOffsets_(width * height, 0);

    for (uint32_t y = 0, linearIndex = 0; y < height; ++y)
    {
        for (uint32_t x = 0; x < width; ++x, ++linearIndex)
        {
            uint32_t swizzledIndex = GetSwizzledOffset(mode, width, height, x, y, bitsPerPixel);
            uint32_t sourceIndex = swizzledIndex;
            uint32_t destIndex = linearIndex;
            if (destIndex < byteCount)
            {
                imageSwizzledOffsets_[destIndex] = sourceIndex;
            }
        }
    }

    for (uint32_t y = 0, linearIndex = 0; y < height; ++y)
    {
        for (uint32_t x = 0; x < width; ++x, ++linearIndex)
        {
            // Copy the color between swizzled/unswizzled textures.
            uint32_t sourceIndex = imageSwizzledOffsets_[linearIndex];
            uint32_t destIndex = linearIndex;
            uint32_t destStart = (uint32_t)(destIndex * bytesPerPixel);
            if (sourceIndex < sourcePixelCount &&
                destIndex < byteCount)
            {
                // change this to a memcpy instead
                //dst[destIndex] = src[sourceIndex];
                memcpy(&dst[destIndex * bytesPerPixel], &src[sourceIndex * bytesPerPixel], bytesPerPixel);
            }
        }
    }
}

void UnswizzleNWiiU(BYTE* src, BYTE* dst, uint32_t width, uint32_t height, double bitsPerPixel)
{
    return SwizzleMasterFunction(src, dst, width, height, bitsPerPixel, SwizzleMode::NWiiU);
}

void DeswizzleD3D12_64KBRaw(BYTE* src, BYTE* dst, uint32_t width, uint32_t height, double bitsPerPixel)
{
    return SwizzleMasterFunction(src, dst, width, height, bitsPerPixel, SwizzleMode::D3D12_64KBRaw);
}

///////////////////////////////
//   GENERAL CONVERT CODE    //
///////////////////////////////

// General format converts

// Untiles DXT compressed blocks
void DeswizzleD3D12_64KBDXT(
    const BYTE* src,
    BYTE* dst,
    uint32_t blockWidth,
    uint32_t blockHeight,
    double bytes_per_block)
{
    const uint32_t tile_size_bytes = 64 * 1024; // 64KB
    const uint32_t linear_tile_size = 1024;
    const uint32_t tile_row_bytes = linear_tile_size;
    const uint32_t tile_width = (uint32_t)(linear_tile_size / bytes_per_block);
    const uint32_t tile_height = 64;

    uint32_t tiles_x = (blockWidth + tile_width - 1) / tile_width;
    uint32_t tiles_y = (blockHeight + tile_height - 1) / tile_height;

    for (uint32_t ty = 0; ty < tiles_y; ++ty)
    {
        for (uint32_t tx = 0; tx < tiles_x; ++tx)
        {
            uint32_t tile_index = ty * tiles_x + tx;
            const uint8_t* tile_base = src + tile_index * tile_size_bytes;

            for (uint32_t row = 0; row < tile_height; ++row)
            {
                uint32_t y = ty * tile_height + row;
                if (y >= blockHeight) continue;

                const uint8_t* tile_row_src = tile_base + row * tile_row_bytes;
                uint8_t* dst_row = dst + (uint32_t)(y * blockWidth * bytes_per_block);
                uint32_t x_offset = tx * tile_width;
                if (x_offset >= blockWidth) continue;

                uint32_t copy_bytes = (uint32_t)(x_offset + tile_width > blockWidth)
                    ? (uint32_t)((blockWidth - x_offset) * bytes_per_block)
                    : (uint32_t)(tile_width * bytes_per_block);

                memcpy(dst_row + (uint32_t)(x_offset * bytes_per_block), tile_row_src, copy_bytes);
            }
        }
    }
}

void ConvertDepth24Stencil8_to_24_24_24_8(const BYTE* src, BYTE* dst, uint32_t width, uint32_t height)
{
    size_t pixelCount = width * height;
    for (size_t i = 0; i < pixelCount; ++i)
    {
        uint32_t packed;
        memcpy(&packed, src + i * 4, sizeof(uint32_t));

        // Write depth replicated 3 times (9 bytes total)
        for (int j = 0; j < 3; ++j)
        {
            dst[i * 10 + j * 3 + 0] = (packed & 0xFF);
            dst[i * 10 + j * 3 + 1] = ((packed >> 8) & 0xFF);
            dst[i * 10 + j * 3 + 2] = ((packed >> 16) & 0xFF);
        }

        // Write stencil (1 byte)
        dst[i * 10 + 9] = ((packed >> 24) & 0xFF);
    }
}

void ConvertDepth32_to_32_32_32(const BYTE *src, BYTE *dst, uint32_t width, uint32_t height)
{
    size_t pixelCount = width * height;

    const uint32_t* src16 = (const uint32_t*)src;

    for (size_t i = 0; i < pixelCount; ++i)
    {
        uint32_t value = src16[i];
        memcpy(&dst[i * 3 * sizeof(uint32_t)], &value, sizeof(uint32_t));
        memcpy(&dst[i * 3 * sizeof(uint32_t) + sizeof(uint32_t)], &value, sizeof(uint32_t));
        memcpy(&dst[i * 3 * sizeof(uint32_t) + 2 * sizeof(uint32_t)], &value, sizeof(uint32_t));
    }
}

void ConvertDepth16_to_16_16_16(const BYTE *src, BYTE *dst, uint32_t width, uint32_t height)
{
    size_t pixelCount = width * height;

    const uint16_t* src16 = (const uint16_t*)src;

    for (size_t i = 0; i < pixelCount; ++i)
    {
        uint16_t value = src16[i];
        memcpy(&dst[i * 3 * sizeof(uint16_t)], &value, sizeof(uint16_t));
        memcpy(&dst[i * 3 * sizeof(uint16_t) + sizeof(uint16_t)], &value, sizeof(uint16_t));
        memcpy(&dst[i * 3 * sizeof(uint16_t) + 2 * sizeof(uint16_t)], &value, sizeof(uint16_t));
    }
}

void ConvertDepth8_to_8_8_8(const BYTE *src, BYTE *dst, uint32_t width, uint32_t height)
{
    size_t pixelCount = width * height;

    for (size_t i = 0; i < pixelCount; ++i)
    {
        uint8_t value = src[i];
        memcpy(&dst[i * 3 * sizeof(uint8_t)], &value, sizeof(uint8_t));
        memcpy(&dst[i * 3 * sizeof(uint8_t) + sizeof(uint8_t)], &value, sizeof(uint8_t));
        memcpy(&dst[i * 3 * sizeof(uint8_t) + 2 * sizeof(uint8_t)], &value, sizeof(uint8_t));
    }
}

// Convert GL_R16I (signed 16-bit) to GL_R16 (unsigned 16-bit scaled)
void ConvertR16I_to_R16(const BYTE *src, BYTE *dst, uint32_t width, uint32_t height)
{
    int16_t* src16 = (int16_t*)src;
    uint16_t* dst16 = (uint16_t*)dst;
    size_t pixelCount = width * height;

    for (size_t i = 0; i < pixelCount; ++i)
    {
        // Convert signed to unsigned using biasing
        dst16[i] = (uint16_t)(src16[i] + 32768);
    }
}

// Convert GL_R32I (signed 32-bit) to GL_R32UI (unsigned 32-bit scaled)
void ConvertR32I_to_R32UI(const BYTE *src, BYTE *dst, uint32_t width, uint32_t height)
{
    int32_t *src32 = (int32_t*)src;
    uint32_t *dst32 = (uint32_t*)dst;
    size_t pixelCount = width * height;

    for (size_t i = 0; i < pixelCount; ++i)
    {
        // Convert signed to unsigned using biasing
        dst32[i] = (uint32_t)(src32[i] + 2147483648U);
    }
}

// Helper to decode 11-bit or 10-bit float values into 32-bit float
static float decode_packed_float(uint32_t packed, int bits)
{
    uint32_t mantissa_bits = bits - 5; // 5-bit exponent
    uint32_t mantissa_mask = (1 << mantissa_bits) - 1;
    uint32_t exponent_mask = (1 << 5) - 1;

    uint32_t mantissa = packed & mantissa_mask;
    uint32_t exponent = (packed >> mantissa_bits) & exponent_mask;

    if (exponent == 0 && mantissa == 0)
        return 0.0f; // zero

    float m = (float)mantissa / (1 << mantissa_bits);
    float e = powf(2.0f, (float)((int)exponent - 15));
    return m * e;
}

// Converts a buffer of GL_R11F_G11F_B10F to GL_RGB32F
void ConvertR11FG11FB10F_to_RGB32F(const BYTE *src_byte, BYTE *dst_byte, uint32_t width, uint32_t height)
{
    const uint32_t *src = (const uint32_t*)src_byte;
    float *dst = (float*)dst_byte;
    uint32_t pixel_count = width * height;
    for (size_t i = 0; i < pixel_count; ++i)
    {
        uint32_t packed = src[i];

        // Extract components
        uint32_t b10 = packed & 0x3FF;
        uint32_t g11 = (packed >> 10) & 0x7FF;
        uint32_t r11 = (packed >> 21) & 0x7FF;

        // Decode each to float
        float r = decode_packed_float(r11, 11);
        float g = decode_packed_float(g11, 11);
        float b = decode_packed_float(b10, 10);

        dst[i * 3 + 0] = r;
        dst[i * 3 + 1] = g;
        dst[i * 3 + 2] = b;
    }
}

// Converts 10-bit float (assumed format: 1 sign bit, 5 exponent bits, 4 mantissa bits)
static float decode_f10(uint16_t val) {
    uint16_t sign = (val >> 9) & 0x1;
    uint16_t exponent = (val >> 4) & 0x1F;
    uint16_t mantissa = val & 0xF;

    if (exponent == 0) {
        // Subnormal
        return ldexpf((float)mantissa, -14);
    }
    else if (exponent == 0x1F) {
        // Inf or NaN
        return INFINITY;
    }
    else {
        float value = 1.0f + mantissa / 16.0f;
        return ldexpf(value, exponent - 15) * (sign ? -1.0f : 1.0f);
    }
}

void ConvertU2F10F10F10_to_RGBA32F(const BYTE *src_byte, BYTE *dst_byte, uint32_t width, uint32_t height)
{
    const uint32_t *src = (const uint32_t*)src_byte;
    float *dst = (float*)dst_byte;
    uint32_t pixel_count = width * height;
    for (size_t i = 0; i < pixel_count; ++i) {
        uint32_t pixel = src[i];

        // Extract ABGR bits
        uint32_t r_bits = (pixel >> 0) & 0x3FF;
        uint32_t g_bits = (pixel >> 10) & 0x3FF;
        uint32_t b_bits = (pixel >> 20) & 0x3FF;
        uint32_t a_bits = (pixel >> 30) & 0x3;

        // Decode channels
        float r = decode_f10((uint16_t)r_bits);
        float g = decode_f10((uint16_t)g_bits);
        float b = decode_f10((uint16_t)b_bits);
        float a = a_bits / 3.0f;

        // Store as RGBA32F
        dst[i * 4 + 0] = r;
        dst[i * 4 + 1] = g;
        dst[i * 4 + 2] = b;
        dst[i * 4 + 3] = a;
    }
}

void ConvertDepth32Stencil8_to_RGBA32f(const BYTE *src, BYTE *dst, uint32_t width, uint32_t height)
{
    const float *depth_stencil_array = (float*)src;
    float *rgba_array = (float*)dst;
    // Calculate the number of bytes required for a single float and GL_UNSIGNED_INT_24_8 value.
    size_t bytes_per_float = sizeof(float);
    size_t bytes_per_int = 4; // We know that is the size of an unsigned int on this machine.

    // Convert each (depth, stencil) pair into a 4-component RGBA32F floating point texture buffer
    for (uint32_t y = 0; y < height; ++y)
    {
        for (uint32_t x = 0; x < width; ++x)
        {
            size_t index_in_depth_stencil_array = y * (width * bytes_per_int) + x * bytes_per_int;
            float depth_value, stencil_value;

            // Extract the depth value from the input array.
            depth_value = *(float*)((char*)depth_stencil_array + index_in_depth_stencil_array);

            // Extract the stencil value from the input array. 
            size_t index_of_stencil_byte = (index_in_depth_stencil_array % bytes_per_int) + 3;
            char* pointer_to_bytes = (char*)depth_stencil_array + index_in_depth_stencil_array;

            // Copy the low byte of the stencil into a float, then scale it to 0..1.
            stencil_value = *(float*)((char*)(pointer_to_bytes + index_of_stencil_byte)) / 255.0f;

            // Assign the depth value to RGB and the stencil value to A
            rgba_array[(y * width * 4) + (x * 4)] = depth_value;   // R component, same as depth
            rgba_array[(y * width * 4) + (x * 4) + 1] = depth_value; // G component, same as depth
            rgba_array[(y * width * 4) + (x * 4) + 2] = depth_value; // B component, same as depth
            rgba_array[(y * width * 4) + (x * 4) + 3] = stencil_value;// A component, copy of the stencil value.
        }
    }

}

// Here for when we find these formats
// Convert swizzled P4 (4-bit indexed) texture to RGBA32
void ConvertP4_to_RGBA32(const BYTE* src, const uint32_t* palette, BYTE* dstByte, uint32_t width, uint32_t height)
{
    size_t pixelCount = width * height;

    uint32_t* dst = (uint32_t*)dstByte;

    for (size_t i = 0; i < pixelCount; ++i)
    {
        uint8_t byte = src[i / 2];
        uint8_t index = (i & 1) ? (byte & 0x0F) : (byte >> 4);
        dst[i] = palette[index];
    }
}

// Here for when we find these formats
// Convert swizzled P8 (8-bit indexed) texture to RGBA32
void ConvertP8_to_RGBA32(const BYTE* src, const uint32_t* palette, BYTE* dstByte, size_t width, size_t height)
{
    size_t pixelCount = width * height;

    uint32_t* dst = (uint32_t*)dstByte;

    for (size_t i = 0; i < pixelCount; ++i)
    {
        uint8_t index = src[i];
        dst[i] = palette[index];
    }
}

// Converts GX_TF_CI14 indexed texture to RGBA32
// src: pointer to CI14 texture data (2 bytes per texel)
// tlut: palette of 16,384 colors (uint32_t ABGR format)
// dst: output buffer (1 uint32_t per pixel)
void ConvertP14_to_RGBA32(const BYTE* src, const uint32_t* tlut, BYTE* dstByte, size_t width, size_t height)
{
    size_t pixelCount = width * height;

    uint32_t* dst = (uint32_t*)dstByte;

    for (size_t i = 0; i < pixelCount; ++i)
    {
        uint16_t index = *(const uint16_t*)(src + i * 2);
        index &= 0x3FFF; // Only lower 14 bits are valid
        dst[i] = tlut[index];
    }
}

// Apply sRGB gamma curve to a linear channel value (0–255)
static uint8_t linear_to_srgb(uint8_t c)
{
    float fc = c / 255.0f;
    float srgb = (fc <= 0.0031308f)
        ? fc * 12.92f
        : 1.055f * powf(fc, 1.0f / 2.4f) - 0.055f;
    int result = (int)(srgb * 255.0f + 0.5f);
    return (uint8_t)(result < 0 ? 0 : result > 255 ? 255 : result);
}

static uint8_t srgb_to_linear(uint8_t c)
{
    float fc = c / 255.0f;
    float linear = (fc <= 0.04045f)
        ? fc / 12.92f
        : powf((fc + 0.055f) / 1.055f, 2.4f);
    int result = (int)(linear * 255.0f + 0.5f);
    return (uint8_t)(result < 0 ? 0 : result > 255 ? 255 : result);
}

// Converts a linear RGBA32 buffer to sRGBA in-place
void RGBA_to_sRGBA(BYTE* src, uint32_t pixelCount)
{
    uint32_t* pixels = (uint32_t*)src;
    for (size_t i = 0; i < pixelCount; ++i)
    {
        uint32_t color = pixels[i];

        uint8_t r = (color >> 0) & 0xFF;
        uint8_t g = (color >> 8) & 0xFF;
        uint8_t b = (color >> 16) & 0xFF;
        uint8_t a = (color >> 24) & 0xFF;

        r = linear_to_srgb(r);
        g = linear_to_srgb(g);
        b = linear_to_srgb(b);

        pixels[i] = (a << 24) | (b << 16) | (g << 8) | r;
    }
}

// Converts a sRGBA32 buffer to linear RGBA32 in-place
void sRGBA_to_RGBA(BYTE* src, uint32_t pixelCount)
{
    uint32_t* pixels = (uint32_t*)src;
    for (size_t i = 0; i < pixelCount; ++i)
    {
        uint32_t color = pixels[i];

        uint8_t r = (color >> 0) & 0xFF;
        uint8_t g = (color >> 8) & 0xFF;
        uint8_t b = (color >> 16) & 0xFF;
        uint8_t a = (color >> 24) & 0xFF;

        r = srgb_to_linear(r);
        g = srgb_to_linear(g);
        b = srgb_to_linear(b);

        pixels[i] = (a << 24) | (b << 16) | (g << 8) | r;
    }
}

///////////////////////////////
//    WII U SWIZZLE CODE     //
///////////////////////////////

// Wii U swizzle code source https://github.com/KillzXGaming/Switch-Toolbox

// Source: https://github.com/KillzXGaming/Switch-Toolbox/blob/master/Switch_Toolbox_Library/Texture%20Decoding/Wii%20U/GX2.cs

// Flags struct
typedef struct {
    uint32_t value;
} Gx2Flags;

// TileInfo struct
typedef struct {
    uint32_t banks;
    uint32_t bankWidth;
    uint32_t bankHeight;
    uint32_t macroAspectRatio;
    uint32_t tileSplitBytes;
    uint32_t pipeConfig;
} Gx2TileInfo;

// surfaceIn struct
typedef struct {
    uint32_t size;
    uint32_t tileMode;
    uint32_t format;
    uint32_t bpp;
    uint32_t numSamples;
    uint32_t width;
    uint32_t height;
    uint32_t numSlices;
    uint32_t slice;
    uint32_t mipLevel;
    Gx2Flags flags;
    uint32_t numFrags;
    uint32_t tileType;
    Gx2TileInfo pTileInfo;
    int tileIndex;
    uint32_t blockWidth;
    uint32_t blockHeight;
    uint32_t elemMode;
} Gx2SurfaceIn;

// surfaceOut struct
typedef struct {
    uint32_t size;
    uint32_t pitch;
    uint32_t height;
    uint32_t depth;
    uint32_t surfSize;
    uint32_t tileMode;
    uint32_t baseAlign;
    uint32_t pitchAlign;
    uint32_t heightAlign;
    uint32_t depthAlign;
    uint32_t bpp;
    uint32_t pixelPitch;
    uint32_t pixelHeight;
    uint32_t pixelBits;
    uint32_t sliceSize;
    uint32_t pitchTileMax;
    uint32_t heightTileMax;
    uint32_t sliceTileMax;
    uint32_t tileType;
    Gx2TileInfo pTileInfo;
    int tileIndex;
    uint32_t format;
    uint32_t blockWidth;
    uint32_t blockHeight;
    uint32_t elemMode;
} Gx2SurfaceOut;

typedef struct {
    uint32_t dim;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t numMips;
    uint32_t firstSlice;
    uint32_t numSlices;
    uint32_t format;
    uint32_t aa;
    uint32_t use;
    int resourceFlags;
    uint32_t imageSize;
    uint32_t imagePtr;
    int MemPtr;
    uint32_t mipSize;
    uint32_t mipPtr;
    uint32_t tileMode;
    uint32_t swizzle;
    uint32_t mip_swizzle; //Used for botw Tex2
    uint32_t alignment;
    uint32_t pitch;
    uint32_t bpp;
    uint32_t imageCount;
    uint32_t firstMip;
    uint32_t numArray;
    BYTE* data;
    BYTE* mipData;
    BYTE* retData;

    int32_t blockWidth;
    int32_t blockHeight;
    uint32_t elemMode;

    std::vector <uint32_t> mipOffset;
    uint8_t compSel[4];
    uint32_t texRegs[5];
} GX2Surface;



uint32_t nextPow2(uint32_t dim)
{
    uint32_t newDim = 1;
    if (dim < 0x7FFFFFFF)
    {
        while (newDim < dim)
            newDim *= 2;
    }
    else
        newDim = 0x80000000;

    return newDim;
}

uint32_t hwlComputeMipLevel(Gx2SurfaceIn* pIn, Gx2SurfaceOut* pOut)
{
    uint32_t handled = 0;

    if (49 <= pIn->format && pIn->format <= 55)
    {
        if (pIn->mipLevel != 0)
        {
            uint32_t width = pIn->width;
            uint32_t height = pIn->height;
            uint32_t slices = pIn->numSlices;

            if (((pIn->flags.value >> 12) & 1) != 0)
            {
                uint32_t widtha = width >> (int)pIn->mipLevel;
                uint32_t heighta = height >> (int)pIn->mipLevel;

                if (((pIn->flags.value >> 4) & 1) == 0)
                    slices >>= (int)pIn->mipLevel;

                width = max(1, widtha);
                height = max(1, heighta);
                slices = max(1, slices);
            }

            pIn->width = nextPow2(width);
            pIn->height = nextPow2(height);
            pIn->numSlices = slices;
        }

        handled = 1;
    }
    return handled;
}

uint32_t powTwoAlign(uint32_t x, uint32_t align)
{
    return ~(align - 1) & (x + align - 1);
}

void computeMipLevel(Gx2SurfaceIn* pIn, Gx2SurfaceOut* pOut)
{
    uint32_t slices = 0;
    uint32_t height = 0;
    uint32_t width = 0;
    uint32_t hwlHandled = 0;

    if (49 <= pIn->format && pIn->format <= 55 && (pIn->mipLevel == 0 || ((pIn->flags.value >> 12) & 1) != 0))
    {
        pIn->width = powTwoAlign(pIn->width, 4);
        pIn->height = powTwoAlign(pIn->height, 4);
    }

    //if (bDebugLog)
    //{
    //    LogDebug("-------------------------------------------\n");
    //    LogDebug(" hwlComputeMipLevel\n");
    //    LogDebug("-------------------------------------------\n");
    //    LogDebug(" pIn.width %d\n", pIn->width);
    //    LogDebug(" pIn.height %d\n", pIn->height);
    //    LogDebug(" pIn.numSlices %d\n", pIn->numSlices);
    //}

    hwlHandled = hwlComputeMipLevel(pIn, pOut);

    //if (bDebugLog)
    //{
    //    LogDebug(" Output:\n");
    //    LogDebug(" pIn.width %d\n", pIn->width);
    //    LogDebug(" pIn.height %d\n", pIn->height);
    //    LogDebug(" pIn.numSlices %d\n", pIn->numSlices);
    //    LogDebug("-------------------------------------------\n");
    //}

    if (hwlHandled == 0 && pIn->mipLevel != 0 && ((pIn->flags.value >> 12) & 1) != 0)
    {
        width = max(1, pIn->width >> (int)pIn->mipLevel);
        height = max(1, pIn->height >> (int)pIn->mipLevel);
        slices = max(1, pIn->numSlices);

        if (((pIn->flags.value >> 4) & 1) == 0)
            slices = max(1, slices >> (int)pIn->mipLevel);

        if (pIn->format != 47 && pIn->format != 48)
        {
            width = nextPow2(width);
            height = nextPow2(height);
            slices = nextPow2(slices);
        }
        pIn->width = width;
        pIn->height = height;
        pIn->numSlices = slices;
    }
}

uint32_t adjustSurfaceInfo(uint32_t elemMode, uint32_t expandX, uint32_t expandY, uint32_t bpp, uint32_t width, uint32_t height, Gx2SurfaceIn* pIn, Gx2SurfaceOut* pOut)
{
    uint32_t bBCnFormat = 0;
    uint32_t widtha, heighta;

    switch (elemMode)
    {
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
        if (bpp != 0)
            bBCnFormat = 1;

        break;
    }

    if (width != 0 && height != 0)
    {
        if (expandX > 1 || expandY > 1)
        {
            if (elemMode == 4)
            {
                widtha = expandX * width;
                heighta = expandY * height;
            }
            else if (bBCnFormat != 0)
            {
                widtha = width / expandX;
                heighta = height / expandY;
            }
            else
            {
                widtha = (width + expandX - 1) / expandX;
                heighta = (height + expandY - 1) / expandY;
            }

            pIn->width = max(1, widtha);
            pIn->height = max(1, heighta);
        }
    }

    if (bpp != 0)
    {
        switch (elemMode)
        {
        case 4:
            pIn->bpp = bpp / expandX / expandY;
            break;

        case 5:
        case 6:
            pIn->bpp = expandY * expandX * bpp;
            break;

        case 9:
        case 12:
            pIn->bpp = 64;
            break;

        case 10:
        case 11:
        case 13:
            pIn->bpp = 128;
            break;

        default:
            pIn->bpp = bpp;
            break;
        }

        return pIn->bpp;
    }

    return 0;
}

uint32_t convertToNonBankSwappedMode(uint32_t tileMode)
{
    switch (tileMode)
    {
    case 8:
        return 4;
    case 9:
        return 5;
    case 10:
        return 6;
    case 11:
        return 7;
    case 14:
        return 12;
    case 15:
        return 13;
    }
    return tileMode;
}

uint32_t computeSurfaceThickness(uint32_t tileMode)
{
    switch (tileMode)
    {
    case 0x03: // AddrTileMode.ADDR_TM_1D_TILED_THICK:
    case 0x07: // AddrTileMode.ADDR_TM_2D_TILED_THICK:
    case 0x0B: // AddrTileMode.ADDR_TM_2B_TILED_THICK:
    case 0x0D: // AddrTileMode.ADDR_TM_3D_TILED_THICK:
    case 0x0F: // AddrTileMode.ADDR_TM_3B_TILED_THICK:
        return 4;

    case 0x10: // AddrTileMode.ADDR_TM_2D_TILED_XTHICK:
    case 0x11: // AddrTileMode.ADDR_TM_3D_TILED_XTHICK:
        return 8;

    default:
        return 1;
    }
}

uint32_t computeSurfaceTileSlices(uint32_t tileMode, uint32_t bpp, uint32_t numSamples)
{
    uint32_t bytePerSample = ((bpp << 6) + 7) >> 3;
    uint32_t tileSlices = 1;
    uint32_t samplePerTile;

    if (computeSurfaceThickness(tileMode) > 1)
        numSamples = 4;

    if (bytePerSample != 0)
    {
        samplePerTile = 2048 / bytePerSample;
        if (samplePerTile < numSamples)
            tileSlices = max(1, numSamples / samplePerTile);
    }

    return tileSlices;
}

uint32_t computeSurfaceMipLevelTileMode(uint32_t baseTileMode, uint32_t bpp, uint32_t level, uint32_t width, uint32_t height,
    uint32_t numSlices, uint32_t numSamples, uint32_t isDepth, uint32_t noRecursive)
{
    uint32_t widthAlignFactor = 1;
    uint32_t macroTileWidth = 32;
    uint32_t macroTileHeight = 16;
    uint32_t tileSlices = computeSurfaceTileSlices(baseTileMode, bpp, numSamples);
    uint32_t expTileMode = baseTileMode;

    uint32_t widtha, heighta, numSlicesa, thickness, microTileBytes;

    //if (bDebugLog)
    //    LogDebug("baseTileMode %d\n", baseTileMode);

    if (numSamples > 1 || tileSlices > 1 || isDepth != 0)
    {
        if (baseTileMode == 7)
            expTileMode = 4;
        else if (baseTileMode == 13)
            expTileMode = 12;
        else if (baseTileMode == 11)
            expTileMode = 8;
        else if (baseTileMode == 15)
            expTileMode = 14;
    }

    if (baseTileMode == 2 && numSamples > 1)
    {
        expTileMode = 4;
    }
    else if (baseTileMode == 3)
    {
        if (numSamples > 1 || isDepth != 0)
            expTileMode = 2;

        if (numSamples == 2 || numSamples == 4)
            expTileMode = 7;
    }
    else
    {
        expTileMode = baseTileMode;
    }
    //if (bDebugLog)
    //    LogDebug("computeSurfaceMipLevelTileMode expTileMode %d\n", expTileMode);

    if (noRecursive != 0 || level == 0)
        return expTileMode;

    switch (bpp)
    {
    case 24:
    case 48:
    case 96:
        bpp /= 3;
        break;
    }

    widtha = nextPow2(width);
    heighta = nextPow2(height);
    numSlicesa = nextPow2(numSlices);

    expTileMode = convertToNonBankSwappedMode(expTileMode);
    thickness = computeSurfaceThickness(expTileMode);
    microTileBytes = (numSamples * bpp * (thickness << 6) + 7) >> 3;

    if (microTileBytes < 256)
    {
        widthAlignFactor = max(1, 256 / microTileBytes);
    }
    if (expTileMode == 4 || expTileMode == 12)
    {
        if ((widtha < widthAlignFactor * macroTileWidth) || heighta < macroTileHeight)
            expTileMode = 2;
    }
    else if (expTileMode == 5)
    {
        macroTileWidth = 16;
        macroTileHeight = 32;

        if ((widtha < widthAlignFactor * macroTileWidth) || heighta < macroTileHeight)
            expTileMode = 2;
    }
    else if (expTileMode == 6)
    {
        macroTileWidth = 8;
        macroTileHeight = 64;

        if ((widtha < widthAlignFactor * macroTileWidth) || heighta < macroTileHeight)
            expTileMode = 2;
    }
    else if (expTileMode == 7 || expTileMode == 13)
    {
        if ((widtha < widthAlignFactor * macroTileWidth) || heighta < macroTileHeight)
            expTileMode = 3;
    }

    if (numSlicesa < 4)
    {
        if (expTileMode == 3)
            expTileMode = 2;
        else if (expTileMode == 7)
            expTileMode = 4;
        else if (expTileMode == 13)
            expTileMode = 12;
    }

    return computeSurfaceMipLevelTileMode(
        expTileMode,
        bpp,
        level,
        widtha,
        heighta,
        numSlicesa,
        numSamples,
        isDepth,
        1);
}

void padDimensions(uint32_t tileMode, uint32_t padDims, uint32_t isCube, uint32_t pitchAlign, uint32_t heightAlign, uint32_t sliceAlign, uint32_t expPitch[1], uint32_t expHeight[1], uint32_t expNumSlices[1], uint32_t padDimens[3])
{
    uint32_t thickness = computeSurfaceThickness(tileMode);
    if (padDims == 0)
        padDims = 3;

    if ((pitchAlign & (pitchAlign - 1)) == 0)
        expPitch[0] = powTwoAlign(expPitch[0], pitchAlign);
    else
    {
        expPitch[0] += pitchAlign - 1;
        expPitch[0] /= pitchAlign;
        expPitch[0] *= pitchAlign;
    }

    if (padDims > 1)
        expHeight[0] = powTwoAlign(expHeight[0], heightAlign);

    if (padDims > 2 || thickness > 1)
    {
        if (isCube != 0)
            expNumSlices[0] = nextPow2(expNumSlices[0]);

        if (thickness > 1)
            expNumSlices[0] = powTwoAlign(expNumSlices[0], sliceAlign);
    }

    padDimens[0] = expPitch[0];
    padDimens[1] = expHeight[0];
    padDimens[2] = expNumSlices[0];
}

uint32_t adjustPitchAlignment(Gx2Flags flags, uint32_t pitchAlign[1])
{
    if (((flags.value >> 13) & 1) != 0)
        pitchAlign[0] = powTwoAlign(pitchAlign[0], 0x20);

    return pitchAlign[0];
}

void computeSurfaceAlignmentsLinear(uint32_t tileMode, uint32_t bpp, Gx2Flags flags, uint32_t compAllignLinear[3])
{
    uint32_t pixelsPerPipeInterleave;
    uint32_t baseAlign, pitchAlign, heightAlign;

    if (tileMode == 0)
    {
        baseAlign = 1;
        pitchAlign = (bpp != 1 ? (uint32_t)1 : 8);
        heightAlign = 1;
    }
    else if (tileMode == 1)
    {
        pixelsPerPipeInterleave = 2048 / bpp;
        baseAlign = 256;
        pitchAlign = max(0x40, pixelsPerPipeInterleave);
        heightAlign = 1;
    }
    else
    {
        baseAlign = 1;
        pitchAlign = 1;
        heightAlign = 1;
    }
    pitchAlign = adjustPitchAlignment(flags, &pitchAlign);

    compAllignLinear[0] = baseAlign;
    compAllignLinear[1] = pitchAlign;
    compAllignLinear[2] = heightAlign;
}

void computeSurfaceInfoLinear(uint32_t tileMode, uint32_t bpp, uint32_t numSamples, uint32_t pitch, uint32_t height,
    uint32_t numSlices, uint32_t mipLevel, uint32_t padDims, Gx2Flags flags, uint32_t expPitch[1], uint32_t expHeight[1], uint32_t expNumSlices[1], uint32_t compSurfInfoLinear[9])
{
    expPitch[0] = pitch;
    expHeight[0] = height;
    expNumSlices[0] = numSlices;

    uint32_t valid = 1;
    uint32_t microTileThickness = computeSurfaceThickness(tileMode);

    uint32_t baseAlign, pitchAlign, heightAlign, slices;
    uint32_t pPitchOut, pHeightOut, pNumSlicesOut, pSurfSize, pBaseAlign, pPitchAlign, pHeightAlign, pDepthAlign;

    uint32_t compAllignLinear[3];

    computeSurfaceAlignmentsLinear(tileMode, bpp, flags, compAllignLinear);

    baseAlign = compAllignLinear[0];
    pitchAlign = compAllignLinear[1];
    heightAlign = compAllignLinear[2];

    if ((((flags.value >> 9) & 1) != 0) && (mipLevel == 0))
    {
        expPitch[0] /= 3;
        expPitch[0] = nextPow2(expPitch[0]);
    }
    if (mipLevel != 0)
    {
        expPitch[0] = nextPow2(expPitch[0]);
        expHeight[0] = nextPow2(expHeight[0]);

        if (((flags.value >> 4) & 1) != 0)
        {
            expNumSlices[0] = numSlices;

            if (numSlices <= 1)
                padDims = 2;
            else
                padDims = 0;
        }
        else
            expNumSlices[0] = nextPow2(numSlices);
    }

    uint32_t padDimens[3];

    padDimensions(
        tileMode,
        padDims,
        (flags.value >> 4) & 1,
        pitchAlign,
        heightAlign,
        microTileThickness,
        expPitch,
        expHeight,
        expNumSlices,
        padDimens);

    expPitch[0] = padDimens[0];
    expHeight[0] = padDimens[1];
    expNumSlices[0] = padDimens[2];

    if ((((flags.value >> 9) & 1) != 0) && (mipLevel == 0))
        expPitch[0] *= 3;

    slices = expNumSlices[0] * numSamples / microTileThickness;
    pPitchOut = expPitch[0];
    pHeightOut = expHeight[0];
    pNumSlicesOut = expNumSlices[0];
    pSurfSize = (expHeight[0] * expPitch[0] * slices * bpp * numSamples + 7) / 8;
    pBaseAlign = baseAlign;
    pPitchAlign = pitchAlign;
    pHeightAlign = heightAlign;
    pDepthAlign = microTileThickness;

    compSurfInfoLinear[0] = valid;
    compSurfInfoLinear[1] = pPitchOut;
    compSurfInfoLinear[2] = pHeightOut;
    compSurfInfoLinear[3] = pNumSlicesOut;
    compSurfInfoLinear[4] = pSurfSize;
    compSurfInfoLinear[5] = pBaseAlign;
    compSurfInfoLinear[6] = pPitchAlign;
    compSurfInfoLinear[7] = pHeightAlign;
    compSurfInfoLinear[8] = pDepthAlign;
}

void computeSurfaceAlignmentsMicroTiled(uint32_t tileMode, uint32_t bpp, Gx2Flags flags, uint32_t numSamples, uint32_t surfMicroAlign[3])
{
    switch (bpp)
    {
    case 24:
    case 48:
    case 96:
        bpp /= 3;
        break;
    }
    uint32_t thickness = computeSurfaceThickness(tileMode);
    uint32_t baseAlign = 256;
    uint32_t pitchAlign = max(8, 256 / bpp / numSamples / thickness);
    uint32_t heightAlign = 8;

    pitchAlign = adjustPitchAlignment(flags, &pitchAlign);

    surfMicroAlign[0] = baseAlign;
    surfMicroAlign[1] = pitchAlign;
    surfMicroAlign[2] = heightAlign;
}

void computeSurfaceInfoMicroTiled(uint32_t tileMode, uint32_t bpp, uint32_t numSamples, uint32_t pitch, uint32_t height, uint32_t numSlices, uint32_t mipLevel, uint32_t padDims, Gx2Flags flags, uint32_t expPitch[1], uint32_t expHeight[1], uint32_t expNumSlices[1], uint32_t microTileInfo[10])
{
    expPitch[0] = pitch;
    expHeight[0] = height;
    expNumSlices[0] = numSlices;

    uint32_t valid = 1;
    uint32_t expTileMode = tileMode;
    uint32_t microTileThickness = computeSurfaceThickness(tileMode);
    uint32_t pPitchOut, pHeightOut, pNumSlicesOut, pSurfSize, pTileModeOut, pBaseAlign, pPitchAlign, pHeightAlign, pDepthAlign;

    if (mipLevel != 0)
    {
        expPitch[0] = nextPow2(pitch);
        expHeight[0] = nextPow2(height);
        if (((flags.value >> 4) & 1) != 0)
        {
            expNumSlices[0] = numSlices;

            if (numSlices <= 1)
                padDims = 2;

            else
                padDims = 0;
        }

        else
            expNumSlices[0] = nextPow2(numSlices);

        if (expTileMode == 3 && expNumSlices[0] < 4)
        {
            expTileMode = 2;
            microTileThickness = 1;
        }
    }
    uint32_t surfMicroAlign[3];

    computeSurfaceAlignmentsMicroTiled(
        expTileMode,
        bpp,
        flags,
        numSamples,
        surfMicroAlign);

    uint32_t baseAlign = surfMicroAlign[0];
    uint32_t pitchAlign = surfMicroAlign[1];
    uint32_t heightAlign = surfMicroAlign[2];

    uint32_t padDimens[3];

    padDimensions(
        expTileMode,
        padDims,
        (flags.value >> 4) & 1,
        pitchAlign,
        heightAlign,
        microTileThickness,
        expPitch,
        expHeight,
        expNumSlices,
        padDimens);

    expPitch[0] = padDimens[0];
    expHeight[0] = padDimens[1];
    expNumSlices[0] = padDimens[2];

    pPitchOut = expPitch[0];
    pHeightOut = expHeight[0];
    pNumSlicesOut = expNumSlices[0];
    pSurfSize = (expHeight[0] * expPitch[0] * expNumSlices[0] * bpp * numSamples + 7) / 8;
   
    //if (bDebugLog)
    //{
    //    LogDebug("pSurfSize %d\n", pSurfSize);
    //    LogDebug("expHeight %d\n", expHeight);
    //    LogDebug("expPitch %d\n", expPitch);
    //    LogDebug("expNumSlices %d\n", expNumSlices);
    //    LogDebug("numSamples %d\n", numSamples);
    //}

    pTileModeOut = expTileMode;
    pBaseAlign = baseAlign;
    pPitchAlign = pitchAlign;
    pHeightAlign = heightAlign;
    pDepthAlign = microTileThickness;

    microTileInfo[0] = valid;
    microTileInfo[1] = pPitchOut;
    microTileInfo[2] = pHeightOut;
    microTileInfo[3] = pNumSlicesOut;
    microTileInfo[4] = pSurfSize;
    microTileInfo[5] = pTileModeOut;
    microTileInfo[6] = pBaseAlign;
    microTileInfo[7] = pPitchAlign;
    microTileInfo[8] = pHeightAlign;
    microTileInfo[9] = pDepthAlign;
}

uint32_t isThickMacroTiled(uint32_t tileMode)
{
    switch (tileMode)
    {
    case 0x7: // AddrTileMode.ADDR_TM_2D_TILED_THICK:
    case 0x8: // AddrTileMode.ADDR_TM_2B_TILED_THICK:
    case 0xD: // AddrTileMode.ADDR_TM_3D_TILED_THICK:
    case 0xF: // AddrTileMode.ADDR_TM_3B_TILED_THICK:
        return 1;

    default:
        return 0;
    }
}

uint32_t computeMacroTileAspectRatio(uint32_t tileMode)
{
    switch (tileMode)
    {
    case 0x5: // AddrTileMode.ADDR_TM_2D_TILED_THIN2:
    case 0x9: // AddrTileMode.ADDR_TM_2B_TILED_THIN2:
        return 2;

    case 0x6: // AddrTileMode.ADDR_TM_2D_TILED_THIN4:
    case 0xA: // AddrTileMode.ADDR_TM_2B_TILED_THIN4:
        return 4;

    default:
        return 1;
    }
}

void computeSurfaceAlignmentsMacroTiled(uint32_t tileMode, uint32_t bpp, Gx2Flags flags, uint32_t numSamples, uint32_t tup[5])
{
    uint32_t aspectRatio = computeMacroTileAspectRatio(tileMode);
    uint32_t thickness = computeSurfaceThickness(tileMode);

    switch (bpp)
    {
    case 24:
    case 48:
    case 96:
        bpp /= 3;
        break;
    case 3:
        bpp = 1;
        break;
    }
    uint32_t macroTileWidth = 32 / aspectRatio;
    uint32_t macroTileHeight = aspectRatio * 16;

    uint32_t pitchAlign = max(macroTileWidth, macroTileWidth * (256 / bpp / (8 * thickness) / numSamples));
    pitchAlign = adjustPitchAlignment(flags, &pitchAlign);

    uint32_t heightAlign = macroTileHeight;
    uint32_t macroTileBytes = numSamples * ((bpp * macroTileHeight * macroTileWidth + 7) >> 3);

    uint32_t baseAlign;

    if (thickness == 1)
        baseAlign = max(macroTileBytes, (numSamples * heightAlign * bpp * pitchAlign + 7) >> 3);
    else
        baseAlign = max(256, (4 * heightAlign * bpp * pitchAlign + 7) >> 3);

    uint32_t microTileBytes = (thickness * numSamples * (bpp << 6) + 7) >> 3;
    uint32_t numSlicesPerMicroTile = (microTileBytes < 2048 ? (uint32_t)1 : microTileBytes / 2048);

    baseAlign /= numSlicesPerMicroTile;

    tup[0] = baseAlign;
    tup[1] = pitchAlign;
    tup[2] = heightAlign;
    tup[3] = macroTileWidth;
    tup[4] = macroTileHeight;
}

uint32_t isBankSwappedTileMode(uint32_t tileMode)
{
    switch (tileMode)
    {
    case 0x8: // AddrTileMode.ADDR_TM_2B_TILED_THIN1:
    case 0x9: // AddrTileMode.ADDR_TM_2B_TILED_THIN2:
    case 0xA: // AddrTileMode.ADDR_TM_2B_TILED_THIN4:
    case 0xB: // AddrTileMode.ADDR_TM_2B_TILED_THICK:
    case 0xE: // AddrTileMode.ADDR_TM_3B_TILED_THIN1:
    case 0xF: // AddrTileMode.ADDR_TM_3B_TILED_THICK:
        return 1;

    default:
        return 0;
    }
}

uint32_t computeSurfaceBankSwappedWidth(uint32_t tileMode, uint32_t bpp, uint32_t numSamples, uint32_t pitch)
{
    if (isBankSwappedTileMode(tileMode) == 0)
        return 0;

    uint32_t bytesPerSample = 8 * bpp;
    uint32_t samplesPerTile, slicesPerTile;

    if (bytesPerSample != 0)
    {
        samplesPerTile = 2048 / bytesPerSample;
        slicesPerTile = max(1, numSamples / samplesPerTile);
    }

    else
        slicesPerTile = 1;

    if (isThickMacroTiled(tileMode) != 0)
        numSamples = 4;

    uint32_t bytesPerTileSlice = numSamples * bytesPerSample / slicesPerTile;

    uint32_t factor = computeMacroTileAspectRatio(tileMode);
    uint32_t swapTiles = max(1, 128 / bpp);

    uint32_t swapWidth = swapTiles * 32;
    uint32_t heightBytes = numSamples * factor * bpp * 2 / slicesPerTile;
    uint32_t swapMax = 0x4000 / heightBytes;
    uint32_t swapMin = 256 / bytesPerTileSlice;

    uint32_t bankSwapWidth = min(swapMax, max(swapMin, swapWidth));

    while (bankSwapWidth >= 2 * pitch)
        bankSwapWidth >>= 1;

    return bankSwapWidth;
}

void computeSurfaceInfoMacroTiled(uint32_t tileMode, uint32_t baseTileMode, uint32_t bpp, uint32_t numSamples,
    uint32_t pitch, uint32_t height, uint32_t numSlices, uint32_t mipLevel, uint32_t padDims, Gx2Flags flags, uint32_t expPitch[1], uint32_t expHeight[1], uint32_t expNumSlices[1], uint32_t compSurfInfoMacoTile[10])
{
    expPitch[0] = pitch;
    expHeight[0] = height;
    expNumSlices[0] = numSlices;

    uint32_t valid = 1;
    uint32_t expTileMode = tileMode;
    uint32_t microTileThickness = computeSurfaceThickness(tileMode);

    uint32_t baseAlign, pitchAlign, heightAlign, macroWidth, macroHeight;
    uint32_t bankSwappedWidth, pitchAlignFactor;
    uint32_t result, pPitchOut, pHeightOut, pNumSlicesOut, pSurfSize, pTileModeOut, pBaseAlign, pPitchAlign, pHeightAlign, pDepthAlign;

    if (mipLevel != 0)
    {
        expPitch[0] = nextPow2(pitch);
        expHeight[0] = nextPow2(height);

        if (((flags.value >> 4) & 1) != 0)
        {
            expNumSlices[0] = numSlices;

            if (numSlices <= 1)
                padDims = 2;
            else
                padDims = 0;
        }
        else
            expNumSlices[0] = nextPow2(numSlices);

        if (expTileMode == 7 && expNumSlices[0] < 4)
        {
            expTileMode = 4;
            microTileThickness = 1;
        }
    }
    if (tileMode == baseTileMode
        || mipLevel == 0
        || isThickMacroTiled(baseTileMode) == 0
        || isThickMacroTiled(tileMode) != 0)
    {
        uint32_t tup[5];

        computeSurfaceAlignmentsMacroTiled(
            tileMode,
            bpp,
            flags,
            numSamples, 
            tup);

        baseAlign = tup[0];
        pitchAlign = tup[1];
        heightAlign = tup[2];
        macroWidth = tup[3];
        macroHeight = tup[4];

        bankSwappedWidth = computeSurfaceBankSwappedWidth(tileMode, bpp, numSamples, pitch);

        if (bankSwappedWidth > pitchAlign)
            pitchAlign = bankSwappedWidth;

        uint32_t padDimens[3];

        padDimensions(
            tileMode,
            padDims,
            (flags.value >> 4) & 1,
            pitchAlign,
            heightAlign,
            microTileThickness,
            expPitch,
            expHeight,
            expNumSlices,
            padDimens);

        expPitch[0] = padDimens[0];
        expHeight[0] = padDimens[1];
        expNumSlices[0] = padDimens[2];

        pPitchOut = expPitch[0];
        pHeightOut = expHeight[0];
        pNumSlicesOut = expNumSlices[0];
        pSurfSize = (expHeight[0] * expPitch[0] * expNumSlices[0] * bpp * numSamples + 7) / 8;
        pTileModeOut = expTileMode;
        pBaseAlign = baseAlign;
        pPitchAlign = pitchAlign;
        pHeightAlign = heightAlign;
        pDepthAlign = microTileThickness;
        result = valid;
    }

    else
    {
        uint32_t tup[5];

        computeSurfaceAlignmentsMacroTiled(
            baseTileMode,
            bpp,
            flags,
            numSamples,
            tup);

        baseAlign = tup[0];
        pitchAlign = tup[1];
        heightAlign = tup[2];
        macroWidth = tup[3];
        macroHeight = tup[4];

        pitchAlignFactor = max(1, 32 / bpp);

        if (expPitch[0] < pitchAlign * pitchAlignFactor || expHeight[0] < heightAlign)
        {
            expTileMode = 2;

            uint32_t microTileInfo[10];
            
            computeSurfaceInfoMicroTiled(
                2,
                bpp,
                numSamples,
                pitch,
                height,
                numSlices,
                mipLevel,
                padDims,
                flags,
                expPitch,
                expHeight,
                expNumSlices,
                microTileInfo);

            result = microTileInfo[0];
            pPitchOut = microTileInfo[1];
            pHeightOut = microTileInfo[2];
            pNumSlicesOut = microTileInfo[3];
            pSurfSize = microTileInfo[4];
            pTileModeOut = microTileInfo[5];
            pBaseAlign = microTileInfo[6];
            pPitchAlign = microTileInfo[7];
            pHeightAlign = microTileInfo[8];
            pDepthAlign = microTileInfo[9];
        }

        else
        {
            uint32_t tup[5];

            computeSurfaceAlignmentsMacroTiled(
                tileMode,
                bpp,
                flags,
                numSamples,
                tup);

            baseAlign = tup[0];
            pitchAlign = tup[1];
            heightAlign = tup[2];
            macroWidth = tup[3];
            macroHeight = tup[4];

            bankSwappedWidth = computeSurfaceBankSwappedWidth(tileMode, bpp, numSamples, pitch);
            if (bankSwappedWidth > pitchAlign)
                pitchAlign = bankSwappedWidth;

            uint32_t padDimens[3];

            padDimensions(
                tileMode,
                padDims,
                (flags.value >> 4) & 1,
                pitchAlign,
                heightAlign,
                microTileThickness,
                expPitch,
                expHeight,
                expNumSlices,
                padDimens);

            expPitch[0] = padDimens[0];
            expHeight[0] = padDimens[1];
            expNumSlices[0] = padDimens[2];

            pPitchOut = expPitch[0];
            pHeightOut = expHeight[0];
            pNumSlicesOut = expNumSlices[0];
            pSurfSize = (expHeight[0] * expPitch[0] * expNumSlices[0] * bpp * numSamples + 7) / 8;

            pTileModeOut = expTileMode;
            pBaseAlign = baseAlign;
            pPitchAlign = pitchAlign;
            pHeightAlign = heightAlign;
            pDepthAlign = microTileThickness;
            result = valid;
        }
    }

    compSurfInfoMacoTile[0] = result;
    compSurfInfoMacoTile[1] = pPitchOut;
    compSurfInfoMacoTile[2] = pHeightOut;
    compSurfInfoMacoTile[3] = pNumSlicesOut;
    compSurfInfoMacoTile[4] = pSurfSize;
    compSurfInfoMacoTile[5] = pTileModeOut;
    compSurfInfoMacoTile[6] = pBaseAlign;
    compSurfInfoMacoTile[7] = pitchAlign;
    compSurfInfoMacoTile[8] = heightAlign;
    compSurfInfoMacoTile[9] = pDepthAlign;
}

uint32_t ComputeSurfaceInfoEx(Gx2SurfaceIn* pIn, Gx2SurfaceOut* pOut)
{
    uint32_t tileMode = pIn->tileMode;
    uint32_t bpp = pIn->bpp;
    uint32_t numSamples = max(1, pIn->numSamples);
    uint32_t pitch = pIn->width;
    //if (bDebugLog)
    //{
    //    LogDebug("ComputeSurfaceInfoEx pitch %d\n", pitch);
    //}
    uint32_t height = pIn->height;
    uint32_t numSlices = pIn->numSlices;
    uint32_t mipLevel = pIn->mipLevel;
    Gx2Flags flags = { 0 };
    uint32_t pPitchOut = pOut->pitch;
    uint32_t pHeightOut = pOut->height;
    uint32_t pNumSlicesOut = pOut->depth;
    uint32_t pTileModeOut = pOut->tileMode;
    uint32_t pSurfSize = (uint32_t)pOut->surfSize;
    uint32_t pBaseAlign = pOut->baseAlign;
    uint32_t pPitchAlign = pOut->pitchAlign;
    uint32_t pHeightAlign = pOut->heightAlign;
    uint32_t pDepthAlign = pOut->depthAlign;
    uint32_t padDims = 0;
    uint32_t valid = 0;
    uint32_t baseTileMode = tileMode;

    //if (bDebugLog)
    //{
    //    LogDebug("---------------------------\n");
    //    LogDebug("%d\n", tileMode);
    //    LogDebug("%d\n", bpp);
    //    LogDebug("%d\n", numSamples);
    //    LogDebug("%d\n", pitch);
    //    LogDebug("%d\n", height);
    //    LogDebug("%d\n", numSlices);
    //    LogDebug("%d\n", mipLevel);
    //    LogDebug("%d\n", flags);
    //    LogDebug("%d\n", pPitchOut);
    //    LogDebug("%d\n", pHeightOut);
    //    LogDebug("%d\n", pNumSlicesOut);
    //    LogDebug("%d\n", pTileModeOut);
    //    LogDebug("%d\n", pSurfSize);
    //    LogDebug("%d\n", pBaseAlign);
    //    LogDebug("%d\n", pPitchAlign);
    //    LogDebug("%d\n", pHeightAlign);
    //    LogDebug("%d\n", pDepthAlign);
    //    LogDebug("%d\n", padDims);
    //    LogDebug("%d\n", valid);
    //    LogDebug("%d\n", baseTileMode);
    //    LogDebug("---------------------------\n");
    //}

    flags.value = pIn->flags.value;

    //if (bDebugLog)
    //{
    //    LogDebug("padDims %d\n", padDims);
    //}

    if ((((flags.value >> 4) & 1) != 0) && (mipLevel == 0))
        padDims = 2;

    //if (bDebugLog)
    //{
    //    LogDebug("padDims %d\n", padDims);
    //}

    if (((flags.value >> 6) & 1) != 0)
        tileMode = convertToNonBankSwappedMode(tileMode);
    else
    {
        //if (bDebugLog)
        //    LogDebug("%d\n", tileMode);

        tileMode = computeSurfaceMipLevelTileMode(
            tileMode,
            bpp,
            mipLevel,
            pitch,
            height,
            numSlices,
            numSamples,
            (flags.value >> 1) & 1, 0);

        //if (bDebugLog)
        //{
        //    LogDebug("---------------------------\n");
        //    LogDebug("%d\n",tileMode);
        //    LogDebug("---------------------------\n");
        //}
    }

    uint32_t expPitch[1] = { 0 };
    uint32_t expHeight[1] = { 0 };
    uint32_t expNumSlices[1] = { 0 };
    uint32_t compSurfInfoLinear[9] = { 0 };
    uint32_t compSurfInfoMicroTile[10] = { 0 };
    uint32_t compSurfInfoMacoTile[10] = { 0 };

    switch (tileMode)
    {
    case 0:
    case 1:
        computeSurfaceInfoLinear(
            tileMode,
            bpp,
            numSamples,
            pitch,
            height,
            numSlices,
            mipLevel,
            padDims,
            flags,
            expPitch,
            expHeight,
            expNumSlices,
            compSurfInfoLinear);

        valid = compSurfInfoLinear[0];
        pPitchOut = compSurfInfoLinear[1];
        pHeightOut = compSurfInfoLinear[2];
        pNumSlicesOut = compSurfInfoLinear[3];
        pSurfSize = compSurfInfoLinear[4];
        pBaseAlign = compSurfInfoLinear[5];
        pPitchAlign = compSurfInfoLinear[6];
        pHeightAlign = compSurfInfoLinear[7];
        pDepthAlign = compSurfInfoLinear[8];

        pTileModeOut = tileMode;
        break;
    case 2:
    case 3:
        computeSurfaceInfoMicroTiled(
            tileMode,
            bpp,
            numSamples,
            pitch,
            height,
            numSlices,
            mipLevel,
            padDims,
            flags,
            expPitch,
            expHeight,
            expNumSlices,
            compSurfInfoMicroTile);

        valid = compSurfInfoMicroTile[0];
        pPitchOut = compSurfInfoMicroTile[1];
        pHeightOut = compSurfInfoMicroTile[2];
        pNumSlicesOut = compSurfInfoMicroTile[3];
        pSurfSize = compSurfInfoMicroTile[4];
        pTileModeOut = compSurfInfoMicroTile[5];
        pBaseAlign = compSurfInfoMicroTile[6];
        pPitchAlign = compSurfInfoMicroTile[7];
        pHeightAlign = compSurfInfoMicroTile[8];
        pDepthAlign = compSurfInfoMicroTile[9];

        break;
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
        computeSurfaceInfoMacroTiled(
            tileMode,
            baseTileMode,
            bpp,
            numSamples,
            pitch,
            height,
            numSlices,
            mipLevel,
            padDims,
            flags,
            expPitch,
            expHeight,
            expNumSlices,
            compSurfInfoMacoTile);

        valid = compSurfInfoMacoTile[0];
        pPitchOut = compSurfInfoMacoTile[1];
        pHeightOut = compSurfInfoMacoTile[2];
        pNumSlicesOut = compSurfInfoMacoTile[3];
        pSurfSize = compSurfInfoMacoTile[4];
        pTileModeOut = compSurfInfoMacoTile[5];
        pBaseAlign = compSurfInfoMacoTile[6];
        pPitchAlign = compSurfInfoMacoTile[7];
        pHeightAlign = compSurfInfoMacoTile[8];
        pDepthAlign = compSurfInfoMacoTile[9];
        break;
    }

    pOut->pitch = pPitchOut;
    pOut->height = pHeightOut;
    pOut->depth = pNumSlicesOut;
    pOut->tileMode = pTileModeOut;
    pOut->surfSize = pSurfSize;
    pOut->baseAlign = pBaseAlign;
    pOut->pitchAlign = pPitchAlign;
    pOut->heightAlign = pHeightAlign;
    pOut->depthAlign = pDepthAlign;

    //if (bDebugLog)
    //{
    //    LogDebug("%d\n", pOut->pitch);
    //    LogDebug("%d\n", pOut->height);
    //    LogDebug("%d\n", pOut->depth);
    //    LogDebug("%d\n", pOut->tileMode);
    //    LogDebug("%d\n", pOut->surfSize);
    //    LogDebug("%d\n", pOut->baseAlign);
    //    LogDebug("%d\n", pOut->pitchAlign);
    //    LogDebug("%d\n", pOut->heightAlign);
    //    LogDebug("%d\n", pOut->depthAlign);
    //}

    if (valid == 0)
        return 3;

    return 0;
}

uint32_t restoreSurfaceInfo(uint32_t elemMode, uint32_t expandX, uint32_t expandY, uint32_t bpp, Gx2SurfaceIn* pIn, Gx2SurfaceOut* pOut)
{
    uint32_t width, height;

    if (pOut->pixelPitch != 0 && pOut->pixelHeight != 0)
    {
        width = pOut->pixelPitch;
        height = pOut->pixelHeight;

        if (expandX > 1 || expandY > 1)
        {
            if (elemMode == 4)
            {
                width /= expandX;
                height /= expandY;
            }

            else
            {
                width *= expandX;
                height *= expandY;
            }
        }

        pOut->pixelPitch = max(1, width);
        pOut->pixelHeight = max(1, height);
    }
    if (bpp != 0)
    {
        switch (elemMode)
        {
        case 4:
            return expandY * expandX * bpp;

        case 5:
        case 6:
            return bpp / expandX / expandY;

        case 9:
        case 12:
            return 64;

        case 10:
        case 11:
        case 13:
            return 128;

        default:
            return bpp;
        }
    }

    return 0;
}

static uint8_t formatExInfo[64 * 4] = {
    //  bpp   expandX expandY elemMode
        0x00, 0x01,   0x01,   0x03, // 0x00
        0x08, 0x01,   0x01,   0x03, // 0x01 GX2_SURFACE_FORMAT_UNORM_R8
        0x08, 0x01,   0x01,   0x03, // 0x02 T_R4_G4
        0x08, 0x01,   0x01,   0x03, // 0x03
        0x00, 0x01,   0x01,   0x03, // 0x04
        0x10, 0x01,   0x01,   0x03, // 0x05 GX2_SURFACE_FORMAT_UNORM_R1
        0x10, 0x01,   0x01,   0x03, // 0x06 TC_R16_FLOAT
        0x10, 0x01,   0x01,   0x03, // 0x07 GX2_SURFACE_FORMAT_UNORM_R8_G8
        0x10, 0x01,   0x01,   0x03, // 0x08 GX2_SURFACE_FORMAT_UNORM_R5_G6_B5
        0x10, 0x01,   0x01,   0x03, // 0x09
        0x10, 0x01,   0x01,   0x03, // 0x0A GX2_SURFACE_FORMAT_UNORM_R5_G5_B5_A1
        0x10, 0x01,   0x01,   0x03, // 0x0B GX2_SURFACE_FORMAT_UNORM_R4_G4_B4_A4
        0x10, 0x01,   0x01,   0x03, // 0x0C A1_B5_G5_R5
        0x20, 0x01,   0x01,   0x03, // 0x0D R32
        0x20, 0x01,   0x01,   0x03, // 0x0E R32_FLOAT
        0x20, 0x01,   0x01,   0x03, // 0x0F R16_G16
        0x20, 0x01,   0x01,   0x03, // 0x10 R16_G16_FLOAT
        0x20, 0x01,   0x01,   0x03, // 0x11 GX2_SURFACE_FORMAT_UNORM_R24_X8 
        0x20, 0x01,   0x01,   0x03, // 0x12
        0x20, 0x01,   0x01,   0x03, // 0x13
        0x20, 0x01,   0x01,   0x03, // 0x14
        0x20, 0x01,   0x01,   0x03, // 0x15
        0x20, 0x01,   0x01,   0x03, // 0x16 R11_G11_B10_FLOAT
        0x20, 0x01,   0x01,   0x03, // 0x17 
        0x20, 0x01,   0x01,   0x03, // 0x18 
        0x20, 0x01,   0x01,   0x03, // 0x19 GX2_SURFACE_FORMAT_UNORM_R10_G10_B10_A2
        0x20, 0x01,   0x01,   0x03, // 0x1A R8_G8_B8_A8
        0x20, 0x01,   0x01,   0x03, // 0x1B A2_B20_G10_R10
        0x40, 0x01,   0x01,   0x03, // 0x1C D32_FLOAT_S8_UINT_X24
        0x40, 0x01,   0x01,   0x03, // 0x1D R32_G32
        0x40, 0x01,   0x01,   0x03, // 0x1E R32_G32_FLOAT
        0x40, 0x01,   0x01,   0x03, // 0x1F GX2_SURFACE_FORMAT_UNORM_R16_G16_B16_A16
        0x40, 0x01,   0x01,   0x03, // 0x20 R16_G16_B16_A16
        0x00, 0x01,   0x01,   0x03, // 0x21
        0x80, 0x01,   0x01,   0x03, // 0x22 R32_G32_B32_A32
        0x80, 0x01,   0x01,   0x03, // 0x23 R32_G32_B32_A32_FLOAT
        0x00, 0x01,   0x01,   0x03, // 0x24
        0x01, 0x08,   0x01,   0x05, // 0x25
        0x01, 0x08,   0x01,   0x06, // 0x26
        0x10, 0x01,   0x01,   0x07, // 0x27
        0x10, 0x01,   0x01,   0x08, // 0x28
        0x20, 0x01,   0x01,   0x03, // 0x29
        0x20, 0x01,   0x01,   0x03, // 0x2A
        0x20, 0x01,   0x01,   0x03, // 0x2B
        0x18, 0x03,   0x01,   0x04, // 0x2C
        0x30, 0x03,   0x01,   0x04, // 0x2D
        0x30, 0x03,   0x01,   0x04, // 0x2E
        0x60, 0x03,   0x01,   0x04, // 0x2F
        0x60, 0x03,   0x01,   0x04, // 0x30
        0x40, 0x04,   0x04,   0x09, // 0x31 GX2_SURFACE_FORMAT_UNORM_BC1
        0x80, 0x04,   0x04,   0x0A, // 0x32 GX2_SURFACE_FORMAT_UNORM_BC2
        0x80, 0x04,   0x04,   0x0B, // 0x33 GX2_SURFACE_FORMAT_UNORM_BC3
        0x40, 0x04,   0x04,   0x0C, // 0x34 GX2_SURFACE_FORMAT_UNORM_BC4
        0x40, 0x04,   0x04,   0x0D, // 0x35 GX2_SURFACE_FORMAT_UNORM_BC5
        0x40, 0x04,   0x04,   0x0D, // 0x36 
        0x40, 0x04,   0x04,   0x0D, // 0x37 
        0x00, 0x01,   0x01,   0x03, // 0x38 
        0x00, 0x01,   0x01,   0x03, // 0x39 
        0x00, 0x01,   0x01,   0x03, // 0x3A 
        0x00, 0x01,   0x01,   0x03, // 0x3B 
        0x00, 0x01,   0x01,   0x03, // 0x3C 
        0x00, 0x01,   0x01,   0x03, // 0x3D  
        0x40, 0x01,   0x01,   0x03, // 0x3E 
        0x00, 0x01,   0x01,   0x03, // 0x3F 
};

void computeSurfaceInfo(Gx2SurfaceIn* aSurfIn, Gx2SurfaceOut* pSurfOut, Gx2SurfaceIn* pIn, Gx2SurfaceOut* pOut)
{
    //if (bDebugLog)
    //{
    //    LogDebug(" computeSurfaceInfo ------------------------------------ \n");
    //}
    *pIn = *aSurfIn;
    *pOut = *pSurfOut;

    uint32_t returnCode = 0;

    uint32_t width, height, bpp, elemMode = 0;
    uint32_t expandY, expandX;

    if (pIn->bpp > 0x80)
        returnCode = 3;

    //if (bDebugLog)
    //    LogDebug("returnCode %d\n", returnCode);

    if (returnCode == 0)
    {
        //if (bDebugLog)
        //{
        //    LogDebug("-------------------------------------------\n");
        //    LogDebug(" computeMipLevel\n");
        //    LogDebug("-------------------------------------------\n");
        //    LogDebug(" pIn.width %d\n", pIn->width);
        //    LogDebug(" pIn.height %d\n", pIn->height);
        //    LogDebug(" pIn.numSlices %d\n", pIn->numSlices);
        //}

        computeMipLevel(pIn, pOut);

        width = pIn->width;
        height = pIn->height;
        bpp = pIn->bpp;
        expandX = 1;
        expandY = 1;

        //if (bDebugLog)
        //{
        //    LogDebug("%d\n", pIn->width);
        //    LogDebug("%d\n", pIn->height);
        //    LogDebug("%d\n", pIn->numSlices);
        //    LogDebug("-------------------------------------------\n");
        //}

        pOut->pixelBits = pIn->bpp;

        if (pIn->format != 0)
        {
            bpp = formatExInfo[pIn->format * 4];
            expandX = formatExInfo[pIn->format * 4 + 1];
            expandY = formatExInfo[pIn->format * 4 + 2];
            elemMode = formatExInfo[pIn->format * 4 + 3];

            //if (bDebugLog)
            //{
            //    LogDebug("bpp %d\n", bpp);
            //    LogDebug("expandX %d\n", expandX);
            //    LogDebug("expandY %d\n", expandY);
            //    LogDebug("elemMode %d\n", elemMode);
            //}

            if (elemMode == 4 && expandX == 3 && pIn->tileMode == 1)
                pIn->flags.value |= 0x200;

            bpp = adjustSurfaceInfo(elemMode, expandX, expandY, bpp, width, height, pIn, pOut);

            //if (bDebugLog)
            //{
            //    LogDebug("width %d\n", pIn->width);
            //    LogDebug("height %d\n", pIn->height);
            //    LogDebug("bpp %d\n", pIn->bpp);
            //}

        }
        else if (pIn->bpp != 0)
        {
            pIn->width = max(1, pIn->width);
            pIn->height = max(1, pIn->height);
        }
        else
            returnCode = 3;

        if (returnCode == 0)
            returnCode = ComputeSurfaceInfoEx(pIn, pOut);

        if (returnCode == 0)
        {
            pOut->bpp = pIn->bpp;
            pOut->pixelPitch = pOut->pitch;
            pOut->pixelHeight = pOut->height;

            if (pIn->format != 0 && (((pIn->flags.value >> 9) & 1) == 0 || pIn->mipLevel == 0))
                bpp = restoreSurfaceInfo(elemMode, expandX, expandY, bpp, pIn, pOut);

            if (((pIn->flags.value >> 5) & 1) != 0)
                pOut->sliceSize = (uint32_t)pOut->surfSize;

            else
            {
                pOut->sliceSize = (uint32_t)(pOut->surfSize / pOut->depth);

                if (pIn->slice == (pIn->numSlices - 1) && pIn->numSlices > 1)
                    pOut->sliceSize += pOut->sliceSize * (pOut->depth - pIn->numSlices);
            }

            pOut->pitchTileMax = (pOut->pitch >> 3) - 1;
            pOut->heightTileMax = (pOut->height >> 3) - 1;
            pOut->sliceTileMax = (pOut->height * pOut->pitch >> 6) - 1;
        }
    }

}

uint32_t DIV_ROUND_UP(uint32_t n, uint32_t d)
{
    return (n + d - 1) / d;
}

uint32_t GX2TileModeToAddrTileMode(uint32_t tileMode)
{
    if (tileMode == 0)
        assert(0 && "Use tileMode from getDefaultGX2TileMode().");

    if (tileMode == 16)
        return 0;

    return tileMode;
}

long computeSurfaceAddrFromCoordLinear(uint32_t x, uint32_t y, uint32_t slice, uint32_t sample, uint32_t bpp, uint32_t pitch, uint32_t height, uint32_t numSlices)
{
    uint32_t sliceOffset = pitch * height * (slice + sample * numSlices);
    return (y * pitch + x + sliceOffset) * bpp;
}

uint32_t computePixelIndexWithinMicroTile(uint32_t x, uint32_t y, uint32_t z, uint32_t bpp, uint32_t tileMode, bool IsDepth)
{
    uint32_t pixelBit0 = 0;
    uint32_t pixelBit1 = 0;
    uint32_t pixelBit2 = 0;
    uint32_t pixelBit3 = 0;
    uint32_t pixelBit4 = 0;
    uint32_t pixelBit5 = 0;
    uint32_t pixelBit6 = 0;
    uint32_t pixelBit7 = 0;
    uint32_t pixelBit8 = 0;

    uint32_t thickness = computeSurfaceThickness(tileMode);

    if (IsDepth)
    {
        pixelBit0 = x & 1;
        pixelBit1 = y & 1;
        pixelBit2 = (x & 2) >> 1;
        pixelBit3 = (y & 2) >> 1;
        pixelBit4 = (x & 4) >> 2;
        pixelBit5 = (y & 4) >> 2;
    }
    else
    {
        switch (bpp)
        {
        case 8:
            pixelBit0 = x & 1;
            pixelBit1 = (x & 2) >> 1;
            pixelBit2 = (x & 4) >> 2;
            pixelBit3 = (y & 2) >> 1;
            pixelBit4 = y & 1;
            pixelBit5 = (y & 4) >> 2;
            break;
        case 0x10:
            pixelBit0 = x & 1;
            pixelBit1 = (x & 2) >> 1;
            pixelBit2 = (x & 4) >> 2;
            pixelBit3 = y & 1;
            pixelBit4 = (y & 2) >> 1;
            pixelBit5 = (y & 4) >> 2;
            break;
        case 0x20:
        case 0x60:
            pixelBit0 = x & 1;
            pixelBit1 = (x & 2) >> 1;
            pixelBit2 = y & 1;
            pixelBit3 = (x & 4) >> 2;
            pixelBit4 = (y & 2) >> 1;
            pixelBit5 = (y & 4) >> 2;
            break;
        case 0x40:
            pixelBit0 = x & 1;
            pixelBit1 = y & 1;
            pixelBit2 = (x & 2) >> 1;
            pixelBit3 = (x & 4) >> 2;
            pixelBit4 = (y & 2) >> 1;
            pixelBit5 = (y & 4) >> 2;
            break;
        case 0x80:
            pixelBit0 = y & 1;
            pixelBit1 = x & 1;
            pixelBit2 = (x & 2) >> 1;
            pixelBit3 = (x & 4) >> 2;
            pixelBit4 = (y & 2) >> 1;
            pixelBit5 = (y & 4) >> 2;
            break;
        default:
            pixelBit0 = x & 1;
            pixelBit1 = (x & 2) >> 1;
            pixelBit2 = y & 1;
            pixelBit3 = (x & 4) >> 2;
            pixelBit4 = (y & 2) >> 1;
            pixelBit5 = (y & 4) >> 2;
            break;
        }
    }

    if (thickness > 1)
    {
        pixelBit6 = z & 1;
        pixelBit7 = (z & 2) >> 1;
    }

    if (thickness == 8)
        pixelBit8 = (z & 4) >> 2;

    return (pixelBit8 << 8) | (pixelBit7 << 7) | (pixelBit6 << 6) | 32 * pixelBit5 | 16 * pixelBit4 | 8 * pixelBit3 | 4 * pixelBit2 | pixelBit0 | 2 * pixelBit1;
}

long computeSurfaceAddrFromCoordMicroTiled(uint32_t x, uint32_t y, uint32_t slice, uint32_t bpp, uint32_t pitch, uint32_t height, uint32_t tileMode, bool IsDepth)
{
    uint32_t microTileThickness = 1;
    if (tileMode == 0x3) // AddrTileMode.ADDR_TM_1D_TILED_THICK
        microTileThickness = 4;

    uint32_t microTileBytes = (uint32_t)(64 * microTileThickness * bpp + 7) / 8;
    uint32_t microTilesPerRow = pitch >> 3;
    uint32_t microTileIndexX = x >> 3;
    uint32_t microTileIndexY = y >> 3;
    uint32_t microTileIndexZ = slice / microTileThickness;

    long microTileOffset = microTileBytes * (microTileIndexX + microTileIndexY * microTilesPerRow);
    long sliceBytes = (long)(pitch * height * microTileThickness * bpp + 7) / 8;
    long sliceOffset = microTileIndexZ * sliceBytes;

    uint32_t pixelIndex = computePixelIndexWithinMicroTile(x, y, slice, bpp, tileMode, IsDepth);
    long pixelOffset = (bpp * pixelIndex) >> 3;

    return pixelOffset + microTileOffset + sliceOffset;
}

uint32_t computePipeFromCoordWoRotation(uint32_t x, uint32_t y)
{
    return ((y >> 3) ^ (x >> 3)) & 1;
}

uint32_t computeBankFromCoordWoRotation(uint32_t x, uint32_t y)
{
    return ((y >> 5) ^ (x >> 3)) & 1 | 2 * (((y >> 4) ^ (x >> 4)) & 1);
}

uint32_t computeSurfaceRotationFromTileMode(uint32_t tileMode)
{
    switch (tileMode)
    {
    case 0x04: // AddrTileMode.ADDR_TM_2D_TILED_THIN1:
    case 0x05: // AddrTileMode.ADDR_TM_2D_TILED_THIN2:
    case 0x06: // AddrTileMode.ADDR_TM_2D_TILED_THIN4:
    case 0x07: // AddrTileMode.ADDR_TM_2D_TILED_THICK:
    case 0x08: // AddrTileMode.ADDR_TM_2B_TILED_THIN1:
    case 0x09: // AddrTileMode.ADDR_TM_2B_TILED_THIN2:
    case 0x0A: // AddrTileMode.ADDR_TM_2B_TILED_THIN4:
    case 0x0B: // AddrTileMode.ADDR_TM_2B_TILED_THICK:
        return 2;
    case 0x0C: // AddrTileMode.ADDR_TM_3D_TILED_THIN1:
    case 0x0D: //  AddrTileMode.ADDR_TM_3D_TILED_THICK:
    case 0x0E: // AddrTileMode.ADDR_TM_3B_TILED_THIN1:
    case 0x0F: // AddrTileMode.ADDR_TM_3B_TILED_THICK:
        return 1;
    default:
        return 0;
    }
}

long computeSurfaceAddrFromCoordMacroTiled(uint32_t x, uint32_t y, uint32_t slice, uint32_t sample, uint32_t bpp, uint32_t pitch, uint32_t height, uint32_t numSamples, uint32_t tileMode, bool IsDepth, uint32_t pipeSwizzle, uint32_t bankSwizzle)
{
    uint32_t microTileThickness = computeSurfaceThickness(tileMode);

    uint32_t microTileBits = numSamples * bpp * (microTileThickness * 64);
    uint32_t microTileBytes = (microTileBits + 7) / 8;

    uint32_t pixelIndex = computePixelIndexWithinMicroTile(x, y, slice, bpp, tileMode, IsDepth);
    uint32_t bytesPerSample = microTileBytes / numSamples;
    uint32_t sampleOffset = 0;
    uint32_t pixelOffset = 0;
    uint32_t samplesPerSlice = 0;
    uint32_t numSampleSplits = 0;
    uint32_t sampleSlice = 0;

    if (IsDepth)
    {
        sampleOffset = bpp * sample;
        pixelOffset = numSamples * bpp * pixelIndex;
    }
    else
    {
        sampleOffset = sample * (microTileBits / numSamples);
        pixelOffset = bpp * pixelIndex;
    }

    uint32_t elemOffset = pixelOffset + sampleOffset;

    if (numSamples <= 1 || microTileBytes <= 2048)
    {
        samplesPerSlice = numSamples;
        numSampleSplits = 1;
        sampleSlice = 0;
    }
    else
    {
        samplesPerSlice = 2048 / bytesPerSample;
        numSampleSplits = numSamples / samplesPerSlice;
        numSamples = samplesPerSlice;

        uint32_t tileSliceBits = microTileBits / numSampleSplits;
        sampleSlice = elemOffset / tileSliceBits;
        elemOffset %= tileSliceBits;
    }

    elemOffset = (elemOffset + 7) / 8;

    uint32_t pipe = computePipeFromCoordWoRotation(x, y);
    uint32_t bank = computeBankFromCoordWoRotation(x, y);

    uint32_t swizzle_ = pipeSwizzle + 2 * bankSwizzle;
    uint32_t bankPipe = pipe + 2 * bank;
    uint32_t rotation = computeSurfaceRotationFromTileMode(tileMode);
    uint32_t sliceIn = slice;

    if (isThickMacroTiled(tileMode) != 0)
        sliceIn >>= 2;

    bankPipe ^= 2 * sampleSlice * 3 ^ (swizzle_ + sliceIn * rotation);
    bankPipe %= 8;

    pipe = bankPipe % 2;
    bank = bankPipe / 2;

    uint32_t sliceBytes = (height * pitch * microTileThickness * bpp * numSamples + 7) / 8;
    uint32_t sliceOffset = sliceBytes * (sampleSlice + numSampleSplits * slice) / microTileThickness;

    uint32_t macroTilePitch = 32;
    uint32_t macroTileHeight = 16;

    switch (tileMode)
    {
        case 0x5: // AddrTileMode.ADDR_TM_2D_TILED_THIN2:
        case 0x9: // AddrTileMode.ADDR_TM_2B_TILED_THIN2:
            {
                macroTilePitch = 16;
                macroTileHeight = 32;
                break;
            }

        case 0x6: // AddrTileMode.ADDR_TM_2D_TILED_THIN4:
        case 0x1: // AddrTileMode.ADDR_TM_2B_TILED_THIN4:
            {
                macroTilePitch = 8;
                macroTileHeight = 64;
                break;
            }
    }

    uint32_t macroTilesPerRow = pitch / macroTilePitch;
    uint32_t macroTileBytes = (numSamples * microTileThickness * bpp * macroTileHeight
        * macroTilePitch + 7) / 8;
    uint32_t macroTileIndexX = x / macroTilePitch;
    uint32_t macroTileIndexY = y / macroTileHeight;
    long macroTileOffset = (macroTileIndexX + macroTilesPerRow * macroTileIndexY) * macroTileBytes;

    uint8_t bankSwapOrder[10] = {0, 1, 3, 2, 6, 7, 5, 4, 0, 0};

    if (isBankSwappedTileMode(tileMode) != 0)
    {
        uint32_t bankSwapWidth = computeSurfaceBankSwappedWidth(tileMode, bpp, 1, pitch);
        uint32_t swapIndex = macroTilePitch * macroTileIndexX / bankSwapWidth;
        bank ^= bankSwapOrder[swapIndex & 3];
    }

    long totalOffset = elemOffset + ((macroTileOffset + sliceOffset) >> 3);
    return bank << 9 | pipe << 8 | totalOffset & 255 | (long)((int)totalOffset & -256) << 3;
}

bool IsFormatBCN(uint32_t Format)
{
    switch (Format)
    {
    case 0x31:  // GX2SurfaceFormat.T_BC1_UNORM:
    case 0x431: // GX2SurfaceFormat.T_BC1_SRGB:
    case 0x32:  // GX2SurfaceFormat.T_BC2_UNORM:
    case 0x432: // GX2SurfaceFormat.T_BC2_SRGB:
    case 0x33:  // GX2SurfaceFormat.T_BC3_UNORM:
    case 0x433: // GX2SurfaceFormat.T_BC3_SRGB:
    case 0x34:  // GX2SurfaceFormat.T_BC4_UNORM:
    case 0x234: // GX2SurfaceFormat.T_BC4_SNORM:
    case 0x235: // GX2SurfaceFormat.T_BC5_SNORM:
    case 0x35:  // GX2SurfaceFormat.T_BC5_UNORM:
        return true;
    default:
        return false;
    }
}

BYTE* swizzleSurf(uint32_t width, uint32_t height, uint32_t depth, uint32_t format, uint32_t aa, uint32_t use, uint32_t tileMode, uint32_t swizzle_, uint32_t pitch, uint32_t bitsPerPixel, uint32_t slice, uint32_t sample, BYTE* data, uint32_t swizzle, uint32_t dataSize, noeRAPI_t* rapi)
{
    uint32_t bytesPerPixel = bitsPerPixel / 8;

    uint32_t pipeSwizzle, bankSwizzle, pos_;
    long pos;

    BYTE* result = (BYTE*)rapi->Noesis_UnpooledAlloc(dataSize);
    
    if (IsFormatBCN(format))
    {
        width = (width + 3) / 4;
        height = (height + 3) / 4;
    }

    pipeSwizzle = (swizzle_ >> 8) & 1;
    bankSwizzle = (swizzle_ >> 9) & 3;

    if (depth > 1)
    {
        // bankSwizzle = (uint32_t)(slice % 4);
    }

    tileMode = GX2TileModeToAddrTileMode(tileMode);

    bool IsDepth = (use & 4) != 0;
    uint32_t numSamples = (uint32_t)(1 << (int)aa);

    for (uint32_t y = 0; y < height; y++)
    {
        for (uint32_t x = 0; x < width; x++)
        {
            if (tileMode == 0 || tileMode == 1)
            {
                pos = computeSurfaceAddrFromCoordLinear(x, y, slice, sample, bytesPerPixel, pitch, height, depth);
            }
            else if (tileMode == 2 || tileMode == 3)
            {
                pos = computeSurfaceAddrFromCoordMicroTiled(x, y, slice, bitsPerPixel, pitch, height, tileMode, IsDepth);
            }
            else
            {
                pos = computeSurfaceAddrFromCoordMacroTiled(x, y, slice, sample, bitsPerPixel, pitch, height, numSamples, tileMode, IsDepth, pipeSwizzle, bankSwizzle);
            }


            pos_ = (y * width + x) * bytesPerPixel;

            if (pos_ + bytesPerPixel <= dataSize && pos + bytesPerPixel <= dataSize)
            {
                if (swizzle == 0)
                {
                    for (uint32_t n = 0; n < bytesPerPixel; n++)
                        result[pos_ + n] = data[pos + n];
                }
                else
                {
                    for (uint32_t n = 0; n < bytesPerPixel; n++)
                        result[pos + n] = data[pos_ + n];
                }
            }
        }
    }
    return result;
}

BYTE* wiiUdeswizzle(uint32_t width, uint32_t height, uint32_t depth, uint32_t height_, uint32_t format_, uint32_t aa, uint32_t use, uint32_t tileMode, uint32_t swizzle_, uint32_t pitch, uint32_t bpp, uint32_t slice, uint32_t sample, BYTE* dataSrc, uint32_t dataSize, noeRAPI_t* rapi)
{
    return swizzleSurf(width, height, depth, format_, aa, use, tileMode, swizzle_, pitch, bpp, slice, sample, dataSrc, 0, dataSize, rapi);
}

static uint8_t formatHwInfo[64*4] = {
//  block 
//  size  expandX expandY elemMode
    0x00, 0x00,   0x00,   0x01, // 0x00
    0x08, 0x03,   0x00,   0x01, // 0x01 GX2_SURFACE_FORMAT_UNORM_R8
    0x08, 0x01,   0x00,   0x01, // 0x02 T_R4_G4
    0x00, 0x00,   0x00,   0x01, // 0x03
    0x00, 0x00,   0x00,   0x01, // 0x04
    0x10, 0x07,   0x00,   0x00, // 0x05 GX2_SURFACE_FORMAT_UNORM_R1
    0x10, 0x03,   0x00,   0x01, // 0x06 TC_R16_FLOAT
    0x10, 0x03,   0x00,   0x01, // 0x07 GX2_SURFACE_FORMAT_UNORM_R8_G8
    0x10, 0x0B,   0x00,   0x01, // 0x08 GX2_SURFACE_FORMAT_UNORM_R5_G6_B5
    0x10, 0x01,   0x00,   0x01, // 0x09
    0x10, 0x03,   0x00,   0x01, // 0x0A GX2_SURFACE_FORMAT_UNORM_R5_G5_B5_A1
    0x10, 0x03,   0x00,   0x01, // 0x0B GX2_SURFACE_FORMAT_UNORM_R4_G4_B4_A4
    0x10, 0x03,   0x00,   0x01, // 0x0C A1_B5_G5_R5
    0x20, 0x03,   0x00,   0x00, // 0x0D R32
    0x20, 0x07,   0x00,   0x00, // 0x0E R32_FLOAT
    0x20, 0x03,   0x00,   0x00, // 0x0F R16_G16
    0x20, 0x03,   0x00,   0x01, // 0x10 R16_G16_FLOAT
    0x20, 0x05,   0x00,   0x00, // 0x11 GX2_SURFACE_FORMAT_UNORM_R24_X8 
    0x00, 0x00,   0x00,   0x00, // 0x12
    0x20, 0x03,   0x00,   0x00, // 0x13
    0x00, 0x00,   0x00,   0x00, // 0x14
    0x00, 0x00,   0x00,   0x01, // 0x15
    0x20, 0x03,   0x00,   0x01, // 0x16 R11_G11_B10_FLOAT
    0x00, 0x00,   0x00,   0x01, // 0x17 
    0x00, 0x00,   0x00,   0x01, // 0x18 
    0x20, 0x0B,   0x00,   0x01, // 0x19 GX2_SURFACE_FORMAT_UNORM_R10_G10_B10_A2
    0x20, 0x0B,   0x00,   0x01, // 0x1A R8_G8_B8_A8
    0x20, 0x0B,   0x00,   0x01, // 0x1B A2_B20_G10_R10
    0x40, 0x05,   0x00,   0x00, // 0x1C D32_FLOAT_S8_UINT_X24
    0x40, 0x03,   0x00,   0x00, // 0x1D R32_G32
    0x40, 0x03,   0x00,   0x00, // 0x1E R32_G32_FLOAT
    0x40, 0x03,   0x00,   0x00, // 0x1F GX2_SURFACE_FORMAT_UNORM_R16_G16_B16_A16
    0x40, 0x03,   0x00,   0x01, // 0x20 R16_G16_B16_A16
    0x00, 0x00,   0x00,   0x00, // 0x21
    0x80, 0x03,   0x00,   0x00, // 0x22 R32_G32_B32_A32
    0x80, 0x03,   0x00,   0x00, // 0x23 R32_G32_B32_A32_FLOAT
    0x00, 0x00,   0x00,   0x01, // 0x24
    0x00, 0x00,   0x00,   0x01, // 0x25
    0x00, 0x00,   0x00,   0x01, // 0x26
    0x10, 0x01,   0x00,   0x00, // 0x27
    0x10, 0x01,   0x00,   0x00, // 0x28
    0x20, 0x01,   0x00,   0x00, // 0x29
    0x20, 0x01,   0x00,   0x00, // 0x2A
    0x20, 0x01,   0x00,   0x00, // 0x2B
    0x00, 0x01,   0x00,   0x01, // 0x2C
    0x00, 0x01,   0x00,   0x00, // 0x2D
    0x00, 0x01,   0x00,   0x00, // 0x2E
    0x60, 0x01,   0x00,   0x00, // 0x2F
    0x60, 0x01,   0x00,   0x00, // 0x30
    0x40, 0x01,   0x00,   0x01, // 0x31 GX2_SURFACE_FORMAT_UNORM_BC1
    0x80, 0x01,   0x00,   0x01, // 0x32 GX2_SURFACE_FORMAT_UNORM_BC2
    0x80, 0x01,   0x00,   0x01, // 0x33 GX2_SURFACE_FORMAT_UNORM_BC3
    0x40, 0x01,   0x00,   0x01, // 0x34 GX2_SURFACE_FORMAT_UNORM_BC4
    0x80, 0x01,   0x00,   0x01, // 0x35 GX2_SURFACE_FORMAT_UNORM_BC5
    0x00, 0x00,   0x00,   0x00, // 0x36 
    0x00, 0x00,   0x00,   0x00, // 0x37 
    0x00, 0x00,   0x00,   0x00, // 0x38 
    0x00, 0x00,   0x00,   0x00, // 0x39 
    0x00, 0x00,   0x00,   0x00, // 0x3A 
    0x00, 0x00,   0x00,   0x00, // 0x3B 
    0x00, 0x00,   0x00,   0x00, // 0x3C 
    0x00, 0x00,   0x00,   0x00, // 0x3D  
    0x00, 0x00,   0x00,   0x00, // 0x3E 
    0x00, 0x00,   0x00,   0x00, // 0x3F 
};

Gx2SurfaceOut getSurfaceInfo(uint32_t surfaceFormat, uint32_t surfaceWidth, uint32_t surfaceHeight, uint32_t surfaceDepth, uint32_t surfaceDim, uint32_t surfaceTileMode, uint32_t surfaceAA, int level, Gx2SurfaceIn* pIn, Gx2SurfaceOut* pOut)
{
    uint32_t dim = 0;
    uint32_t width = 0;
    uint32_t blockSize = 0;
    uint32_t numSamples = 0;
    uint32_t hwFormat = 0;

    Gx2SurfaceIn aSurfIn = { 0 };
    Gx2SurfaceOut pSurfOut = { 0 };

    hwFormat = (uint32_t)((int)surfaceFormat & 0x3F);


    if (surfaceTileMode == 16)
    {
        numSamples = (uint32_t)(1 << (int)surfaceAA); // should always be 1

        if (hwFormat < 0x31 || hwFormat > 0x35)
            blockSize = 1;
        else
            blockSize = 4;

        width = (uint32_t)(~(blockSize - 1) & (max(1, surfaceWidth >> level) + blockSize - 1));

        pSurfOut.bpp = (uint32_t)formatHwInfo[hwFormat * 4];
        pSurfOut.size = 96;
        pSurfOut.pitch = (uint32_t)(width / blockSize);
        pSurfOut.pixelBits = (uint32_t)formatHwInfo[hwFormat * 4];
        pSurfOut.baseAlign = 1;
        pSurfOut.pitchAlign = 1;
        pSurfOut.heightAlign = 1;
        pSurfOut.depthAlign = 1;
        dim = surfaceDim;

        if (dim == 0)
        {
            pSurfOut.height = 1;
            pSurfOut.depth = 1;
        }
        else if (dim == 1 || dim == 6)
        {
            pSurfOut.height = max(1, surfaceHeight >> level);
            pSurfOut.depth = 1;
        }
        else if (dim == 2)
        {
            pSurfOut.height = max(1, surfaceHeight >> level);
            pSurfOut.depth = max(1, surfaceDepth >> level);
        }
        else if (dim == 3)
        {
            pSurfOut.height = max(1, surfaceHeight >> level);
            pSurfOut.depth = max(6, surfaceDepth);
        }
        else if (dim == 4)
        {
            pSurfOut.height = 1;
            pSurfOut.depth = surfaceDepth;
        }
        else if (dim == 5 || dim == 7)
        {
            pSurfOut.height = max(1, surfaceHeight >> level);
            pSurfOut.depth = surfaceDepth;
        }

        pSurfOut.pixelPitch = width;
        pSurfOut.pixelHeight = (uint32_t)(~(blockSize - 1) & (pSurfOut.height + blockSize - 1));
        pSurfOut.height = (uint32_t)(pSurfOut.pixelHeight / blockSize);
        pSurfOut.surfSize = (uint32_t)(pSurfOut.bpp * numSamples * pSurfOut.depth * pSurfOut.height * pSurfOut.pitch >> 3);

        if (surfaceDim == 2)
            pSurfOut.sliceSize = (uint32_t)pSurfOut.surfSize;
        else
            pSurfOut.sliceSize = (uint32_t)(pSurfOut.surfSize / pSurfOut.depth);

        pSurfOut.pitchTileMax = (pSurfOut.pitch >> 3) - 1;
        pSurfOut.heightTileMax = (pSurfOut.height >> 3) - 1;
        pSurfOut.sliceTileMax = (pSurfOut.height * pSurfOut.pitch >> 6) - 1;

    }
    else
    {
        aSurfIn.size = 60;
        aSurfIn.tileMode = surfaceTileMode & 0x0F;
        aSurfIn.format = hwFormat;
        aSurfIn.bpp = (uint32_t)formatHwInfo[hwFormat * 4];
        aSurfIn.numSamples = (uint32_t)(1 << (int)surfaceAA);
        aSurfIn.numFrags = aSurfIn.numSamples;
        aSurfIn.width = (uint32_t)max(1, surfaceWidth >> level);
        dim = surfaceDim;

        if (dim == 0)
        {
            aSurfIn.height = 1;
            aSurfIn.numSlices = 1;
        }
        else if (dim == 1 || dim == 6)
        {
            aSurfIn.height = (uint32_t)max(1, surfaceHeight >> level);
            aSurfIn.numSlices = 1;
        }
        else if (dim == 2)
        {
            aSurfIn.height = (uint32_t)max(1, surfaceHeight >> level);
            aSurfIn.numSlices = (uint32_t)max(1, surfaceDepth >> level);
        }
        else if (dim == 3)
        {
            aSurfIn.height = (uint32_t)max(1, surfaceHeight >> level);
            aSurfIn.numSlices = (uint32_t)max(6, surfaceDepth);
            aSurfIn.flags.value |= 0x10;
        }
        else if (dim == 4)
        {
            aSurfIn.height = 1;
            aSurfIn.numSlices = surfaceDepth;
        }
        else if (dim == 5 || dim == 7)
        {
            aSurfIn.height = (uint32_t)max(1, surfaceHeight >> level);
            aSurfIn.numSlices = surfaceDepth;
        }

        aSurfIn.slice = 0;
        aSurfIn.mipLevel = (uint32_t)level;

        if (surfaceDim == 2)
            aSurfIn.flags.value |= 0x20;

        if (level == 0)
            aSurfIn.flags.value = (1 << 12) | aSurfIn.flags.value & 0xFFFFEFFF;
        else
            aSurfIn.flags.value = aSurfIn.flags.value & 0xFFFFEFFF;

        pSurfOut.size = 96;
        computeSurfaceInfo(&aSurfIn, &pSurfOut, pIn, pOut);

        *&pSurfOut = *pOut;
    }
    if (pSurfOut.tileMode == 0)
        pSurfOut.tileMode = 16;

    return pSurfOut;
}

uint32_t RoundUp(uint32_t X, uint32_t Y)
{
    return ((X - 1) | (Y - 1)) + 1;
}

void GenerateMipOffsets(GX2Surface* tex, Gx2SurfaceIn* pIn, Gx2SurfaceOut* pOut)
{
    Gx2SurfaceOut surfOut = getSurfaceInfo(tex->format, tex->width, tex->height, 1, 1, tex->tileMode, 0, 0, pIn, pOut);
    uint32_t imageSize = surfOut.surfSize;
    uint32_t mipSize = 0;

    //List<uint> mipOffsets = new List<uint>();
    for (int mipLevel = 0; mipLevel < tex->numMips; mipLevel++)
    {
        if (mipLevel != 0)
        {
            surfOut = getSurfaceInfo(tex->format, tex->width, tex->height, 1, 1, tex->tileMode, 0, mipLevel, pIn, pOut);

            if (mipLevel == 1)
                tex->mipOffset[mipLevel] = imageSize;
            else
                tex->mipOffset[mipLevel] = mipSize;

            //byte[] dataAlignBytes = new byte[RoundUp(mipSize, surfOut.baseAlign) - mipSize];
            mipSize += (uint32_t)(surfOut.surfSize + RoundUp(mipSize, surfOut.baseAlign) - mipSize);
        }
    }
}

// Starting here
// https://github.com/KillzXGaming/Switch-Toolbox/blob/16de585c7b998826e39ac88915f64554c0025423/Switch_Toolbox_Library/Texture%20Decoding/Wii%20U/GX2.cs#L697
BYTE* Gx2Decode(GX2Surface tex, uint32_t ArrayIndex, uint32_t MipIndex, noeRAPI_t* rapi)
{
    Gx2SurfaceIn pIn = { 0 };
    Gx2SurfaceOut pOut = { 0 };

    uint32_t blkWidth,  blkHeight;

    if (IsFormatBCN(tex.format))
    {
        blkWidth = 4;
        blkHeight = 4;
    }
    else
    {
        blkWidth = 1;
        blkHeight = 1;
    }

    GenerateMipOffsets(&tex, &pIn, &pOut);

    Gx2SurfaceOut ImageSurfInfo = getSurfaceInfo(tex.format, tex.width, tex.height, 1, 1, tex.tileMode, 0, 0, &pIn, &pOut);
    uint32_t bpp = DIV_ROUND_UP(ImageSurfInfo.bpp, 8);

    if (tex.numArray == 0)
        tex.numArray = 1;

    //if (tex.data.Length <= 0)
    //    tex.data = tex.mipData;

    BYTE* data = (BYTE*)rapi->Noesis_UnpooledAlloc(tex.imageSize); // must free
    BYTE* mipdata = data; // not going to use mips

    uint32_t mipCount = tex.numMips;
    if (mipCount <= 0)
        mipCount = 1;

    uint32_t dataOffset = 0;
    uint32_t mipDataOffset = 0;

    for (uint32_t arrayLevel = 0; arrayLevel < tex.numArray; arrayLevel++)
    {
        uint32_t mipSpliceSize = 0;

        for (uint32_t mipLevel = 0; mipLevel < mipCount; mipLevel++)
        {
            Gx2SurfaceOut MipSurfInfo = getSurfaceInfo(tex.format, tex.width, tex.height, tex.depth, tex.dim, tex.tileMode, tex.aa, mipLevel, &pIn, &pOut);

            bool GetLevel = (arrayLevel == ArrayIndex && mipLevel == MipIndex);

            uint32_t swizzle = tex.swizzle;

            uint32_t width_ = max(1, tex.width >> mipLevel);
            uint32_t height_ = max(1, tex.height >> mipLevel);

            uint32_t size = DIV_ROUND_UP(width_, blkWidth) * DIV_ROUND_UP(height_, blkHeight) * bpp;

            uint32_t mipOffset = 0;
            if (mipLevel != 0) // only getting first level
            {
                if (tex.mip_swizzle != 0)
                    swizzle = tex.mip_swizzle;

                mipOffset = (tex.mipOffset[mipLevel - 1]);
                if (mipLevel == 1)
                {
                    mipOffset -= ImageSurfInfo.surfSize;
                    mipSpliceSize = MipSurfInfo.sliceSize;
                }
                if (GetLevel)
                {
                    memcpy(data, tex.data, tex.imageSize);
                    memcpy(data, tex.data + mipOffset, MipSurfInfo.sliceSize);
                    data = mipdata;
                }
            }
            else if (GetLevel)
            {
                memcpy(data, tex.data, tex.imageSize);
                memcpy(data, tex.data + dataOffset, size);
            }

            if (GetLevel)
            {
               BYTE* deswizzled = wiiUdeswizzle(width_, height_, MipSurfInfo.depth, MipSurfInfo.height, tex.format, 0, tex.use,
                    MipSurfInfo.tileMode, swizzle, MipSurfInfo.pitch, MipSurfInfo.bpp, arrayLevel, 0, data, tex.imageSize, rapi);
               BYTE* result_ = (BYTE*)rapi->Noesis_UnpooledAlloc(size);
               memcpy(result_, deswizzled, size);
               rapi->Noesis_UnpooledFree(data);
               rapi->Noesis_UnpooledFree(deswizzled);
               return result_;
            }
        }
        
        dataOffset += ImageSurfInfo.sliceSize;
        mipDataOffset += mipSpliceSize;
    }
    return NULL;
}

#endif // !G1T_FORMAT_CONVERT_H