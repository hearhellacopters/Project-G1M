#pragma once

#ifndef G1T_H
#define G1T_H

///////////////////////////////
//           ENUMS           //
///////////////////////////////

enum class PLATFORM : uint32_t
{
	PS2     = 0x00, // Little Endian
	PS3     = 0x01, // Big Endian
	X360    = 0x02, // Big Endian
	NWii    = 0x03, // Big Endian
  //NDS     = 0x04, // Little Endian
	N3DS    = 0x05, // Little Endian
	PSVita  = 0x06, // Little Endian
	Android = 0x07, // Little Endian
	iOS     = 0x08, // Little Endian
	NWiiU   = 0x09, // Big Endian
	WinMac  = 0x0A, // Little Endian
	PS4     = 0x0B, // Little Endian
  //XOne    = 0x0C, // Little Endian - game rom to confrim this
  //???     = 0x0D, 
	WinDX12 = 0x0E, // Little Endian
  //???     = 0x0F, 
	NSwitch = 0x10, // Little Endian
  //???     = 0x11, 
  //???     = 0x12, 
	PS5     = 0x13, // Little Endian
};

enum class TEX_ATTR_TYPE  : uint16_t
{
	// These are sometime directly hardcoded to the client.
	// Meaning that they don't appear in the file, 
	// but are hardcodes on every g1t the game reads
	NONE               = 0x00,
  //???                = 0x01,
  //???                = 0x02,
	NORMAL_MAP         = 0x03,
	NORMAL_WITH_ALPHA  = 0x04,
	N3DS_Wii_SWIZZLE   = 0x05,
	WiiU_SWIZZLE       = 0x06,
	PS4_PLANE_ARRAY    = 0x07,
	PSVITA_PS5_SWIZZLE = 0x08,
  //???                = 0x09,
  //???                = 0x0A,
  //???                = 0x0B,
  //???                = 0x0C,
  //???                = 0x0D,
  //???                = 0x0E,
  //???                = 0x0F,
  //???                = 0x10,
  //???                = 0x11,
	ASTC_TYPE          = 0x15
};

enum class EX_SWIZZLE_TYPE : uint8_t
{
	NONE      = 0x00,
	DX12_64kb = 0x01,
};

enum class S_GT1_LOAD_TYPE : uint8_t
{
	PLANAR      = 0x0,
	CUBE        = 0x1,
	VOLUME      = 0x2,
	PLANE_ARRAY = 0x3,
	CUBE_ARRAY  = 0x4     // ver 63 and above
};

enum class EG1TASTCFormat : uint32_t 
{
	ASTC_4x4   = 0x0,
	ASTC_5x4   = 0x1,
	ASTC_5x5   = 0x2,
	ASTC_6x5   = 0x3,
	ASTC_6x6   = 0x4,
	ASTC_8x5   = 0x5,
	ASTC_8x6   = 0x6,
	ASTC_8x8   = 0x7,
	ASTC_10x5  = 0x8,
	ASTC_10x6  = 0x9,
	ASTC_10x8  = 0xA,
	ASTC_10x10 = 0xB,
	ASTC_12x10 = 0xC,
	ASTC_12x12 = 0xD,
};

enum class KTGL_GD_COLOR_SPACE_TYPE : uint8_t
{
	NONE = 0x0,
	sRGB_FROM_LINEAR = 0x1,
};

///////////////////////////////
//     HELPER FUNCTIONS      //
///////////////////////////////

// Helper function to swap nibbles
void swapNibbles(uint8_t* a, uint8_t* b) {
	uint8_t temp = *a;
	*a = *b;
	*b = temp;
}

// Helper function to swap float array values
void SwapFloatArray(float* arr, size_t count)
{
	for (size_t i = 0; i < count; i++) {
		uint32_t* p = reinterpret_cast<uint32_t*>(&arr[i]);
		LITTLE_BIG_SWAP(*p);
	}
}

// Helper function for data size
uint32_t TextureSizeWithMips(
	uint32_t width, uint32_t height, uint32_t depth,
	uint32_t mipCount, uint32_t arrayCount, uint32_t faceCount,
	uint32_t blockByteSize, uint32_t blockWidth, uint32_t blockHeight,
	uint32_t bCompressedFormat, double bitsPerPixel
)
{
	uint32_t totalSize = 0;
	uint32_t w = width;
	uint32_t h = height;
	uint32_t d = depth;
	uint32_t mip = 0;
	uint32_t blocksX = 0;
	uint32_t blocksY = 0;
	uint32_t mipSize = 0;

	for (mip = 0; mip < mipCount; ++mip)
	{
		mipSize = 0;
		w = width >> mip;  if (w == 0) w = 1;
		h = height >> mip; if (h == 0) h = 1;
		d = depth >> mip;  if (d == 0) d = 1;

		if (bCompressedFormat)
		{
			blocksX = (w + blockWidth - 1) / blockWidth;   if (blocksX == 0) blocksX = 1;
			blocksY = (h + blockHeight - 1) / blockHeight; if (blocksY == 0) blocksY = 1;

			mipSize = blocksX * blocksY * d * blockByteSize;

			// Sanity check for smallest block size
			if (mipSize < blockByteSize)
			{
				mipSize = blockByteSize;
			}

			totalSize += mipSize;
		}
		else
		{
			mipSize = (uint32_t)((w * h * d) * (bitsPerPixel / 8));

			// Sanity check for smallest pixel size
			if (mipSize < blockByteSize)
			{
				mipSize = blockByteSize;
			}

			totalSize += mipSize;
		}
	}

	return totalSize * arrayCount * faceCount;
}

///////////////////////////////
//      HEADER STRUCTS       //
///////////////////////////////

template <bool bBigEndian>
struct G1T_HEADER
{
	char MAGIC[4] = {};
	uint32_t VERSION = 0;
	uint32_t VERSION_NUMBER = 0;
	uint32_t FILE_SIZE = 0;
	uint32_t TEX_OFFSET = 0;
	uint32_t TEX_COUNT = 0;
	// VERSION_NUMBER > 30 always 0 for SYSTEM
	PLATFORM SYSTEM = PLATFORM::PS2;
	// VERSION_NUMBER > 50 no EX Headers
	uint32_t HEADER_EX_SIZE = 0;
	uint32_t HEADER_SIZE = 20;
	bool bPanic = false;

	G1T_HEADER(BYTE* buffer, uint32_t& offset, noeRAPI_t* rapi, int bufferLen)
	{
		// panic check
		if (HEADER_SIZE > (uint32_t)bufferLen)
		{
			bPanic = true;
			return;
		}

		MAGIC[0] = (*(char*)(buffer + offset)); offset += 1;
		MAGIC[1] = (*(char*)(buffer + offset)); offset += 1;
		MAGIC[2] = (*(char*)(buffer + offset)); offset += 1;
		MAGIC[3] = (*(char*)(buffer + offset)); offset += 1;

		const char EXPECTED_BIG_MAGIC[4] = { 'G', '1', 'T', 'G' };
		const char EXPECTED_LITTLE_MAGIC[4] = { 'G', 'T', '1', 'G' };

		// panic check
		if ( !(memcmp(MAGIC, EXPECTED_BIG_MAGIC, 4) == 0 ||
			 memcmp(MAGIC, EXPECTED_LITTLE_MAGIC, 4) == 0)
			)
		{
			bPanic = true;
		}

		VERSION = (*(uint32_t*)(buffer + offset)); if (bBigEndian) LITTLE_BIG_SWAP(VERSION); offset += 4;

		// get the version number
		VERSION_NUMBER = 
			(((VERSION >> 24 ) & 0xFF) - 0x30) * 1000 +
			(((VERSION >> 16 ) & 0xFF) - 0x30) * 100 +
			(((VERSION >> 8  ) & 0xFF) - 0x30) * 10 +
			  (VERSION & 0xFF) - 0x30;

		FILE_SIZE = (*(uint32_t*)(buffer + offset));  if (bBigEndian) LITTLE_BIG_SWAP(FILE_SIZE); offset += 4;

		// panic check
		if (FILE_SIZE > (uint32_t)bufferLen)
		{
			bPanic = true;
		}

		TEX_OFFSET = *(uint32_t*)(buffer + offset);  if (bBigEndian) LITTLE_BIG_SWAP(TEX_OFFSET); offset += 4;

		// panic check
		if (TEX_OFFSET > (uint32_t)bufferLen)
		{
			bPanic = true;
		}

		TEX_COUNT = *(uint32_t*)(buffer + offset); if (bBigEndian) LITTLE_BIG_SWAP(TEX_COUNT); offset += 4;

		if (VERSION_NUMBER < 30)
		{
			SYSTEM = PLATFORM::PS2;
			HEADER_EX_SIZE = 0;
			HEADER_SIZE = 20;
		}
		else if (VERSION_NUMBER < 50)
		{
			HEADER_SIZE = 24;
			// panic check
			if (HEADER_SIZE > (uint32_t)bufferLen)
			{
				bPanic = true;
				return;
			}
			SYSTEM = *(PLATFORM*)(buffer + offset); if (bBigEndian) LITTLE_BIG_SWAP(SYSTEM); offset += 4;
			HEADER_EX_SIZE = 0;
		}
		else
		{
			HEADER_SIZE = 28;
			// panic check
			if (HEADER_SIZE > (uint32_t)bufferLen)
			{
				bPanic = true;
				return;
			}
			SYSTEM = *(PLATFORM*)(buffer + offset); if (bBigEndian) LITTLE_BIG_SWAP(SYSTEM); offset += 4;
			HEADER_EX_SIZE = *(uint32_t*)(buffer + offset); if (bBigEndian) LITTLE_BIG_SWAP(HEADER_EX_SIZE); offset += 4;
		}

		if (bDebugLog)
		{
			char mesBuffer[150] = {};
			// File name for texture renaming
			std::string inFile = rapi->Noesis_GetLastCheckedName();
			std::filesystem::path pathObj(inFile);
			std::string fileName = pathObj.stem().string();
			LogDebug("G1T_FILE_NAME:\t\t\t%s\n", fileName.c_str());
			LogDebug("G1T_HEADER:\n");
			LogDebug("\tMAGIC:\t\t\t%c%c%c%c (%s)\n", MAGIC[0], MAGIC[1], MAGIC[2], MAGIC[3], bBigEndian ? "Big Endian" : "Little Endian");
			LogDebug("\tVERSION:\t\t%d\n", VERSION_NUMBER);
			LogDebug("\tFILE_SIZE:\t\t%d\n", FILE_SIZE);
			LogDebug("\tTEX_OFFSET:\t\t%d\n", TEX_OFFSET);
			LogDebug("\tTEX_COUNT:\t\t%d\n", TEX_COUNT);
			std::string SYSTEM_STR = "";
			switch (SYSTEM)
			{
			case PLATFORM::PS2:
				SYSTEM_STR = "PS2";
				break;
			case PLATFORM::PS3:
				SYSTEM_STR = "PS3";
				break;
			case PLATFORM::X360:
				SYSTEM_STR = "XBOX 360";
				break;
			case PLATFORM::NWii:
				SYSTEM_STR = "Nintendo Wii";
				break;
			case PLATFORM::N3DS:
				SYSTEM_STR = "Nintendo 3DS";
				break;
			case PLATFORM::PSVita:
				SYSTEM_STR = "PS Vita";
				break;
			case PLATFORM::Android:
				SYSTEM_STR = "Android";
				break;
			case PLATFORM::iOS:
				SYSTEM_STR = "iOS";
				break;
			case PLATFORM::NWiiU:
				SYSTEM_STR = "Nintendo Wii U";
				break;
			case PLATFORM::WinMac:
				SYSTEM_STR = "Windows / Mac";
				break;
			case PLATFORM::WinDX12:
				SYSTEM_STR = "Windows Direct-X 12";
				break;
			case PLATFORM::PS4:
				SYSTEM_STR = "PS4";
				break;
			case PLATFORM::NSwitch:
				SYSTEM_STR = "Nintendo Switch";
				break;
			case PLATFORM::PS5:
				SYSTEM_STR = "PS5";
				break;
			default:
				SYSTEM_STR = "Unknown System";
				break;
			}
			LogDebug("\tSYSTEM:\t\t\t%d (%s)\n", SYSTEM, SYSTEM_STR.c_str());
			LogDebug("\tHEADER_SIZE:\t\t%d\n", HEADER_SIZE);
			LogDebug("\tHEADER_EX_SIZE:\t\t%d\n\n\t\t-- TEXTURE INFO --\n\n", HEADER_EX_SIZE);
		}
	}
};

// Likely deprecated
template <bool bBigEndian>
struct S_G1T_TEX_ATTR_HEADER
{
	TEX_ATTR_TYPE TYPE = TEX_ATTR_TYPE::NONE;
	uint8_t COUNT = 0;
	uint8_t RESERVED_CONSUMED = 0;
	uint32_t headerSize = 4;
	std::vector<uint32_t> EXTRA_INTS; // Shouldn't be used here but here anyways
	bool bPanic = false;

	S_G1T_TEX_ATTR_HEADER(BYTE* buffer, uint32_t& offset, int bufferLen)
	{
		// panic check
		if ((offset + headerSize) > (uint32_t)bufferLen)
		{
			bPanic = true;
			return;
		}

		if (bBigEndian)
		{
			RESERVED_CONSUMED = *(uint8_t*)(buffer + offset); offset += 1;
			COUNT = *(uint8_t*)(buffer + offset); offset += 1;
			TYPE = *(TEX_ATTR_TYPE*)(buffer + offset); if (bBigEndian) LITTLE_BIG_SWAP(TYPE); offset += 2;
			headerSize += COUNT * 4;
		}
		else
		{
			TYPE = *(TEX_ATTR_TYPE*)(buffer + offset); if (bBigEndian) LITTLE_BIG_SWAP(TYPE); offset += 2;
			COUNT = *(uint8_t*)(buffer + offset); offset += 1;
			RESERVED_CONSUMED = *(uint8_t*)(buffer + offset); offset += 1;
			headerSize += COUNT * 4;
		}

		// panic check
		if(RESERVED_CONSUMED != 0)
		{
			bPanic = true;
		}

		// panic check
		if ((offset + (COUNT * 4)) > (uint32_t)bufferLen)
		{
			bPanic = true;
		}

		if (COUNT > 0)
		{
			EXTRA_INTS.resize(COUNT);
			for (uint8_t i = 0; i < COUNT; i++) {
				EXTRA_INTS[i] = *(uint32_t*)(buffer + offset + (i * sizeof(uint32_t))); if (bBigEndian) LITTLE_BIG_SWAP(EXTRA_INTS[i]);
			}
		}		
	}
};

template <bool bBigEndian>
struct S_G1T_HEADER_EX
{
	TEX_ATTR_TYPE TYPE = TEX_ATTR_TYPE::NONE;
	uint8_t COUNT = 0;
	uint8_t RESERVED_CONSUMED = 0;
	uint32_t headerSize = 4;
	EG1TASTCFormat KT_ASTC_FORMAT = EG1TASTCFormat::ASTC_12x12; // default
	uint32_t WiiU_SWIZZLE = 0;
	bool bPanic = false;

	// PS4 array data
	uint32_t ID1 = 0;
	float FLOAT1[4] = {};
	uint32_t ID2 = 0;
	float FLOAT2[4] = {};
	uint32_t ID3 = 0;
	float FLOAT3[5] = {};
	uint32_t ID4 = 0;
	float FLOAT4[4] = {};
	uint32_t ID5 = 0;
	float FLOAT5[4] = {};
	uint32_t ID6 = 0;
	float FLOAT6[5] = {};
	uint32_t ARRAY_DEPTH = 1;

	std::vector<uint32_t> EXTRA_INTS; // Sometimes used as KIDs

	S_G1T_HEADER_EX(BYTE* buffer, uint32_t& offset, int bufferLen)
	{
		// panic check
		if ((offset + headerSize) > (uint32_t)bufferLen)
		{
			bPanic = true;
			return;
		}

		if (bBigEndian)
		{
			RESERVED_CONSUMED = *(uint8_t*)(buffer + offset); offset += 1;
			COUNT = *(uint8_t*)(buffer + offset); offset += 1;
			TYPE = *(TEX_ATTR_TYPE*)(buffer + offset); if (bBigEndian) LITTLE_BIG_SWAP(TYPE); offset += 2;
			headerSize += COUNT * 4;
		}
		else
		{
			TYPE = *(TEX_ATTR_TYPE*)(buffer + offset); if (bBigEndian) LITTLE_BIG_SWAP(TYPE); offset += 2;
			COUNT = *(uint8_t*)(buffer + offset); offset += 1;
			RESERVED_CONSUMED = *(uint8_t*)(buffer + offset); offset += 1;
			headerSize += COUNT * 4;
		}
		
		// panic check
		if (RESERVED_CONSUMED != 0)
		{
			bPanic = true;
		}

		// panic check
		if (((headerSize - 4) + offset) > (uint32_t)bufferLen)
		{
			bPanic = true;
		}

		if (TYPE == TEX_ATTR_TYPE::ASTC_TYPE && COUNT == 1)
		{
			KT_ASTC_FORMAT = *(EG1TASTCFormat*)(buffer + offset); if (bBigEndian) LITTLE_BIG_SWAP(KT_ASTC_FORMAT); offset += 4;
		}
		else if (TYPE == TEX_ATTR_TYPE::WiiU_SWIZZLE && COUNT == 1)
		{
			WiiU_SWIZZLE = *(uint32_t*)(buffer + offset); if (bBigEndian) LITTLE_BIG_SWAP(WiiU_SWIZZLE); offset += 4;
		}
		else if (TYPE == TEX_ATTR_TYPE::PS4_PLANE_ARRAY && COUNT == 32)
		{
			// unsure what this is, Vector bases?
			ID1 = *(uint32_t*)(buffer + offset); if (bBigEndian) LITTLE_BIG_SWAP(ID1); offset += 4;
			memcpy(FLOAT1, buffer + offset, 16); if (bBigEndian) SwapFloatArray(FLOAT1, 4); offset += 16;
			ID2 = *(uint32_t*)(buffer + offset); if (bBigEndian) LITTLE_BIG_SWAP(ID2); offset += 4;
			memcpy(FLOAT2, buffer + offset, 16); if (bBigEndian) SwapFloatArray(FLOAT2, 4); offset += 16;
			ID3 = *(uint32_t*)(buffer + offset); if (bBigEndian) LITTLE_BIG_SWAP(ID3); offset += 4;
			memcpy(FLOAT3, buffer + offset, 20); if (bBigEndian) SwapFloatArray(FLOAT3, 5); offset += 20;
			ID4 = *(uint32_t*)(buffer + offset); if (bBigEndian) LITTLE_BIG_SWAP(ID4); offset += 4;
			memcpy(FLOAT4, buffer + offset, 16); if (bBigEndian) SwapFloatArray(FLOAT4, 4); offset += 16;
			ID5 = *(uint32_t*)(buffer + offset); if (bBigEndian) LITTLE_BIG_SWAP(ID5); offset += 4;
			memcpy(FLOAT5, buffer + offset, 16); if (bBigEndian) SwapFloatArray(FLOAT5, 4); offset += 16;
			ID6 = *(uint32_t*)(buffer + offset); if (bBigEndian) LITTLE_BIG_SWAP(ID6); offset += 4;
			memcpy(FLOAT6, buffer + offset, 20); if (bBigEndian) SwapFloatArray(FLOAT6, 5); offset += 20;
			// I have no idea what this is
			if (ID1 == 922948206)
			{
				ARRAY_DEPTH = 2;
			}
		}
		else
		{
			// Read 'COUNT' number of int values from the buffer
			EXTRA_INTS.resize(COUNT);
			for (uint8_t i = 0; i < COUNT; i++) {
				EXTRA_INTS[i] = *(uint32_t*)(buffer + offset); if (bBigEndian) LITTLE_BIG_SWAP(EXTRA_INTS[i]); offset += 4;
			}
		}
		
	}
};

template <bool bBigEndian>
struct S_G1T_TEX_HEADER
{
	uint8_t mipSys = 0;
	S_GT1_LOAD_TYPE KTGL_TEXTURE_TYPE = S_GT1_LOAD_TYPE::PLANAR;
	uint8_t MIP_COUNT = 0;

	uint8_t KTGL_PIXEL_FORMAT = 0;

	uint8_t dxdy = 0;
	uint8_t PACKED_WIDTH = 0;
	uint8_t PACKED_HEIGHT = 0;
	uint32_t HEIGHT = 0, WIDTH = 0;

	uint8_t depthEx = 0;
	uint8_t PACKED_DEPTH = 0;
	uint8_t READ_G1T_HEADER_EX = 0;
	
	uint8_t address1_2 = 0;
	uint8_t KTGL_GD_TEXADDRESS1 = 0;
	uint8_t KTGL_GD_TEXADDRESS2 = 0;

	uint8_t address3_texFilter1 = 0;
	uint8_t KTGL_GD_TEXADDRESS3 = 0;
	uint8_t KTGL_GD_TEXFILTER1 = 0;

	uint8_t texFilter2_mipFilter1 = 0;
	uint8_t KTGL_GD_TEXFILTER2 = 0;
	uint8_t KTGL_GD_MIPFILTER1 = 0;

	uint8_t mipFilter2_hasEx = 0;
	uint8_t KTGL_GD_MIPFILTER2 = 0;
	uint8_t HAS_TEX_EX_HEADER = 0;

	uint32_t headerSize = 0x8;

	// S_G1T_TEX_HEADER_EX values

	uint32_t TEX_HEADER_EX_SIZE = 0;

	float Z_SCALE = 0;

	uint16_t exFacesArrays = 0;
	uint16_t EX_FACES = 0;
	uint16_t EX_ARRAY_COUNT = 0;

	EX_SWIZZLE_TYPE EX_SWIZZLE = EX_SWIZZLE_TYPE::NONE;
	KTGL_GD_COLOR_SPACE_TYPE KTGL_GD_COLOR_SPACE = KTGL_GD_COLOR_SPACE_TYPE::NONE; // mostly used as srgb from linear

	uint32_t EX_WIDTH = 0;
	uint32_t EX_HEIGHT = 0;

	bool bPanic = false;

	S_G1T_TEX_HEADER(BYTE* buffer, uint32_t& offset, int bufferLen, int i)
	{
		// panic check
		if ((offset + headerSize) > (uint32_t)bufferLen)
		{
			bPanic = true;
			return;
		}

		//Read basic info
		mipSys                = *(uint8_t*)(buffer + offset); offset += 1;
		//split mip_count and subsys
		if (bBigEndian) 
		{
			KTGL_TEXTURE_TYPE = (S_GT1_LOAD_TYPE)((mipSys >> 4) & 0xF);
			MIP_COUNT = ((mipSys >> 0) & 0xF);
		}
		else
		{
			KTGL_TEXTURE_TYPE = (S_GT1_LOAD_TYPE)((mipSys >> 0) & 0xF);
			MIP_COUNT = ((mipSys >> 4) & 0xF);
		}

		KTGL_PIXEL_FORMAT = *(uint8_t*)(buffer + offset); offset += 1;

		// panic check
		if (KTGL_PIXEL_FORMAT > 0x7E)
		{
			bPanic = true;
		}

		dxdy                  = *(uint8_t*)(buffer + offset); offset += 1;
		//split width and height
		PACKED_WIDTH =  ((dxdy >> 0) & 0xF);
		PACKED_HEIGHT = ((dxdy >> 4) & 0xF);
		//compute width and height
		WIDTH = (uint32_t)pow(2, PACKED_WIDTH);
		HEIGHT = (uint32_t)pow(2, PACKED_HEIGHT);

		if (bBigEndian) {
			uint32_t temp = WIDTH;
			WIDTH = HEIGHT;
			HEIGHT = temp;
			swapNibbles(&PACKED_WIDTH, &PACKED_HEIGHT);
		}
		
		depthEx               = *(uint8_t*)(buffer + offset); offset += 1;
		//split depth and read ex header
		PACKED_DEPTH =       ((depthEx >> 0) & 0xF);
		READ_G1T_HEADER_EX = ((depthEx >> 4) & 0xF);
		if (bBigEndian) swapNibbles(&PACKED_DEPTH, &READ_G1T_HEADER_EX);

		address1_2            = *(uint8_t*)(buffer + offset); offset += 1;
		//split texture address
		KTGL_GD_TEXADDRESS1 = ((address1_2 >> 0) & 0xF);
		KTGL_GD_TEXADDRESS2 = ((address1_2 >> 4) & 0xF);
		if (bBigEndian) swapNibbles(&KTGL_GD_TEXADDRESS1, &KTGL_GD_TEXADDRESS2);

		address3_texFilter1  = *(uint8_t*)(buffer + offset); offset += 1;
		//split texture address and texture filter
		KTGL_GD_TEXADDRESS3 = ((address3_texFilter1 >> 0) & 0xF);
		KTGL_GD_TEXFILTER1  = ((address3_texFilter1 >> 4) & 0xF);
		if (bBigEndian) swapNibbles(&KTGL_GD_TEXADDRESS3, &KTGL_GD_TEXFILTER1);

		texFilter2_mipFilter1 = *(uint8_t*)(buffer + offset); offset += 1;
		//split texture filter and mip filter
		KTGL_GD_TEXFILTER2 = ((texFilter2_mipFilter1 >> 0) & 0xF);
		KTGL_GD_MIPFILTER1 = ((texFilter2_mipFilter1 >> 4) & 0xF);
		if (bBigEndian) swapNibbles(&KTGL_GD_TEXFILTER2, &KTGL_GD_MIPFILTER1);

		mipFilter2_hasEx   = *(uint8_t*)(buffer + offset); offset += 1;
		//split mip filter and read tex ex header
		KTGL_GD_MIPFILTER2 = ((mipFilter2_hasEx >> 0) & 0xF);
		HAS_TEX_EX_HEADER  = ((mipFilter2_hasEx >> 4) & 0xF);
		if (bBigEndian) swapNibbles(&KTGL_GD_MIPFILTER2, &HAS_TEX_EX_HEADER);

		//Extra header
		if (HAS_TEX_EX_HEADER > 0)
		{
			TEX_HEADER_EX_SIZE = *(uint32_t*)(buffer + offset); if (bBigEndian) LITTLE_BIG_SWAP(TEX_HEADER_EX_SIZE); offset += 4;
			headerSize += TEX_HEADER_EX_SIZE;

			// panic check
			if ((offset + TEX_HEADER_EX_SIZE) > uint32_t(bufferLen))
			{
				bPanic = true;
				return;
			}

			Z_SCALE      = *(float*)(buffer + offset); if (bBigEndian) LITTLE_BIG_SWAP(Z_SCALE); offset += 4;

			exFacesArrays    = *(uint16_t*)(buffer + offset); if (bBigEndian) LITTLE_BIG_SWAP(exFacesArrays); offset += 2;
			//split faces and array count
			EX_FACES		= ((exFacesArrays >> 0) & 0xF);
			EX_ARRAY_COUNT  = ((exFacesArrays >> 4) & 0xFFF);

			EX_SWIZZLE          = *(EX_SWIZZLE_TYPE*)(buffer + offset); offset += 1;
			KTGL_GD_COLOR_SPACE = *(KTGL_GD_COLOR_SPACE_TYPE*)(buffer + offset); offset += 1;
			
			if (TEX_HEADER_EX_SIZE >= 0x10)
			{
				EX_WIDTH = *(uint32_t*)(buffer + offset); if (bBigEndian) LITTLE_BIG_SWAP(EX_WIDTH); offset += 4;
				if (EX_WIDTH != 0) WIDTH = EX_WIDTH;
			}
			if (TEX_HEADER_EX_SIZE >= 0x14)
			{
				EX_HEIGHT = *(uint32_t*)(buffer + offset); if (bBigEndian) LITTLE_BIG_SWAP(EX_HEIGHT); offset += 4;
				if (EX_HEIGHT != 0) HEIGHT = EX_HEIGHT;
			}

			if ( TEX_HEADER_EX_SIZE != 0xC && 
				 TEX_HEADER_EX_SIZE != 0x14 &&
				bDebugLog)
			{
				LogDebug("TEX_HEADER_EX size error on #%d - TEX_HEADER_EX_SIZE: %d\n", i, TEX_HEADER_EX_SIZE);
			}
		}
	}
};

///////////////////////////////
//      MAIN FUNCTION        //
///////////////////////////////

template <bool bBigEndian>
struct G1TG_TEXTURE
{

	std::vector<uint32_t> offsetList;
	std::vector<S_G1T_TEX_ATTR_HEADER<bBigEndian>> AttrList;
	std::vector<S_G1T_HEADER_EX<bBigEndian>> ExList;

	G1TG_TEXTURE(BYTE* buffer, int bufferLen, CArrayList<noesisTex_t*>& noeTex, noeRAPI_t* rapi)
	{

		///////////////////////////////
		//   READ BASIC HEADER INFO  //
		///////////////////////////////

		// Panic check on header size
		if (bufferLen < 20)
		{
			g_nfn->NPAPI_PopupDebugLog(0);
			LogDebug("G1T file too small!\nRestart app with 'Enable debug log' on in 'Tools/Project G1M' menu for info.\n");
			assert(0 && "G1T too small to be a file!\n");
		}

		// Read basic header
		uint32_t offset = 0;
		G1T_HEADER<bBigEndian> header = G1T_HEADER<bBigEndian>(buffer, offset, rapi, bufferLen);

		// Panic check on header read
		if (header.bPanic)
		{
			g_nfn->NPAPI_PopupDebugLog(0);
			LogDebug("G1T file header read error!\nRestart app with 'Enable debug log' on in 'Tools/Project G1M' menu for info.\n");
			assert(0 && "G1T file header read error!\n");
		}

		// Read ATTR header
		for (uint32_t i = 0; i < header.TEX_COUNT; i++)
		{
			AttrList.push_back(S_G1T_TEX_ATTR_HEADER<bBigEndian>(buffer, offset, bufferLen));
			// Panic check on header
			if (AttrList[i].bPanic)
			{
				g_nfn->NPAPI_PopupDebugLog(0);
				LogDebug("G1T_TEX_ATTR_HEADER read error!\nRestart app with 'Enable debug log' on in 'Tools/Project G1M' menu for info.\n");
				assert(0 && "G1T_TEX_ATTR_HEADER read error!\n");
				break;
			}
		}

		// SKIP_TABLE offsets (data size is now computed and check as well now)
		offsetList.resize(header.TEX_COUNT);
		memcpy(offsetList.data(), buffer + offset, 4 * header.TEX_COUNT); offset += (4 * header.TEX_COUNT);
		for (auto& offs : offsetList)
		{
			if (bBigEndian)
			{
				LITTLE_BIG_SWAP(offs);
			}
			// panic check on size
			if ((offs + header.TEX_OFFSET) > (uint32_t)bufferLen)
			{
				g_nfn->NPAPI_PopupDebugLog(0);
				LogDebug("G1T_SKIP_TABLE size read error!\nRestart app with 'Enable debug log' on in 'Tools/Project G1M' menu for info.\n");
				assert(0 && "G1T_SKIP_TABLE size read error!\n");
			}
		}

		// panic check on size
		if (offset > (uint32_t)bufferLen)
		{
			g_nfn->NPAPI_PopupDebugLog(0);
			LogDebug("G1T_SKIP_TABLE size read error!\nRestart app with 'Enable debug log' on in 'Tools/Project G1M' menu for info.\n");
			assert(0 && "G1T_SKIP_TABLE size longer than buffer!\n");
		}

		// Read EX headers
		if (header.HEADER_EX_SIZE != 0)
		{
			for (uint32_t i = 0; i < header.TEX_COUNT; i++)
			{
				ExList.push_back(S_G1T_HEADER_EX<bBigEndian>(buffer, offset, bufferLen));
				// check check
				if (ExList[i].bPanic)
				{
					g_nfn->NPAPI_PopupDebugLog(0);
					LogDebug("G1T_HEADER_EX read error!\nRestart app with 'Enable debug log' on in 'Tools/Project G1M' menu for info.\n");
					assert(0 && "G1T_HEADER_EX read error!\n");
				}
			}
		}

		///////////////////////////////
		//      SET UP OPERATORS     //
		///////////////////////////////
		
		// PS4 swizzle
		typedef void(*NoesisMisc_Untile1dThin_p)(uint8_t* pDest, const uint32_t destSize, const uint8_t* pSrc, const uint32_t srcSize, const uint32_t w, const uint32_t h, const uint32_t bitsPerTexel, bool isBC, noeRAPI_t* pRapi);
		// uint8_t* pDest, const uint32_t destSize, const uint8_t* pSrc, const uint32_t srcSize, const uint32_t w, const uint32_t h, const uint32_t bitsPerTexel, bool isBC, noeRAPI_t* pRapi
		NoesisMisc_Untile1dThin_p NoesisMisc_Untile1dThin = NULL;
		NoesisMisc_Untile1dThin = (NoesisMisc_Untile1dThin_p)g_nfn->NPAPI_GetUserExtProc("NoesisMisc_Untile1dThin");

		///////////////////////////////
		//      PROCESS TEXTURES     //
		///////////////////////////////

		for (uint32_t i = 0; i < header.TEX_COUNT; i++)
		{
			///////////////////////////////
			//       CHECK OFFSETS       //
			///////////////////////////////

			// This list is updated for alpha atlas textures as this table is incorrect for them
			uint32_t offs = offsetList[i]; 
			offset = header.TEX_OFFSET + offs;

			// panic check
			if (offset > (uint32_t)bufferLen)
			{
				g_nfn->NPAPI_PopupDebugLog(0);
				LogDebug("G1T_SKIP_TABLE location read error!\nRestart app with 'Enable debug log' on in 'Tools/Project G1M' menu for info.\n");
				assert(0 && "G1T_SKIP_TABLE outside of file size!\n");
			}

			///////////////////////////////
			//      READ TEX HEADERS     //
			///////////////////////////////

			S_G1T_TEX_HEADER<bBigEndian> texHeader = S_G1T_TEX_HEADER<bBigEndian>(buffer, offset, bufferLen, i);

			// panic check
			if (texHeader.bPanic)
			{
				g_nfn->NPAPI_PopupDebugLog(0);
				LogDebug("G1T_TEX_HEADER read error on texture index #%d!\nRestart app with 'Enable debug log' on in 'Tools/Project G1M' menu for info.\n", i);
				assert(0 && "G1T_TEX_HEADER read error!\n");
			}

			///////////////////////////////
			//     COMPUTE METADATA      //
			///////////////////////////////

			uint32_t WIDTH = max(texHeader.WIDTH, 1);

			uint32_t HEIGHT = max(texHeader.HEIGHT, 1);

			uint32_t MIPS = max(texHeader.MIP_COUNT, 1);

			// Find depth
			uint32_t DEPTH = max((uint32_t)pow(2, texHeader.PACKED_DEPTH), 1);

			// Not sure why this is a thing but I need it for size
			if (header.HEADER_EX_SIZE != 0 &&
				ExList[i].TYPE == TEX_ATTR_TYPE::PS4_PLANE_ARRAY &&
				texHeader.KTGL_TEXTURE_TYPE == S_GT1_LOAD_TYPE::PLANE_ARRAY
				)
			{
				DEPTH = ExList[i].ARRAY_DEPTH;
			}

			// Find faces
			uint32_t FACES = 1;

			if (texHeader.HAS_TEX_EX_HEADER != 0 &&
				texHeader.EX_FACES != 0)
			{
				FACES = texHeader.EX_FACES;
			}

			// Find plane count
			uint32_t PLANE_COUNT = 1;

			if (texHeader.KTGL_TEXTURE_TYPE == S_GT1_LOAD_TYPE::PLANAR)
			{
				PLANE_COUNT = 1;
			}
			else if (texHeader.KTGL_TEXTURE_TYPE == S_GT1_LOAD_TYPE::PLANE_ARRAY)
			{
				if (texHeader.HAS_TEX_EX_HEADER &&
					texHeader.EX_ARRAY_COUNT != 0)
				{
					PLANE_COUNT = texHeader.EX_ARRAY_COUNT;
				}
			}
			else if (texHeader.KTGL_TEXTURE_TYPE == S_GT1_LOAD_TYPE::VOLUME)
			{
				// sometimes hardcoded to 32
				if (texHeader.HAS_TEX_EX_HEADER &&
					texHeader.EX_ARRAY_COUNT != 0)
				{
					PLANE_COUNT = (uint32_t)pow(2, texHeader.EX_ARRAY_COUNT);
				}
				else
				{
					PLANE_COUNT = 32;
				}
			}
			else if (texHeader.KTGL_TEXTURE_TYPE == S_GT1_LOAD_TYPE::CUBE ||
					 texHeader.KTGL_TEXTURE_TYPE == S_GT1_LOAD_TYPE::CUBE_ARRAY)
			{

				if (texHeader.HAS_TEX_EX_HEADER &&
					texHeader.EX_ARRAY_COUNT != 0
				)
				{
					PLANE_COUNT = 6 * texHeader.EX_ARRAY_COUNT;
				}
				else if (PLANE_COUNT < 6)
				{
					PLANE_COUNT = 6;
				}
			}

			// Size of the first mip image data
			uint32_t firstMipSize = 0;
			// full size of all the image data including mips
			uint32_t totalTexBufferLen = 0;
			// texture size in bits per pixel
			double bitsPerPixel = 0;
			// smallest byte amount for a single pixel (includes compressed block formats)
			uint32_t minBytes = 0;

			///////////////////////////////
			//       SETUP FLAGS         //
			///////////////////////////////

			bool bNormalMap = false;

			if (AttrList[i].TYPE == TEX_ATTR_TYPE::NORMAL_MAP)
			{
				bNormalMap = true;
			}

			// Normals can also be found here
			if (header.HEADER_EX_SIZE != 0 &&
				ExList[i].TYPE == TEX_ATTR_TYPE::NORMAL_MAP)
			{
				bNormalMap = true;
			}

			// No idea why
			bool bNormalWithAlpha = false;

			if (AttrList[i].TYPE == TEX_ATTR_TYPE::NORMAL_WITH_ALPHA)
			{
				bNormalWithAlpha = true;
			}

			// Normals with alpha can also be found here
			if (header.HEADER_EX_SIZE != 0 &&
				ExList[i].TYPE == TEX_ATTR_TYPE::NORMAL_WITH_ALPHA)
			{
				bNormalWithAlpha = true;
			}

			// for hardcoded swizzled textures
			bool bSwizzled = false;

			if (AttrList[i].TYPE == TEX_ATTR_TYPE::N3DS_Wii_SWIZZLE ||
				AttrList[i].TYPE == TEX_ATTR_TYPE::WiiU_SWIZZLE ||
				AttrList[i].TYPE == TEX_ATTR_TYPE::PSVITA_PS5_SWIZZLE)
			{
				bSwizzled = true;
			}

			// Swizzle flags can also be found here
			if (header.HEADER_EX_SIZE != 0 &&
				(ExList[i].TYPE == TEX_ATTR_TYPE::N3DS_Wii_SWIZZLE ||
				 ExList[i].TYPE == TEX_ATTR_TYPE::WiiU_SWIZZLE ||
				 ExList[i].TYPE == TEX_ATTR_TYPE::PSVITA_PS5_SWIZZLE))
			{
				bSwizzled = true;
			}

			if (texHeader.EX_SWIZZLE != EX_SWIZZLE_TYPE::NONE)
			{
				bSwizzled = true;
			}

			// for formats that are not unsigned
			bool bSigned = false;
			// for blocks format 
			bool bCompressedFormat = false;
			bool bNormalized = true;
			bool bIsETC1 = false;
			bool bPICAETC = false;
			bool bIsETC2 = false;
			// For images that have a hidden alpha channel texture below (double height)
			// Info here: https://arm-software.github.io/opengl-es-sdk-for-android/compressed_alpha_channels.html
			bool bHasAlphaAtlas = false;
			// 3DS ETC Alpha
			bool b3DSAlpha = false;
			bool bIsPVRTC = false;
			bool bNeedsX360EndianSwap = false;
			// TODO: handle P4 images when we find the palettes
			bool bPalette4 = false;
			// TODO: handle P8 images when we find the palettes
			bool bPalette8 = false;
			// TODO: handle Nintendor Wii P14 images when we find the palettes
			bool bPalette14 = false;
			bool bFloat = false;
			bool bHalfFloat = false;
			// For a2b#F10g#F10r#F10 to r#F32g#F32b#F32a#F32
			bool bConvert10BitFloat = false;
			// For r#F11g#F11b#F10 to r#F32g#F32b#F32
			bool bConvert11Bit10BitFloat = false;
			// for textures that are depth and stencil
			bool bIsDepth = false;
			// unsigned normalized
			bool bIsUNorm = false;
			// for ASTC sub format
			bool bIsASTC = false;
			// for d24s8 textures to r24b24g24a8
			bool bD24_8Convert = false;
			// for d16 to r16b16g16
			bool bD16Convert = false;
			// for d32 to r#F32B#F32g#F32
			bool bD32Convert = false;
			// for d#F32p24s8 to r#F32b#F32g#F32a#F32
			bool bD32FloatConvert = false;
			// for BC1-7 & DXT
			int fourccFormat = -1;
			// for compressed block sizes
			int blockWidth = 1, blockHeight = 1;

			// raw format bit count per channels, uses rgbampc. Normally rgba but has extras: m is for magnitude, p for padding, c is for color index,
			// d for depth (covers single value to all rgb channels) and l for lumiance (just r), s for stencil (that converts to alpha) 
			std::string rawFormat = "";

			///////////////////////////////
			//    TEXTURE INDEX SWITCH   //
			///////////////////////////////

			switch (texHeader.KTGL_PIXEL_FORMAT)
			{
			case 0x00: // GL_RGBA GL_RGBA GL_UNSIGNED_BYTE
				rawFormat = "r8g8b8a8";
				bitsPerPixel = 0x20;
				minBytes = 0x04;
				bIsUNorm = true;
				break;
			case 0x01: // GL_BGRA GL_BGRA GL_UNSIGNED_BYTE
				rawFormat = "b8g8r8a8";
				bitsPerPixel = 0x20;
				minBytes = 0x04;
				bIsUNorm = true;
				break;
			case 0x02: // GL_R32F GL_RED GL_FLOAT
				rawFormat = "r#F32"; // should be working
				bitsPerPixel = 0x20;
				minBytes = 0x04;
				bFloat = true;
				break;
			case 0x03: // GL_RGBA16F GL_RGBA GL_HALF_FLOAT_OES
				rawFormat = "r#F16g#F16b#F16a#F16"; // should be working
				bitsPerPixel = 0x40;
				minBytes = 0x08;
				bHalfFloat = true;
				break;
			case 0x04: // GL_RGBA32F GL_RGBA GL_FLOAT
				rawFormat = "r#F32g#F32b#F32a#F32"; // should be working
				bitsPerPixel = 0x80;
				minBytes = 0x10;
				bFloat = true;
				break;
			case 0x05: // GL_DEPTH_STENCIL GL_DEPTH_STENCIL GL_UNSIGNED_INT_24_8
				rawFormat = "d24s8"; // convert to r24g24b24a8
				bitsPerPixel = 0x20;
				minBytes = 0x04;
				bIsDepth = true;
				bD24_8Convert = true;
				bIsUNorm = true;
				break;
			case 0x06: // GL_COMPRESSED_RGBA_S3TC_DXT1_EXT GL_RGBA GL_UNSIGNED_BYTE BC1
				fourccFormat = FOURCC_DXT1; // convert to r8g8b8a8
				bitsPerPixel = 0x04;
				minBytes = 0x08;
				bCompressedFormat = true;
				bNeedsX360EndianSwap = true;
				blockWidth = 4;
				blockHeight = 4;
				bIsUNorm = true;
				break;
			case 0x07: // GL_COMPRESSED_RGBA_S3TC_DXT3_EXT GL_RGBA GL_UNSIGNED_BYTE BC2
				fourccFormat = FOURCC_DXT3; // convert to r8g8b8a8
				bitsPerPixel = 0x08;
				minBytes = 0x10;
				bCompressedFormat = true;
				blockWidth = 4;
				blockHeight = 4;
				bIsUNorm = true;
				break;
			case 0x08: // GL_COMPRESSED_RGBA_S3TC_DXT5_EXT GL_RGBA GL_UNSIGNED_BYTE BC3
				fourccFormat = FOURCC_DXT5; // convert to r8g8b8a8
				bitsPerPixel = 0x08;
				minBytes = 0x10;
				bCompressedFormat = true;
				blockWidth = 4;
				blockHeight = 4;
				bIsUNorm = true;
				break;
			case 0x09: // DXGI_FORMAT_R8G8B8A8_TYPELESS DXGI_FORMAT_R8G8B8A8_UNORM DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
				rawFormat = "r8g8b8a8";
				bitsPerPixel = 0x20;
				minBytes = 0x04;
				bIsUNorm = true;
				bSwizzled = true;
				break;
			case 0x0A: // DXGI_FORMAT_B8G8R8A8_TYPELESS DXGI_FORMAT_B8G8R8A8_UNORM DXGI_FORMAT_B8G8R8A8_UNORM_SRGB
				rawFormat = "b8g8r8a8";
				bitsPerPixel = 0x20;
				minBytes = 0x04;
				bIsUNorm = true;
				bSwizzled = true;
				break;
			case 0x0B: // DXGI_FORMAT_R32_TYPELESS DXGI_FORMAT_R32_FLOAT DXGI_FORMAT_R32_FLOAT
				rawFormat = "r#F32"; // should be working
				bitsPerPixel = 0x20;
				minBytes = 0x04;
				bFloat = true;
				break;
			case 0x0C: // DXGI_FORMAT_R16G16B16A16_TYPELESS DXGI_FORMAT_R16G16B16A16_FLOAT DXGI_FORMAT_R16G16B16A16_FLOAT
				rawFormat = "r#F16g#F16b#F16a#F16"; // should be working
				bitsPerPixel = 0x40;
				minBytes = 0x08;
				bHalfFloat = true;
				break;
			case 0x0D: // DXGI_FORMAT_R32G32B32A32_TYPELESS DXGI_FORMAT_R32G32B32A32_FLOAT DXGI_FORMAT_R32G32B32A32_FLOAT 
				rawFormat = "r#F32g#F32b#F32a#F32"; // should be working
				bitsPerPixel = 0x80;
				minBytes = 0x10;
				bFloat = true;
				break;
			case 0x0E: // SCE_GXM_TEXTURE_FORMAT_U2F10F10F10 SWIZZLE4_ABGR
				rawFormat = "a2b#F10g#F10r#F10"; // converts to r#F32g#F32b#F32a#F32
				bitsPerPixel = 0x20;
				minBytes = 0x04;
				bConvert10BitFloat = true;
				break;
			case 0x0F: // GL_ALPHA GL_ALPHA GL_UNSIGNED_BYTE
				rawFormat = "a8";
				bitsPerPixel = 0x08;
				minBytes = 0x01;
				bIsUNorm = true;
				break;
			case 0x10: // DXGI_FORMAT_BC1_TYPELESS DXGI_FORMAT_BC1_UNORM DXGI_FORMAT_BC1_UNORM_SRGB DXT1 (PSV Swizzle & sometimes flip)
				fourccFormat = FOURCC_DXT1; // convert to r8g8b8a8
				bitsPerPixel = 0x04; // mortonWidth
				minBytes = 0x08;
				bCompressedFormat = true;
				blockWidth = 4;
				blockHeight = 4;
				bIsUNorm = true;
				bSwizzled = true; 
				break;
			case 0x11: // DXGI_FORMAT_BC2_TYPELESS DXGI_FORMAT_BC2_UNORM DXGI_FORMAT_BC2_UNORM_SRGB DXT2 / DXT3 (PSV Swizzle & sometimes flip)
				fourccFormat = FOURCC_DXT3; // convert to r8g8b8a8
				bitsPerPixel = 0x08;
				minBytes = 0x10;
				bCompressedFormat = true;
				bIsUNorm = true;
				bSwizzled = true;
				break;
			case 0x12: // DXGI_FORMAT_BC3_TYPELESS DXGI_FORMAT_BC3_UNORM DXGI_FORMAT_BC3_UNORM_SRGB DXT4 / DXT5 (PSV Swizzle & sometimes flip)
				fourccFormat = FOURCC_DXT5; // convert to r8g8b8a8
				bitsPerPixel = 0x08; // mortonWidth
				minBytes = 0x10;
				bCompressedFormat = true;
				blockWidth = 4;
				blockHeight = 4;
				bIsUNorm = true;
				bSwizzled = true;
				break;
			case 0x13: // DXGI_FORMAT_R24G8_TYPELESS DXGI_FORMAT_R24_UNORM_X8_TYPELESS DXGI_FORMAT_D24_UNORM_S8_UINT
				rawFormat = "d24s8"; // convert to r24g24b24a8
				bitsPerPixel = 0x20;
				minBytes = 0x04;
				bIsDepth = true;
				bD24_8Convert = true;
				bIsUNorm = true;
				break;
			case 0x14: // GL_DEPTH_COMPONENT16 GL_DEPTH_COMPONENT GL_UNSIGNED_SHORT
				rawFormat = "d16"; // convert to r16g16b16
				bitsPerPixel = 0x10;
				minBytes = 0x02;
				bIsDepth = true;
				bD16Convert = true;
				break;
			case 0x15: // DXGI_FORMAT_R16_TYPELESS DXGI_FORMAT_R16_UNORM DXGI_FORMAT_D16_UNORM
				rawFormat = "d16";
				bitsPerPixel = 0x10;
				minBytes = 0x02;
				bIsUNorm = true;
				break;
			case 0x16: // GL_R16 GL_RED GL_UNSIGNED_SHORT
				rawFormat = "r16";
				bitsPerPixel = 0x10;
				minBytes = 0x02;
				bIsUNorm = true;
				break;
			case 0x17: // DXGI_FORMAT_R16_TYPELESS DXGI_FORMAT_R16_UNORM DXGI_FORMAT_R16_UNORM
				rawFormat = "r16";
				bitsPerPixel = 0x10;
				minBytes = 0x02;
				bIsUNorm = true;
				break;
			case 0x18: // DXGI_FORMAT_A8_UNORM DXGI_FORMAT_A8_UNORM DXGI_FORMAT_A8_UNORM
				rawFormat = "a8";
				bitsPerPixel = 0x08;
				minBytes = 0x01;
				bIsUNorm = true;
				break;
			case 0x19: // DXGI_FORMAT_B5G6R5_UNORM DXGI_FORMAT_B5G6R5_UNORM DXGI_FORMAT_B5G6R5_UNORM
				rawFormat = "b5g6r5";
				bitsPerPixel = 0x10;
				minBytes = 0x02;
				bIsUNorm = true;
				break;
			case 0x1A: // DXGI_FORMAT_B5G5R5A1_UNORM DXGI_FORMAT_B5G5R5A1_UNORM DXGI_FORMAT_B5G5R5A1_UNORM
				rawFormat = "b5g5r5a1";
				bitsPerPixel = 0x10;
				minBytes = 0x02;
				bIsUNorm = true;
				break;
			case 0x1B: // SCE_GXM_TEXTURE_FORMAT_U4U4U4U4_SWIZZLE4_ARGB
				rawFormat = "b4g4r4a4";
				bitsPerPixel = 0x10;
				minBytes = 0x02;
				bIsUNorm = true;
				break;
			case 0x1C: // DXGI_FORMAT_B5G6R5_UNORM DXGI_FORMAT_B5G6R5_UNORM DXGI_FORMAT_B5G6R5_UNORM
				rawFormat = "b5g6r5";
				bitsPerPixel = 0x10;
				minBytes = 0x02;
				bIsUNorm = true;
				break;
			case 0x1D: // DXGI_FORMAT_B5G5R5A1_UNORM DXGI_FORMAT_B5G5R5A1_UNORM DXGI_FORMAT_B5G5R5A1_UNORM
				rawFormat = "b5g5r5a1";
				bitsPerPixel = 0x10;
				minBytes = 0x02;
				bIsUNorm = true;
				break;
			case 0x1E: // SCE_GXM_TEXTURE_FORMAT_U4U4U4U4_SWIZZLE4_ARGB
				rawFormat = "b4g4r4a4";
				bitsPerPixel = 0x10;
				minBytes = 0x02;
				bIsUNorm = true;
				break;
			case 0x1F: // GL_R32F GL_RED GL_FLOAT
				rawFormat = "r#F32";
				bitsPerPixel = 0x20;
				minBytes = 0x04;
				bFloat = true;
				break;
			case 0x20: // DXGI_FORMAT_R32_TYPELESS DXGI_FORMAT_R32_FLOAT DXGI_FORMAT_R32_FLOAT
				rawFormat = "r#F32";
				bitsPerPixel = 0x20;
				minBytes = 0x04;
				bFloat = true;
				break;
			case 0x21: // GL_BGRA GL_BGRA GL_UNSIGNED_BYTE
				rawFormat = "b8g8r8a8";
				bitsPerPixel = 0x20;
				minBytes = 0x04;
				bIsUNorm = true;
				break;
			case 0x22: // DXGI_FORMAT_B8G8R8X8_TYPELESS DXGI_FORMAT_B8G8R8X8_UNORM DXGI_FORMAT_B8G8R8X8_UNORM_SRGB
				rawFormat = "b8g8r8p8";
				bitsPerPixel = 0x20;
				minBytes = 0x04;
				bIsUNorm = true;
				break;
			case 0x23: // DXGI_FORMAT_R16G16_TYPELESS DXGI_FORMAT_R16G16_UNORM DXGI_FORMAT_R16G16_UNORM 
				rawFormat = "r16g16";
				bitsPerPixel = 0x20;
				minBytes = 0x04;
				bIsUNorm = true;
				break;
			case 0x24: // DXGI_FORMAT_R16G16_TYPELESS DXGI_FORMAT_R16G16_UNORM DXGI_FORMAT_R16G16_UNORM (same as above but had a 0 bpp error)
				rawFormat = "g16r16";
				bitsPerPixel = 0x20;
				minBytes = 0x04;
				bIsUNorm = true;
				break;
			case 0x25: // GX_TF_RGB5A3 GX_TF_RGB5A3 GX_TF_RGB5A3 (Wii)
				rawFormat = "r5g5b5a3"; // also seens as BC6H
				bitsPerPixel = 0x10;
				minBytes = 0x02;
				break;
			case 0x26: // SEC_G8R8 SEC_G8R8 SEC_UNORM
				rawFormat = "g8r8";
				bitsPerPixel = 0x10;
				minBytes = 0x02;
				bIsUNorm = true;
				break;
			case 0x27: // SCE_GXM_TEXTURE_FORMAT_U8U8_SWIZZLE2_GRRR (g8r8)
				rawFormat = "g8r8";
				bitsPerPixel = 0x10;
				minBytes = 0x02;
				bIsUNorm = true;
				break;
			case 0x28: // SEC_R8 SEC_R8 SEC_UNORM 
				rawFormat = "b8"; // XXRX = Red -> Blue
				bitsPerPixel = 0x08;
				minBytes = 0x01;
				bIsUNorm = true;
				break;
			case 0x29: // SEC_R8 SEC_R8 SEC_UNORM (dupe of 0x28 so might have another thing going on)
				rawFormat = "g8"; // XRXX = Red -> Blue
				bitsPerPixel = 0x08;
				minBytes = 0x01;
				bIsUNorm = true;
				break;
			case 0x2A: // GL_R8 GL_RED GL_UNSIGNED_BYTE
				rawFormat = "r8"; // RXXX
				bitsPerPixel = 0x08;
				minBytes = 0x01;
				bIsUNorm = true;
				break;
			case 0x2B: // PICA_LUMINANCE PICA_LUMINANCE PICA_UNORM_4_HALF_BYTE
				rawFormat = "r4"; // just making this red as it's a weird format
				                  // channels can be seen as RRRX
				                  // also seen as BC6H
				bitsPerPixel = 0x04;
				minBytes = 0x01;
				bIsUNorm = true;
				break;
			case 0x2C: // PICA_LUMINANCE PICA_LUMINANCE GL_UNSIGNED_BYTE
				rawFormat = "r8";
				bitsPerPixel = 0x08;
				minBytes = 0x01;
				bIsUNorm = true;
				break;
			case 0x2D: // PICA_LUMINANCE_A PICA_LUMINANCE_A PICA_UNORM_44_BYTE
				rawFormat = "r4a4"; // just making this red as it's a weird format
				bitsPerPixel = 0x08;
				minBytes = 0x01;
				bIsUNorm = true;
				break;
			case 0x2E: // PICA_LUMINANCE_A PICA_LUMINANCE_A GL_UNSIGNED_BYTE
				rawFormat = "r8a8"; // just making this red as it's a weird format
				bitsPerPixel = 0x10;
				minBytes = 0x02;
				bIsUNorm = true;
				break;
			case 0x2F: // GX_TF_I4 GX_TF_I4 GX_TF_I4
				rawFormat = "r4";
				bitsPerPixel = 0x04;
				minBytes = 0x01;
				break;
			case 0x30: // SCE_GXM_TEXTURE_FORMAT_U8_SWIZZLE1_111R (r8)
				rawFormat = "r8"; // might be a special case depending on the system
				                  // NWii:   GX_TF_Z8
						          // X360:   R8
				                  // PSVita: SCE_GXM_TEXTURE_FORMAT_U8_SWIZZLE1_111R
				                  // PS4:    R8
								  // PS5:    A8
				bitsPerPixel = 0x08;
				minBytes = 0x01;
				bIsUNorm = true;
				break;
			case 0x31: // SCE_GXM_TEXTURE_FORMAT_P4_SWIZZLE4_ABGR (CI4)  (a8b8g8r8)
				rawFormat = "r4"; // Converts to r8g8b8a8
							      // NWii:   GX_TF_CI4
				                  // PSVita: SCE_GXM_TEXTURE_FORMAT_P4_SWIZZLE4_ABGR
				bitsPerPixel = 0x04;
				minBytes = 0x01;
				bPalette4 = true;
				bCompressedFormat = true; // not really but thats where this should be processed
				blockWidth = 8;
				blockHeight = 8;
				break;
			case 0x32: // SCE_GXM_TEXTURE_FORMAT_P8_SWIZZLE4_ABGR (CI8) (a8b8g8r8)
				rawFormat = "r8"; // Converts to r8g8b8a8
								  // NWii:   GX_TF_CI8
							      // PSVita: SCE_GXM_TEXTURE_FORMAT_P8_SWIZZLE4_ABGR
				bitsPerPixel = 0x08;
				minBytes = 0x01;
				bPalette8 = true;
				bCompressedFormat = true; // not really but thats where this should be processed
				blockWidth = 8;
				blockHeight = 4;
				break;
			case 0x33: // GX_TF_CI14 GX_TF_CI14 GX_TF_CI14
				rawFormat = "r14"; // Converts to r8g8b8a8
				bitsPerPixel = 0x10;
				minBytes = 0x02;
				bPalette14 = true;
				bCompressedFormat = true; // not really but thats where this should be processed
				blockWidth = 4;
				blockHeight = 4;
				break;
			case 0x34: // GL_RGB GL_RGB GL_UNSIGNED_SHORT_5_6_5     b5g6r5
				rawFormat = "b5g6r5";
				bitsPerPixel = 0x10;
				minBytes = 0x02;
				bIsUNorm = true;
				break;
			case 0x35: // GL_RGBA GL_RGBA GL_UNSIGNED_SHORT_5_5_5_1
				rawFormat = "a1b5g5r5"; // might be r5g5b5a1
				bitsPerPixel = 0x10;
				minBytes = 0x02;
				bIsUNorm = true;
				break;
			case 0x36: // GL_RGBA GL_RGBA GL_UNSIGNED_SHORT_4_4_4_4 a4b4g4r4
				rawFormat = "a4b4g4r4"; // might be r4g4b4a4
				bitsPerPixel = 0x10;
				minBytes = 0x02;
				bIsUNorm = true;
				break;
			case 0x37: // SCE_GXM_TEXTURE_FORMAT_P4_SWIZZLE4_ABGR 
				rawFormat = "r4";
				bitsPerPixel = 0x04;
				minBytes = 0x01;
				bPalette4 = true;
				bCompressedFormat = true; // not really but thats where this should be processed
				blockWidth = 8;
				blockHeight = 8;
				break;
			case 0x38: // GL_R8I GL_RED_INTEGER GL_BYTE
				rawFormat = "r8"; // Converts to r8g8b8a8
				                  // PSVita: SCE_GXM_TEXTURE_FORMAT_P8_SWIZZLE4_ABGR
				bitsPerPixel = 0x08;
				minBytes = 0x01;
				bPalette8 = true;
				bCompressedFormat = true; // not really but thats where this should be processed
				blockWidth = 8;
				blockHeight = 4;
				break;
			case 0x39: // GL_R16I GL_RED_INTEGER GL_SHORT
				rawFormat = "r16";
				bitsPerPixel = 0x10;
				minBytes = 0x02;
				bSigned = true;
				break;
			case 0x3A: // GL_R32I GL_RED_INTEGER GL_INT
				rawFormat = "r32";
				bitsPerPixel = 0x20;
				minBytes = 0x04;
				bSigned = true;
				break;
			case 0x3B: // BNTX_R5G6B5_UNORM BNTX_R5G6B5_UNORM, BNTX_R5G6B5_UNORM      (found in switch)
				rawFormat = "r5g6b5";
				bitsPerPixel = 0x10;
				minBytes = 0x02;
				bIsUNorm = true;
				break;
			case 0x3C: // BNTX_R5G5B5A1_UNORM BNTX_R5G5B5A1_UNORM BNTX_R5G5B5A1_UNORM (found in switch)
				rawFormat = "r5g5b5a1";
				bitsPerPixel = 0x10;
				minBytes = 0x02;
				bIsUNorm = true;
				break;
			case 0x3D: // BNTX_R4G4B4A4_UNORM BNTX_R4G4B4A4_UNORM BNTX_R4G4B4A4_UNORM (found in switch)
				rawFormat = "r4g4b4a4";
				bitsPerPixel = 0x10;
				minBytes = 0x02;
				bIsUNorm = true;
				break;
			case 0x3E: // GL_R16I GL_RED_INTEGER GL_SHORT (likely 0x39 dupe)
				rawFormat = "r16";
				bitsPerPixel = 0x10;
				minBytes = 0x02;
				bSigned = true;
				break;
			case 0x3F: // GL_R32I GL_RED_INTEGER GL_INT   (likely 0x3A dupe)
				rawFormat = "r32";
				bitsPerPixel = 0x20;
				minBytes = 0x04;
				bSigned = true;
				break;
			case 0x40: // GL_RGB10_A2 GL_RGBA GL_UNSIGNED_INT_2_10_10_10_REV
				rawFormat = "r10g10b10a2";
				bitsPerPixel = 0x20;
				minBytes = 0x02;
				bIsUNorm = true;
				break;
			case 0x41: // GL_RGBA16_EXT GL_RGBA GL_UNSIGNED_SHORT
				rawFormat = "r16g16b16a16";
				bitsPerPixel = 0x40;
				minBytes = 0x08;
				bIsUNorm = true;
				break;
			case 0x42: // DXGI_FORMAT_R10G10B10A2_UNORM DXGI_FORMAT_R10G10B10A2_UNORM DXGI_FORMAT_R10G10B10A2_UNORM
				rawFormat = "r10g10b10a2";
				bitsPerPixel = 0x20;
				minBytes = 0x02;
				bIsUNorm = true;
				break;
			case 0x43: // DXGI_FORMAT_R16G16B16A16_TYPELESS DXGI_FORMAT_R16G16B16A16_UNORM DXGI_FORMAT_R16G16B16A16_UNORM 
				rawFormat = "r16g16b16a16";
				bitsPerPixel = 0x40;
				minBytes = 0x08;
				bIsUNorm = true;
				break;
			case 0x44: // PICA_A PICA_A PICA_UNORM_4_HALF_BYTE (3DS)
				rawFormat = "r4";
				bitsPerPixel = 0x04;
				minBytes = 0x01;
				break;
			case 0x45: // GL_RGB GL_RGB GL_UNSIGNED_BYTE       (3DS) (PS Vita swizzle 32x32 tiles)
				rawFormat = "r8g8b8";
				bitsPerPixel = 0x18;
				minBytes = 0x03;
				break;
			case 0x46: // DXGI_FORMAT_R8G8_UNORM DXGI_FORMAT_R8G8_UNORM DXGI_FORMAT_R8G8_UNORM (3DS Swizzle)
				rawFormat = "r8g8";
				bitsPerPixel = 0x10;
				minBytes = 0x02;
				bIsUNorm = true;
				break;
			case 0x47: // PICA_ETC1_RGB8 PICA_ETC1_RGB8 GL_RGB 
				bPICAETC = true; // converts to r8g8b8a8
				bCompressedFormat = true;
				blockWidth = 4;
				blockHeight = 4;
				bIsETC1 = true;
				bitsPerPixel = 0x04;
				minBytes = 0x08;
				break;
			case 0x48: // PICA_ETC1_RGB8A4 PICA_ETC1_RGB8A4 GL_RGBA (has 2 block per to account for the alpha)
				bPICAETC = true; // converts to r8g8b8a8
				bCompressedFormat = true;
				blockWidth = 4;
				blockHeight = 4;
				bIsETC1 = true;
				bitsPerPixel = 0x04;
				minBytes = 0x10;
				b3DSAlpha = true;
				break;
			case 0x49: // GL_RGB GL_RGB GL_UNSIGNED_BYTE
				rawFormat = "r8g8b8";
				bitsPerPixel = 0x18;
				minBytes = 0x03;
				break;
			case 0x4A: // GL_DEPTH_COMPONENT GL_DEPTH_COMPONENT GL_UNSIGNED_INT (r8g8b8) (also 3DS)
				rawFormat = "r8g8b8";
				bitsPerPixel = 0x18;
				minBytes = 0x03;
				bIsDepth = true;
				break;
			case 0x4B: // GL_DEPTH_COMPONENT GL_DEPTH_COMPONENT GL_UNSIGNED_INT (r8g8b8) (like a dupe of above)
				rawFormat = "r8g8b8";
				bitsPerPixel = 0x18;
				minBytes = 0x03;
				bIsDepth = true;
				break;
			case 0x4C: // DXGI_FORMAT_R32G32_TYPELESS DXGI_FORMAT_R32G32_FLOAT DXGI_FORMAT_R32G32_FLOAT
				rawFormat = "r#F32g#F32";
				bitsPerPixel = 0x40;
				minBytes = 0x08;
				bFloat = true;
				break;
			case 0x4D: // DXGI_FORMAT_R32G32_TYPELESS DXGI_FORMAT_R32G32_FLOAT DXGI_FORMAT_R32G32_FLOAT
				rawFormat = "r#F32g#F32";
				bitsPerPixel = 0x40;
				minBytes = 0x08;
				bFloat = true;
				break;
			case 0x4E: // GL_DEPTH_COMPONENT32F GL_DEPTH_COMPONENT GL_FLOAT (R32F)
				rawFormat = "r#F32";
				bitsPerPixel = 0x20;
				minBytes = 0x04;
				bIsDepth = true;
				bFloat = true;
				break;
			case 0x4F: // DXGI_FORMAT_R32_TYPELESS DXGI_FORMAT_R32_FLOAT DXGI_FORMAT_D32_FLOAT
				rawFormat = "d#F32"; // converts to r#F32g#F32b#F32
				bitsPerPixel = 0x20;
				minBytes = 0x04;
				bIsDepth = true;
				bD32Convert = true;
				bFloat = true;
				break;
			case 0x50: // SCE_GXM_TEXTURE_FORMAT_PVRTII2BPP_SWIZZLE4_ABGR
				bIsPVRTC = true; // converts to r8g8b8a8
				bCompressedFormat = true;
				bitsPerPixel = 0x02;
				blockWidth = 8;
				blockHeight = 4;
				minBytes = 0x08;
				break;
			case 0x51: // SCE_GXM_TEXTURE_FORMAT_PVRTII2BPP_SWIZZLE4_ABGR
				bIsPVRTC = true; // converts to r8g8b8a8
				bCompressedFormat = true;
				blockWidth = 8;
				blockHeight = 4;
				bitsPerPixel = 0x02;
				minBytes = 0x08;
				break;
			case 0x52: // SCE_GXM_TEXTURE_FORMAT_PVRTII4BPP_SWIZZLE4_ABGR
				bIsPVRTC = true; // converts to r8g8b8a8
				bCompressedFormat = true;
				blockWidth = 4;
				blockHeight = 4;
				bitsPerPixel = 0x04;
				minBytes = 0x08;
				break;
			case 0x53: // SCE_GXM_TEXTURE_FORMAT_PVRTII4BPP_SWIZZLE4_ABGR 
				bIsPVRTC = true; // converts to r8g8b8a8
				bCompressedFormat = true;
				blockWidth = 4;
				blockHeight = 4;
				bitsPerPixel = 0x04;
				minBytes = 0x08;
				break;
			case 0x54: // GL_LUMINANCE GL_LUMINANCE GL_UNSIGNED_BYTE
				rawFormat = "l8";
				bitsPerPixel = 0x08;
				minBytes = 0x01;
				bIsUNorm = true;
				break;
			case 0x55: // GL_LUMINANCE_ALPHA GL_LUMINANCE_ALPHA GL_UNSIGNED_BYTE (also seen as G8R8)
				rawFormat = "l8a8";
				bitsPerPixel = 0x10;
				minBytes = 0x02;
				bIsUNorm = true;
				break;
			case 0x56: // GL_ETC1_RGB8_OES GL_RGB GL_UNSIGNED_BYTE
				bIsETC1 = true; // converts to r8g8b8a8
				bCompressedFormat = true;
				blockWidth = 4;
				blockHeight = 4;
				rawFormat = "RGB";
				if (texHeader.KTGL_GD_COLOR_SPACE == KTGL_GD_COLOR_SPACE_TYPE::sRGB_FROM_LINEAR)
				{
					rawFormat = "sRGB";
				}
				bitsPerPixel = 0x04;
				minBytes = 0x08;
				break;
			case 0x57: // GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG GL_RGBA GL_UNSIGNED_BYTE
				bIsPVRTC = true; // converts to r8g8b8a8
				bCompressedFormat = true;
				blockWidth = 8;
				blockHeight = 4;
				bitsPerPixel = 0x02;
				minBytes = 0x08;
				break;
			case 0x58: // GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG GL_RGBA GL_UNSIGNED_BYTE
				bIsPVRTC = true; // converts to r8g8b8a8
				bCompressedFormat = true;
				blockWidth = 4;
				blockHeight = 4;
				bitsPerPixel = 0x04;
				minBytes = 0x08;
				break;
			case 0x59: // GL_COMPRESSED_RGBA_S3TC_DXT1_EXT GL_RGBA GL_UNSIGNED_BYTE (BC1)
				fourccFormat = FOURCC_DXT1; // converts to r8g8b8a8
				bitsPerPixel = 0x04;
				minBytes = 0x08;
				bCompressedFormat = true;
				blockWidth = 4;
				blockHeight = 4;
				bIsUNorm = true;
				break;
			case 0x5A: // GL_COMPRESSED_RGBA_S3TC_DXT3_EXT GL_RGBA GL_UNSIGNED_BYTE (BC2)
				fourccFormat = FOURCC_DXT3; // converts to r8g8b8a8
				bitsPerPixel = 0x08;
				minBytes = 0x10;
				bCompressedFormat = true;
				blockWidth = 4;
				blockHeight = 4;
				bIsUNorm = true;
				break;
			case 0x5B: // GL_COMPRESSED_RGBA_S3TC_DXT5_EXT GL_RGBA GL_UNSIGNED_BYTE (BC3)
				fourccFormat = FOURCC_DXT5; // converts to r8g8b8a8
				bitsPerPixel = 0x08;
				minBytes = 0x10;
				bCompressedFormat = true;
				blockWidth = 4;
				blockHeight = 4;
				bIsUNorm = true;
				break;
			case 0x5C: // DXGI_FORMAT_BC4_TYPELESS DXGI_FORMAT_BC4_UNORM DXGI_FORMAT_BC4)_UNORM 
				fourccFormat = FOURCC_BC4; // converts to r8g8b8a8
				bitsPerPixel = 0x04;
				minBytes = 0x08;
				bCompressedFormat = true;
				blockWidth = 4;
				blockHeight = 4;
				bNormalized = false;
				bIsUNorm = true;
				break;
			case 0x5D: // DXGI_FORMAT_BC5_TYPELESS DXGI_FORMAT_BC5_UNORM DXGI_FORMAT_BC5_UNORM 
				fourccFormat = FOURCC_BC5; // converts to r8g8b8a8
				bitsPerPixel = 0x08;
				minBytes = 0x10;
				bCompressedFormat = true;
				blockWidth = 4;
				blockHeight = 4;
				bNormalized = false;
				bIsUNorm = true;
				break;
			case 0x5E: // DXGI_FORMAT_BC6H_TYPELESS DXGI_FORMAT_BC6H_UF16 DXGI_FORMAT_BC6H_UF16 // Uses cubemaps
				fourccFormat = FOURCC_BC6H; // converts to r8g8b8a8
				bitsPerPixel = 0x08;
				minBytes = 0x10;
				bCompressedFormat = true;
				blockWidth = 4;
				blockHeight = 4;
				bNormalized = false;
				bIsUNorm = true;
				break;
			case 0x5F: // DXGI_FORMAT_BC7_TYPELESS DXGI_FORMAT_BC7_UNORM DXGI_FORMAT_BC7_UNORM_SRGB  
				fourccFormat = FOURCC_BC7; // converts to r8g8b8a8
				bitsPerPixel = 0x08;
				minBytes = 0x10;
				bCompressedFormat = true;
				blockWidth = 4;
				blockHeight = 4;
				bNormalized = false;
				bIsUNorm = true;
				break;
			case 0x60: // DXGI_FORMAT_BC1_TYPELESS DXGI_FORMAT_BC1_UNORM DXGI_FORMAT_BC1_UNORM_SRGB  DXT1 (swizzled)
				fourccFormat = FOURCC_BC1; // converts to r8g8b8a8
				bitsPerPixel = 0x04;
				minBytes = 0x08;
				bCompressedFormat = true;
				blockWidth = 4;
				blockHeight = 4;
				bIsUNorm = true;
				bSwizzled = true;
				break;
			case 0x61: // DXGI_FORMAT_BC2_TYPELESS DXGI_FORMAT_BC2_UNORM DXGI_FORMAT_BC2_UNORM_SRGB  DXT2 / DXT3 (swizzled)
				fourccFormat = FOURCC_BC2; // converts to r8g8b8a8
				bitsPerPixel = 0x08;
				minBytes = 0x10;
				bCompressedFormat = true;
				blockWidth = 4;
				blockHeight = 4;
				bIsUNorm = true;
				bSwizzled = true;
				break;
			case 0x62: // DXGI_FORMAT_BC3_TYPELESS DXGI_FORMAT_BC3_UNORM DXGI_FORMAT_BC3_UNORM_SRGB  DXT4 / DXT5 (swizzled)
				fourccFormat = FOURCC_BC3; // converts to r8g8b8a8
				bitsPerPixel = 0x04;
				minBytes = 0x10;
				bCompressedFormat = true;
				blockWidth = 4;
				blockHeight = 4;
				bIsUNorm = true;
				bSwizzled = true;
				break;
			case 0x63: // DXGI_FORMAT_BC4_TYPELESS DXGI_FORMAT_BC4_UNORM DXGI_FORMAT_BC4_UNORM ATI1  (swizzled)
				fourccFormat = FOURCC_BC4; // converts to r8g8b8a8
				bitsPerPixel = 0x04;
				minBytes = 0x08;
				bCompressedFormat = true;
				blockWidth = 4;
				blockHeight = 4;
				bNormalized = false;
				bIsUNorm = true;
				bSwizzled = true;
				break;
			case 0x64: // DXGI_FORMAT_BC5_TYPELESS DXGI_FORMAT_BC5_UNORM DXGI_FORMAT_BC5_UNORM ATI2  (swizzled)
				fourccFormat = FOURCC_BC5; // converts to r8g8b8a8
				bitsPerPixel = 0x08;
				minBytes = 0x10;
				bCompressedFormat = true;
				blockWidth = 4;
				blockHeight = 4;
				bNormalized = false;
				bIsUNorm = true;
				bSwizzled = true;
				break;
			case 0x65: // DXGI_FORMAT_BC6H_TYPELESS DXGI_FORMAT_BC6H_UF16 DXGI_FORMAT_BC6H_UF16      (swizzled)
				fourccFormat = FOURCC_BC6H; // converts to r8g8b8a8
				bitsPerPixel = 0x08;
				minBytes = 0x10;
				bCompressedFormat = true;
				blockWidth = 4;
				blockHeight = 4;
				bNormalized = false;
				bIsUNorm = true;
				bSwizzled = true;
				break;
			case 0x66: // DXGI_FORMAT_BC7_TYPELESS DXGI_FORMAT_BC7_UNORM DXGI_FORMAT_BC7_UNORM_SRGB  (swizzled)
				fourccFormat = FOURCC_BC7; // converts to r8g8b8a8
				bitsPerPixel = 0x08;
				minBytes = 0x10;
				bCompressedFormat = true;
				blockWidth = 4;
				blockHeight = 4;
				bNormalized = false;
				bIsUNorm = true;
				bSwizzled = true;
				break;
			case 0x67: // GL_RGBA8UI GL_RGBA_INTEGER GL_UNSIGNED_BYTE
				rawFormat = "r8g8b8a8";
				bitsPerPixel = 0x20;
				minBytes = 0x04;
				break;
			case 0x68: // GL_RG8UI GL_RG_INTEGER GL_UNSIGNED_BYTE
				rawFormat = "r8g8";
				bitsPerPixel = 0x10;
				minBytes = 0x02;
				break;
			case 0x69: // GL_RG16F GL_RG GL_HALF_FLOAT_OES
				rawFormat = "r#F16g#F16";
				bitsPerPixel = 0x20;
				minBytes = 0x04;
				bHalfFloat = true;
				break;
			case 0x6A: // GL_R16F GL_RED GL_HALF_FLOAT_OES
				rawFormat = "r#F16";
				bitsPerPixel = 0x10;
				minBytes = 0x02;
				bHalfFloat = true;
				break;
			case 0x6B: // GL_R11F_G11F_B10F GL_RGB GL_UNSIGNED_INT_10F_11F_11F_REV
				rawFormat = "r#F11g#F11b#F10"; // very special case
				bitsPerPixel = 0x20;
				minBytes = 0x04;
				bConvert11Bit10BitFloat = true;
				break;
			case 0x6C: // GL_DEPTH32F_STENCIL8 GL_DEPTH_COMPONENT GL_FLOAT_32_UNSIGNED_INT_24_8_REV (df32p24s8)
				rawFormat = "d#F32p24s8";
				bitsPerPixel = 0x40;
				minBytes = 0x08;
				bIsDepth = true;
				bFloat = true;
				bD32FloatConvert = true;
				break;
			case 0x6D: // DXGI_FORMAT_R32G8X24_TYPELESS DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS DXGI_FORMAT_D32_FLOAT_S8X24_UINT 
				rawFormat = "d#F32s8p24";
				bitsPerPixel = 0x40;
				minBytes = 0x08;
				bIsDepth = true;
				bFloat = true;
				bD32FloatConvert = true;
				break;
			case 0x6E: // SEC_R16 SEC_R16 SEC_UNORM
				rawFormat = "r16";
				bitsPerPixel = 0x10;
				minBytes = 0x02;
				bIsUNorm = true;
				break;
			case 0x6F: // GL_ETC1_RGB8_OES GL_RGB GL_UNSIGNED_BYTE (alpha under, bpp adjusted)
				bIsETC1 = true; // converts to r8g8b8a8
				bCompressedFormat = true;
				blockWidth = 4;
				blockHeight = 4;
				blockWidth = 4;
				blockHeight = 4;
				rawFormat = "RGB";
				if (texHeader.KTGL_GD_COLOR_SPACE == KTGL_GD_COLOR_SPACE_TYPE::sRGB_FROM_LINEAR)
				{
					rawFormat = "sRGB";
				}
				bitsPerPixel = 0x04; // hard codes to 0x08 but we adjust the height
				HEIGHT *= 2;
				minBytes = 0x08;
				bHasAlphaAtlas = true;
				break;
			case 0x70: // GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG GL_RGB GL_UNSIGNED_BYTE (likely alpha under, bpp adjusted)
				bIsPVRTC = true; // converts to r8g8b8a8
				bCompressedFormat = true;
				blockWidth = 4;
				blockHeight = 4;
				bitsPerPixel = 0x04; // hard codes to 0x08 but we adjust the height
				HEIGHT *= 2;
				minBytes = 0x08;
				bHasAlphaAtlas = true;
				break;
			case 0x71: // GL_COMPRESSED_RGBA8_ETC2_EAC GL_RGBA GL_UNSIGNED_BYTE
				bIsETC2 = true; // converts to r8g8b8a8
				bCompressedFormat = true;
				blockWidth = 4;
				blockHeight = 4;
				rawFormat = "RGBA";
				if (texHeader.KTGL_GD_COLOR_SPACE == KTGL_GD_COLOR_SPACE_TYPE::sRGB_FROM_LINEAR)
				{
					rawFormat = "sRGBA";
				}
				bitsPerPixel = 0x08;
				minBytes = 0x10;
				break;
			case 0x72: // GL_R8 GL_RED GL_UNSIGNED_BYTE
				rawFormat = "r8";
				bitsPerPixel = 0x08;
				minBytes = 0x01;
				bIsUNorm = true;
				break;
			case 0x73: // GL_RG8 GL_RG GL_UNSIGNED_BYTE
				rawFormat = "r8g8";
				bitsPerPixel = 0x10;
				minBytes = 0x02;
				bIsUNorm = true;
				break;
			case 0x74: // DXGI_FORMAT_R8G8B8A8_TYPELESS DXGI_FORMAT_R8G8B8A8_UINT DXGI_FORMAT_R8G8B8A8_UINT
				rawFormat = "r8g8b8a8";
				bitsPerPixel = 0x20;
				minBytes = 0x04;
				break;
			case 0x75: // DXGI_FORMAT_R8G8_TYPELESS DXGI_FORMAT_R8G8_UINT DXGI_FORMAT_R8G8_UINT
				rawFormat = "r8g8";
				bitsPerPixel = 0x10;
				minBytes = 0x02;
				break;
			case 0x76: // DXGI_FORMAT_R16G16_TYPELESS DXGI_FORMAT_R16G16_FLOAT DXGI_FORMAT_R16G16_FLOAT
				rawFormat = "r#F16g#F16";
				bitsPerPixel = 0x10;
				minBytes = 0x02;
				bHalfFloat = true;
				break;
			case 0x77: // DXGI_FORMAT_R16_TYPELESS DXGI_FORMAT_R16_FLOAT DXGI_FORMAT_R16_FLOAT
				rawFormat = "r#F16";
				bitsPerPixel = 0x10;
				minBytes = 0x02;
				bHalfFloat = true;
				break;
			case 0x78: // DXGI_FORMAT_R11G11B10_FLOAT GL_HALF_FLOAT_OES GL_HALF_FLOAT_OES
				rawFormat = "r#F11g#F11b#F10"; // very special case
				bitsPerPixel = 0x20;
				minBytes = 0x04;
				bConvert11Bit10BitFloat = true;
				break;
			case 0x79: // SEC_R16 SEC_R16 SEC_UNORM (dupe of 0x6E so something else might be going on)
				rawFormat = "r16";
				bitsPerPixel = 0x10;
				minBytes = 0x02;
				bIsUNorm = true;
				break;
			case 0x7A: // GL_COMPRESSED_R11_EAC GL_RED GL_UNSIGNED_BYTE
				bIsETC2 = true; // converts to r8g8b8a8
				bCompressedFormat = true;
				blockWidth = 4;
				blockHeight = 4;
				rawFormat = "R";
				if (texHeader.KTGL_GD_COLOR_SPACE == KTGL_GD_COLOR_SPACE_TYPE::sRGB_FROM_LINEAR)
				{
					rawFormat = "Rs";
				}
				bitsPerPixel = 0x04;
				minBytes = 0x08;
				break;
			case 0x7B: // GL_COMPRESSED_RG11_EAC GL_RG GL_UNSIGNED_BYTE
				bIsETC2 = true; // converts to r8g8b8a8
				bCompressedFormat = true;
				blockWidth = 4;
				blockHeight = 4;
				rawFormat = "RG";
				if (texHeader.KTGL_GD_COLOR_SPACE == KTGL_GD_COLOR_SPACE_TYPE::sRGB_FROM_LINEAR)
				{
					rawFormat = "RGs";
				}
				bitsPerPixel = 0x08;
				minBytes = 0x10;
				break;
			case 0x7C: // GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 GL_RGBA GL_UNSIGNED_BYTE
				bIsETC2 = true; // converts to r8g8b8a8
				bCompressedFormat = true;
				blockWidth = 4;
				blockHeight = 4;
				rawFormat = "RGBA1";
				if (texHeader.KTGL_GD_COLOR_SPACE == KTGL_GD_COLOR_SPACE_TYPE::sRGB_FROM_LINEAR)
				{
					rawFormat = "sRGBA1";
				}
				bitsPerPixel = 0x04;
				minBytes = 0x08;
				break;
			case 0x7D: //  ASTC uses ASTC_SUB format
			case 0x7E: //  also ASTC uses ASTC_SUB format likely swizzled flaged?
				if (texHeader.KTGL_PIXEL_FORMAT == 0x7E)
				{
					bSwizzled = true; // guessing with this here
				}
				rawFormat = "ASTC_12x12"; // default converts to r8g8b8a8
				bCompressedFormat = true;
				bitsPerPixel = 0.89;
				minBytes = 0x10;
				bIsASTC = true;
				if (header.HEADER_EX_SIZE)
				{
					switch (ExList[i].KT_ASTC_FORMAT)
					{
					case EG1TASTCFormat::ASTC_4x4:
						rawFormat = "ASTC_4x4";
						bitsPerPixel = 8.0;
						blockWidth = 4;
						blockHeight = 4;
						break;
					case EG1TASTCFormat::ASTC_5x4:
						rawFormat = "ASTC_5x4";
						bitsPerPixel = 6.40;
						blockWidth = 5;
						blockHeight = 4;
						break;
					case EG1TASTCFormat::ASTC_5x5:
						rawFormat = "ASTC_5x5";
						bitsPerPixel = 5.12;
						blockWidth = 5;
						blockHeight = 5;
						break;
					case EG1TASTCFormat::ASTC_6x5:
						rawFormat = "ASTC_6x5";
						bitsPerPixel = 4.27;
						blockWidth = 6;
						blockHeight = 5;
						break;
					case EG1TASTCFormat::ASTC_6x6:
						rawFormat = "ASTC_6x6";
						bitsPerPixel = 3.56;
						blockWidth = 6;
						blockHeight = 6;
						break;
					case EG1TASTCFormat::ASTC_8x5:
						rawFormat = "ASTC_8x5";
						bitsPerPixel = 3.20;
						blockWidth = 8;
						blockHeight = 5;
						break;
					case EG1TASTCFormat::ASTC_8x6:
						rawFormat = "ASTC_8x6";
						bitsPerPixel = 2.67;
						blockWidth = 8;
						blockHeight = 6;
						break;
					case EG1TASTCFormat::ASTC_8x8:
						rawFormat = "ASTC_8x8";
						bitsPerPixel = 2.00;
						blockWidth = 8;
						blockHeight = 8;
						break;
					case EG1TASTCFormat::ASTC_10x5:
						rawFormat = "ASTC_10x5";
						bitsPerPixel = 2.56;
						blockWidth = 10;
						blockHeight = 5;
						break;
					case EG1TASTCFormat::ASTC_10x6:
						rawFormat = "ASTC_10x6";
						bitsPerPixel = 2.13;
						blockWidth = 10;
						blockHeight = 6;
						break;
					case EG1TASTCFormat::ASTC_10x8:
						rawFormat = "ASTC_10x8";
						bitsPerPixel = 1.60;
						blockWidth = 10;
						blockHeight = 8;
						break;
					case EG1TASTCFormat::ASTC_10x10:
						rawFormat = "ASTC_10x10";
						bitsPerPixel = 1.28;
						blockWidth = 10;
						blockHeight = 10;
						break;
					case EG1TASTCFormat::ASTC_12x10:
						rawFormat = "ASTC_12x10";
						bitsPerPixel = 1.07;
						blockWidth = 12;
						blockHeight = 10;
						break;
					case EG1TASTCFormat::ASTC_12x12:
						rawFormat = "ASTC_12x12";
						bitsPerPixel = 0.89;
						blockWidth = 12;
						blockHeight = 12;
						break;
					default:
						{
							g_nfn->NPAPI_PopupDebugLog(0);
							LogDebug("Uknown ASTC Texture on image %d of %d!\nRestart app with 'Enable debug log' on in 'Tools/Project G1M' menu for info.\n", i + 1, ExList[i].KT_ASTC_FORMAT);
							assert(0 && "Unknown ASTC texture format.");
						}
						break;
					}
				}
				break;
			default:
				g_nfn->NPAPI_PopupDebugLog(0);
				LogDebug("Uknown Texture on index #%d of %d!\nRestart app with 'Enable debug log' on in 'Tools/Project G1M' menu for info.\n", i, texHeader.KTGL_PIXEL_FORMAT);
				assert(0 && "Unknown texture format.");
				break;
			}

			///////////////////////////////
			//    COMPUTE IMAGE SIZE     //
			///////////////////////////////

			firstMipSize = TextureSizeWithMips(
				WIDTH,
				HEIGHT,
				DEPTH,
				1 /*MIPS*/,
				1 /*PLANE_COUNT*/,
				FACES,
				minBytes,
				blockWidth,
				blockHeight,
				bCompressedFormat,
				bitsPerPixel
			);

			totalTexBufferLen = TextureSizeWithMips(
				WIDTH,
				HEIGHT,
				DEPTH,
				MIPS,
				PLANE_COUNT,
				FACES,
				minBytes,
				blockWidth,
				blockHeight,
				bCompressedFormat,
				bitsPerPixel
			);

			///////////////////////////////
			//       DEBUG LOG DATA      //
			///////////////////////////////

			if (bDebugLog)
			{
				LogDebug("G1T_TEXTURE #%d @ %d\n", i + 1, offset);
				// Log any metadata
				std::string TYPE_STR = "";
				if (AttrList[i].TYPE != TEX_ATTR_TYPE::NONE)
				{
					TYPE_STR = "";
					switch (AttrList[i].TYPE)
					{
					case TEX_ATTR_TYPE::NORMAL_MAP:
						TYPE_STR = "NORMAL_MAP";
						break;
					case TEX_ATTR_TYPE::NORMAL_WITH_ALPHA:
						TYPE_STR = "NORMAL_WITH_ALPHA";
						break;
					case TEX_ATTR_TYPE::N3DS_Wii_SWIZZLE:
						TYPE_STR = "N3DS_Wii_SWIZZLE";
						break;
					case TEX_ATTR_TYPE::WiiU_SWIZZLE:
						TYPE_STR = "WiiU_SWIZZLE";
						break;
					case TEX_ATTR_TYPE::PS4_PLANE_ARRAY:
						TYPE_STR = "PS4_PLANE_ARRAY";
						break;
					case TEX_ATTR_TYPE::PSVITA_PS5_SWIZZLE:
						TYPE_STR = "PSVITA_PS5_SWIZZLE";
						break;
					case TEX_ATTR_TYPE::ASTC_TYPE:
						TYPE_STR = "ASTC_TYPE";
						break;
					default:
						TYPE_STR = "Unknown type";
						break;
					}
					LogDebug("\t*TEX_ATTR_TYPE:\t\t%d (%s)\n", AttrList[i].TYPE, TYPE_STR.c_str());
				}
				if (header.HEADER_EX_SIZE != 0)
				{
					TYPE_STR = "";
					switch (ExList[i].TYPE)
					{
					case TEX_ATTR_TYPE::NORMAL_MAP:
						TYPE_STR = "NORMAL_MAP";
						break;
					case TEX_ATTR_TYPE::NORMAL_WITH_ALPHA:
						TYPE_STR = "NORMAL_WITH_ALPHA";
						break;
					case TEX_ATTR_TYPE::N3DS_Wii_SWIZZLE:
						TYPE_STR = "N3DS_Wii_SWIZZLE";
						break;
					case TEX_ATTR_TYPE::WiiU_SWIZZLE:
						TYPE_STR = "WiiU_SWIZZLE";
						break;
					case TEX_ATTR_TYPE::PS4_PLANE_ARRAY:
						TYPE_STR = "PS4_PLANE_ARRAY";
						break;
					case TEX_ATTR_TYPE::PSVITA_PS5_SWIZZLE:
						TYPE_STR = "PSVITA_PS5_SWIZZLE";
						break;
					case TEX_ATTR_TYPE::ASTC_TYPE:
						TYPE_STR = "ASTC_TYPE";
						break;
					default:
						TYPE_STR = "Unknown type";
						break;
					}
					LogDebug("\t*TEX_EX_TYPE:\t\t%d (%s)\n", ExList[i].TYPE, TYPE_STR.c_str());
					LogDebug("\t*TEX_EX_COUNT:\t\t%d\n", ExList[i].COUNT);
					LogDebug("\t*ARRAY_DEPTH:\t\t%d\n", ExList[i].ARRAY_DEPTH);
					if (ExList[i].TYPE == TEX_ATTR_TYPE::ASTC_TYPE && ExList[i].COUNT == 1)
					{
						LogDebug("\t*KT_ASTC_FORMAT:\t%d (%s)\n", ExList[i].KT_ASTC_FORMAT, rawFormat.c_str());
					}
				}
				// Log main entry
				TYPE_STR = "";
				switch (texHeader.KTGL_TEXTURE_TYPE)
				{
				case S_GT1_LOAD_TYPE::PLANAR:
					TYPE_STR = "PLANAR";
					break;
				case S_GT1_LOAD_TYPE::CUBE:
					TYPE_STR = "CUBE";
					break;
				case S_GT1_LOAD_TYPE::VOLUME:
					TYPE_STR = "VOLUME";
					break;
				case S_GT1_LOAD_TYPE::CUBE_ARRAY:
					TYPE_STR = "CUBE_ARRAY";
					break;
				case S_GT1_LOAD_TYPE::PLANE_ARRAY:
					TYPE_STR = "PLANE_ARRAY";
					break;
				default:
					TYPE_STR = "Unknown type";
					break;
				}
				LogDebug("\tKTGL_TEXTURE_TYPE:\t%d (%s)\n", texHeader.KTGL_TEXTURE_TYPE, TYPE_STR.c_str());
				LogDebug("\tMIP_COUNT:\t\t%d\n", texHeader.MIP_COUNT);
				LogDebug("\tKTGL_PIXEL_FORMAT:\t0x%02X\n", texHeader.KTGL_PIXEL_FORMAT);
				LogDebug("\tSYS_TEX_FORMAT:\t\t%s\n", getFormatStr(header.SYSTEM, texHeader.KTGL_PIXEL_FORMAT).c_str());
				LogDebug("\tPACKED_WIDTH:\t\t%d (%d)\n", texHeader.PACKED_WIDTH, WIDTH);
				LogDebug("\tPACKED_HEIGHT:\t\t%d (%d)\n", texHeader.PACKED_HEIGHT, HEIGHT);
				LogDebug("\tPACKED_DEPTH:\t\t%d (%d)\n", texHeader.PACKED_DEPTH, DEPTH);
				LogDebug("\tREAD_G1T_HEADER_EX:\t%d\n", texHeader.READ_G1T_HEADER_EX);
				LogDebug("\tKTGL_GD_TEXADDRESS1:\t%d\n", texHeader.KTGL_GD_TEXADDRESS1);
				LogDebug("\tKTGL_GD_TEXADDRESS2:\t%d\n", texHeader.KTGL_GD_TEXADDRESS2);
				LogDebug("\tKTGL_GD_TEXADDRESS3:\t%d\n", texHeader.KTGL_GD_TEXADDRESS3);
				LogDebug("\tKTGL_GD_TEXFILTER1:\t%d\n", texHeader.KTGL_GD_TEXFILTER1);
				LogDebug("\tKTGL_GD_TEXFILTER2:\t%d\n", texHeader.KTGL_GD_TEXFILTER2);
				LogDebug("\tKTGL_GD_MIPFILTER1:\t%d\n", texHeader.KTGL_GD_MIPFILTER1);
				LogDebug("\tKTGL_GD_MIPFILTER2:\t%d\n", texHeader.KTGL_GD_MIPFILTER2);
				LogDebug("\tHAS_TEX_EX_HEADER:\t%d\n", texHeader.HAS_TEX_EX_HEADER);
				if (texHeader.HAS_TEX_EX_HEADER >= 1)
				{
					LogDebug("\tTEX_HEADER_EX_SIZE:\t%d\n", texHeader.TEX_HEADER_EX_SIZE);
					LogDebug("\tZ_SCALE:\t\t\t%.4f\n", texHeader.Z_SCALE);
					LogDebug("\tEX_FACES:\t\t%d (%d)\n", texHeader.EX_FACES, FACES);
					LogDebug("\tEX_ARRAY_COUNT:\t%d (%d)\n", texHeader.EX_ARRAY_COUNT, PLANE_COUNT);
					LogDebug("\tEX_SWIZZLE:\t\t%d\n", texHeader.EX_SWIZZLE);
					LogDebug("\tKTGL_GD_COLOR_SPACE:\t%d\n", texHeader.KTGL_GD_COLOR_SPACE);
					LogDebug( "\tEX_WIDTH:\t\t%d\n", texHeader.EX_WIDTH);
					LogDebug("\tEX_HEIGHT:\t\t%d\n", texHeader.EX_HEIGHT);
				}
				LogDebug("\t\t-TEXTURE %d META-\n", i + 1);
				if (bCompressedFormat)
				{
					LogDebug("\tbCompressedFormat:\t%s\n", bCompressedFormat ? "true" : "false");
					if (fourccFormat != -1) LogDebug("\tfourccFormat:\t\t%c%c%c%c\n", fourccFormat & 0xFF, (fourccFormat >> 8) & 0xFF, (fourccFormat >> 16) & 0xFF, (fourccFormat >> 24) & 0xFF);
					if (bIsETC1)            LogDebug("\tbIsETC1:\t\t\t%s\n", bIsETC1 ? "true" : "false");
					if (bPICAETC)           LogDebug("\tbPICAETC:\t\t%s\n", bPICAETC ? "true" : "false");
					if (bIsETC2)            LogDebug("\tbIsETC2:\t\t\t%s\n", bIsETC2 ? "true" : "false");
					if (bPalette4)          LogDebug("\tbPalette4:\t\t%s\n", bPalette4 ? "true" : "false");
					if (bPalette8)          LogDebug("\tbPalette8:\t\t%s\n", bPalette8 ? "true" : "false");
					if (bPalette14)         LogDebug("\tbPalette14:\t\t%s\n", bPalette14 ? "true" : "false");
					if (bIsPVRTC)           LogDebug("\tbIsPVRTC:\t\t%s\n", bIsPVRTC ? "true" : "false");
					if (bIsASTC)            LogDebug("\tbIsASTC:\t\t\t%s\n\n", bIsASTC ? "true" : "false");
					if (bHasAlphaAtlas)     LogDebug("\tbHasAlphaAtlas:\t\t%s\n", bHasAlphaAtlas ? "true" : "false");
					if (b3DSAlpha)          LogDebug("\tb3DSAlpha:\t\t%s\n", b3DSAlpha ? "true" : "false");
					LogDebug("\tblockDim:\t\t\t%dx%d\n", blockWidth, blockHeight);
					LogDebug("\tblockByteSize:\t\t%d\n", minBytes);
				}
				if (rawFormat != "")
				{
					LogDebug("\trawFormat:\t\t%s\n", rawFormat.c_str());
				}
				LogDebug("\tbitsPerPixel:\t\t%.2f\n", bitsPerPixel);
				LogDebug("\tminBytes:\t\t\t%d\n", minBytes);
				LogDebug("\tfirstMipSize:\t\t%d\n", int(firstMipSize));
				LogDebug("\ttotalTexBufferLen:\t\t%d\n", int(totalTexBufferLen));
				if (bIsUNorm)                LogDebug("\tbIsUNorm:\t\t%s\n", bIsUNorm ? "true" : "false");
				if (bNormalMap)              LogDebug("\tbNormalMap:\t\t%s\n", bNormalMap ? "true" : "false");
				if (bNormalWithAlpha)        LogDebug("\tbNormalWithAlpha:\t\t%s\n", bNormalWithAlpha ? "true" : "false");
				if(bNormalized)              LogDebug("\tbNormalize:\t\t%s\n", bNormalized ? "true" : "false");
				if (bSwizzled)               LogDebug("\tbSwizzled:\t\t%s\n", bSwizzled ? "true" : "false");
				if (bSigned)                 LogDebug("\tbSigned:\t\t\t%s\n", bSigned ? "true" : "false");	
				if (bNeedsX360EndianSwap)    LogDebug("\tbNeedsX360EndianSwap:\t%s\n", bNeedsX360EndianSwap ? "true" : "false");
				if (bFloat)                  LogDebug("\tbFloat:\t\t\t%s\n", bFloat ? "true" : "false");
				if (bHalfFloat)              LogDebug("\tbHalfFloat:\t\t%s\n", bHalfFloat ? "true" : "false");
				if (bIsDepth)                LogDebug("\tbIsDepth:\t\t%s\n", bIsDepth ? "true" : "false");
				if (bD16Convert)             LogDebug("\tbD16Convert:\t\t%s\n", bD16Convert ? "true" : "false");
				if (bD32Convert)             LogDebug("\tbD32Convert:\t\t%s\n", bD32Convert ? "true" : "false");
				if (bD32FloatConvert)        LogDebug("\tbD32FloatConvert:\t\t%s\n", bD32FloatConvert ? "true" : "false");
				if (bD24_8Convert)           LogDebug("\tbD24_8Convert:\t\t%s\n", bD24_8Convert ? "true" : "false");
				if (bConvert10BitFloat)      LogDebug("\tbConvert10BitFloat:\t%s\n", bConvert10BitFloat ? "true" : "false");
				if (bConvert11Bit10BitFloat) LogDebug("\tbConvert11Bit10BitFloat:\t%s\n", bConvert11Bit10BitFloat ? "true" : "false");				
			}

			///////////////////////////////
			//     CHECK IMAGE SIZE      //
			///////////////////////////////

			// Sanity check on texture data size
			if (header.TEX_COUNT == 1)
			{
				// If there is only one texture, check if there is extra data and meet it
				if (offset + totalTexBufferLen < (uint32_t)bufferLen)
				{
					LogDebug("Data size of %d but extra data found. Expecting %d bytes. Format might be incorrect.\n", (uint32_t)totalTexBufferLen, (uint32_t)(bufferLen - offset));
					totalTexBufferLen = bufferLen - offset;
				}
				else if (offset + totalTexBufferLen > (uint32_t)bufferLen)
				{
					LogDebug("Data size of %d but more than the data left in the file %d. Format might be incorrect.\n", (uint32_t)totalTexBufferLen, (uint32_t)(bufferLen - offset));
					totalTexBufferLen = bufferLen - offset;
				}
			}
			else if (i < int(header.TEX_COUNT - 1))
			{
				uint32_t expected_offset = header.TEX_OFFSET + offsetList[i + 1];
				uint32_t dif = expected_offset - (offset + totalTexBufferLen);
				// Check if the size matchs the next texture.
				// if not, adjust based on if over or under.
				if (expected_offset != offset + totalTexBufferLen)
				{
					// If next expected offset is ahead, meet it. But check for max file size.
					if (expected_offset > offset + totalTexBufferLen)
					{
						LogDebug("Data found on texture %d was less than the expected. At %d instead of %d, format might be incorrect.\n", i+1, (uint32_t)totalTexBufferLen, (uint32_t)(expected_offset - offset));
						if (offset + totalTexBufferLen + dif <= (uint32_t)bufferLen)
						{
							totalTexBufferLen += dif;
						}
						else
						{
							LogDebug("Data found on texture %d size difference more than the file size at %d, format might be incorrect.\n", i+1, (uint32_t)(offset + totalTexBufferLen));
						}
					}
					else
						// If expected offset is under, warn and continue. But check for max file size.
					{
						if (!bHasAlphaAtlas)
						{
							LogDebug("Data found on texture %d was more than expected at %d instead of %d, format might be incorrect.\n", i+1, (uint32_t)totalTexBufferLen, (uint32_t)(expected_offset - offset));
							totalTexBufferLen = expected_offset - offset;
						}
					}
				}
			}
			else
			{
				// If this is the last texture, check if there is extra data and meet it
				if (offset + totalTexBufferLen < (uint32_t)bufferLen)
				{
					LogDebug("Extra data found on texture %d with size of %d instead of %d, format might be incorrect.\n", i+1, (uint32_t)totalTexBufferLen, (int)(bufferLen - offset));
					totalTexBufferLen = bufferLen - offset;
				}
			}

			// these formats have incorrect offsets in the skip table so we must fix
			if (bHasAlphaAtlas)
			{
				if (i < (header.TEX_COUNT - 1))
				{
					offsetList[i + 1] = offsetList[i] + texHeader.headerSize + (uint32_t)totalTexBufferLen;
				}
			}

			///////////////////////////////
			//        UNTILE DATA        //
			///////////////////////////////

			// We untile, decomp then post convert
			// untiledTexData -> decompTexData -> postConvertTexData -> finalTexData
			BYTE* texStart = buffer + offset;
			// for sanity checks between stages
			BYTE* passOffPointer = nullptr;
			uint32_t currentImageSize = firstMipSize;
			
			// Endian swap if needed
			if (header.SYSTEM == PLATFORM::X360)
			{
				if (bNeedsX360EndianSwap || bSwizzled)
				{	//Swap endian for x360 textures
					uint16_t* tmp = (uint16_t*)(buffer + offset);
					for (uint32_t i = 0; i < totalTexBufferLen / 2; i++)
					{
						LITTLE_BIG_SWAP(tmp[i]);
					}
				}
			}

			// Being that there can be more than one texture in a texture now,
			// we process by planes
			uint32_t planesToProcess = bPlanes ? PLANE_COUNT : 1;
			for (uint32_t p = 0; p < planesToProcess; p++)
			{
				BYTE* untiledTexData = nullptr;
				bool bUntiledTexShouldFree = false;

				// Untile/Unswizzle the data if relevant
				if (bSwizzled)
				{
					switch (header.SYSTEM)
					{
					case PLATFORM::X360:
					{
						untiledTexData = (BYTE*)rapi->Noesis_UnpooledAlloc(currentImageSize); bUntiledTexShouldFree = true;
						if (fourccFormat == -1)
						{
							rapi->Noesis_UntileImageRAW(untiledTexData, texStart, (int)currentImageSize, WIDTH, HEIGHT, (int)bitsPerPixel);
						}
						else
						{
							rapi->Noesis_UntileImageDXT(untiledTexData, texStart, (int)currentImageSize, WIDTH, HEIGHT, (int)bitsPerPixel * 2);
						}
						passOffPointer = untiledTexData;
						break;
					}
					case PLATFORM::NWiiU:
					{
						int texDimType = 1; // Texture2D aka dimensions
						switch (texHeader.KTGL_TEXTURE_TYPE)
						{
						case S_GT1_LOAD_TYPE::PLANAR:
							texDimType = 1; // Texture2D
							break;
						case S_GT1_LOAD_TYPE::CUBE:
							texDimType = 3; // TextureCube
							break;
						case S_GT1_LOAD_TYPE::VOLUME:
							texDimType = 2; // Texture3D
							break;
						default:
							break;
						}
						int WiiUSwizzle = 0; // should pretty much always be 0
						if (header.HEADER_EX_SIZE != 0)
						{
							WiiUSwizzle = ExList[i].WiiU_SWIZZLE;
						}

						// starting here 
						// https://github.com/KillzXGaming/Switch-Toolbox/blob/16de585c7b998826e39ac88915f64554c0025423/File_Format_Library/FileFormats/HyruleWarriors/G1T/G1TFile.cs#L249
						GX2Surface surf = { 0 };
						surf.bpp = (uint32_t)bitsPerPixel;
						surf.height = HEIGHT;
						surf.width = WIDTH;
						surf.aa = 0;
						surf.alignment = 0;
						surf.depth = PLANE_COUNT; // DEPTH;
						surf.dim = texDimType;
						surf.format = getNWiiUFormatValue(texHeader.KTGL_PIXEL_FORMAT, (uint8_t)texHeader.KTGL_GD_COLOR_SPACE);
						surf.use = 2; // USE_COLOR_BUFFER
						surf.pitch = 0;
						surf.data = buffer + offset; // this function gets the array textures so we always start at the start of the tex buffer
						surf.numMips = MIPS;
						surf.mipOffset.resize(MIPS);
						surf.mipData = buffer + offset; // this function gets the array textures so we always start at the start of the tex buffer
						surf.tileMode = 0x04; // MODE_2D_TILED_THIN1
						surf.swizzle = WiiUSwizzle;
						surf.imageSize = totalTexBufferLen;
						surf.numArray = PLANE_COUNT;

						untiledTexData = Gx2Decode(surf, p, 0, rapi); bUntiledTexShouldFree = true;
						passOffPointer = untiledTexData;
						break;
					}
					case PLATFORM::PS4:
					{
						untiledTexData = (BYTE*)rapi->Noesis_UnpooledAlloc(currentImageSize); bUntiledTexShouldFree = true;
						if (fourccFormat == -1)
						{
							NoesisMisc_Untile1dThin(untiledTexData, (int)currentImageSize, texStart, (int)currentImageSize, WIDTH, HEIGHT, (int)bitsPerPixel, 0, rapi);
						}
						else
						{
							NoesisMisc_Untile1dThin(untiledTexData, (int)currentImageSize, texStart, (int)currentImageSize, WIDTH, HEIGHT, (int)bitsPerPixel, 1, rapi);
						}
						passOffPointer = untiledTexData;
						break;
					}
					case PLATFORM::NSwitch:
					{
						untiledTexData = (BYTE*)rapi->Noesis_UnpooledAlloc(currentImageSize * 4); bUntiledTexShouldFree = true;
						int nblockWidth = 4;
						int nblockHeight = (HEIGHT <= 256) ? 3 : 4;
						nblockHeight = (HEIGHT <= 128) ? 2 : nblockHeight;
						nblockHeight = (HEIGHT <= 64) ? 1 : nblockHeight;
						int widthInBlocks = (WIDTH + (nblockWidth - 1)) / nblockWidth;
						int	heightInBlocks = (HEIGHT + (nblockHeight - 1)) / nblockHeight;
						int maxBlockHeight = rapi->Image_TileCalculateBlockLinearGOBBlockHeight(HEIGHT, nblockHeight);
						rapi->Image_UntileBlockLinearGOBs(untiledTexData, (int)currentImageSize, texStart, (int)currentImageSize, widthInBlocks, heightInBlocks, maxBlockHeight, (int)(bitsPerPixel) * 2);
						passOffPointer = untiledTexData;
						break;
					}
					case PLATFORM::WinMac:
					case PLATFORM::WinDX12:
					{
						if (texHeader.EX_SWIZZLE != EX_SWIZZLE_TYPE::NONE)
						{
							switch (texHeader.EX_SWIZZLE)
							{
							case EX_SWIZZLE_TYPE::DX12_64kb:  // DeswizzleD3D12_64KB
							{
								if (currentImageSize > 65536) // texture data less than 64kb does not swizzle
								{
									if (bCompressedFormat)
									{
										// working on DXT, unsure about other formats
										untiledTexData = (BYTE*)rapi->Noesis_UnpooledAlloc(currentImageSize); bUntiledTexShouldFree = true;
										DeswizzleD3D12_64KBDXT(texStart, untiledTexData, (uint32_t)((WIDTH + blockWidth - 1) / blockWidth), (uint32_t)((HEIGHT + blockHeight - 1) / blockHeight), minBytes);
										passOffPointer = untiledTexData;
									}
									else
									{
										// worked on 32 bpp but could use more samples
										untiledTexData = (BYTE*)rapi->Noesis_UnpooledAlloc(currentImageSize); bUntiledTexShouldFree = true;
										DeswizzleD3D12_64KBRaw(texStart, untiledTexData, WIDTH, HEIGHT, bitsPerPixel);
										passOffPointer = untiledTexData;
									}
								}
								else
								{
									// if not used, passes the start pointer on to the next section
									// untiledTexData -> decompTexData -> postConvertTexData -> finalTexData
									passOffPointer = texStart;
								}
								break;
							}
							default:
								// if not used, passes the start pointer on to the next section
								// untiledTexData -> decompTexData -> postConvertTexData -> finalTexData
								passOffPointer = texStart;
								break;
							}
						}
						else
						{
							// if not used, passes the pointer on to the next section
							// untiledTexData -> decompTexData -> postConvertTexData -> finalTexData
							passOffPointer = texStart;
						}
						break;
					}
					// all others
					case PLATFORM::PS2:     
					case PLATFORM::PS3:
					case PLATFORM::NWii:
					case PLATFORM::N3DS:    // Should be working
					case PLATFORM::PSVita:  // working
					case PLATFORM::PS5:
						untiledTexData = (BYTE*)rapi->Noesis_UnpooledAlloc(currentImageSize); bUntiledTexShouldFree = true;
						if (fourccFormat == -1)
						{
							rapi->Image_MortonOrder(texStart, untiledTexData, WIDTH, HEIGHT, (int)bitsPerPixel, 0);
						}
						else
						{
							rapi->Image_MortonOrder(texStart, untiledTexData, WIDTH >> 1, HEIGHT >> 2, (int)bitsPerPixel, 1);
						}
						passOffPointer = untiledTexData;
						break;
					case PLATFORM::Android: // No known swizzles
					case PLATFORM::iOS:     // No known swizzles
					default:
						// if not used, passes the pointer on to the next section
						// untiledTexData -> decompTexData -> postConvertTexData -> finalTexData
						passOffPointer = texStart;
						break;
					}
				}
				else
				{
					// if not used, passes the pointer on to the next section
					// untiledTexData -> decompTexData -> postConvertTexData -> finalTexData
					passOffPointer = texStart;
				}

				///////////////////////////////
				//    DECOMPRESS TEXTURES    //
				///////////////////////////////

				BYTE* decompTexData = nullptr;
				bool bDecompTexShouldFree = false;

				if (bCompressedFormat)
				{
					// Decompress PVRTC
					// Mode		Block	Bits		Bytes 
					//			Size	per Pixel	Per Block
					// ----------------------------------
					// 4bpp		4x4		4			8
					// 2bpp		8x4		2			8
					if (bIsPVRTC)
					{
						decompTexData = rapi->Image_DecodePVRTC(passOffPointer, (int)currentImageSize, WIDTH, HEIGHT, (int)bitsPerPixel); bDecompTexShouldFree = true;
						rawFormat = "r8g8b8a8";
						bitsPerPixel = 0x20;
						currentImageSize = (uint32_t)(WIDTH * HEIGHT * 4);
						passOffPointer = decompTexData;
					}
					// Decompress ASTC
					// Block	Bits		Bytes 
					// Size		per Pixel	Per Block
					// ------------------------------
					// 4x4		8			16
					// 5x4		6.4			16
					// 5x5		5.12		16
					// 6x5		4.27		16
					// 6x6 		3.56		16
					// 8x5		3.20		16
					// 8x6		2.67		16
					// 10x5		2.56		16
					// 10x6		2.13		16
					// 8x8		2			16
					// 10x8		1.60		16
					// 10x10	1.28		16
					// 12x10	1.07		16
					// 12x12	0.89		16
					else if (bIsASTC)
					{
						int pBlockDims[3] = { (int)blockWidth, (int)blockHeight, (int)DEPTH };
						int pImageSize[3] = { (int)WIDTH, (int)HEIGHT,  (int)DEPTH };
						decompTexData = (BYTE*)rapi->Noesis_UnpooledAlloc(WIDTH * HEIGHT * DEPTH * 4); bDecompTexShouldFree = true;
						rapi->Image_DecodeASTC(decompTexData, passOffPointer, (unsigned int)currentImageSize, pBlockDims, pImageSize);
						rawFormat = "r8g8b8a8";
						bitsPerPixel = 0x20;
						currentImageSize = (uint32_t)(WIDTH * HEIGHT * 4);
						passOffPointer = decompTexData;
					}
					// Decompress ETC - 4x4
					// Format 		Block		 Bits		Notes
					//				Size (bytes) per Pixel	
					// ------------------------------------------
					// ETC1			4			 8			RGB only, no alpha channel support.
					// ETC2 RGB		4			 8			RGB, higher quality compared to ETC1.
					// ETC2 RGBA	8			 16			RGBA with full alpha channel.
					// ETC2 RGBA1	4			 8			RGBA with 1 - bit "punchthrough" alpha(fully opaque or fully transparent).
					// EAC R11		4			 8			Single - channel(Red or grayscale) data, 11 - bits per channel with 0.5 bytes per channel.
					// EAC RG11		8			 16			Two - channel(Red and Green) data, 11 - bits per channel, 1 byte per pixel total.
					else if (bIsETC1 || bIsETC2)
					{
						if (bPICAETC)
						{
							decompTexData = (BYTE*)rapi->Noesis_UnpooledAlloc(WIDTH * HEIGHT * DEPTH * 16); bDecompTexShouldFree = true;
							rapi->Image_DecodePICA200ETC(decompTexData, passOffPointer, WIDTH, HEIGHT, b3DSAlpha, 0, 0);
							flip_vertically(decompTexData, WIDTH, HEIGHT, 4);
							passOffPointer = decompTexData;
						}
						else
						{
							decompTexData = (BYTE*)rapi->Noesis_UnpooledAlloc(WIDTH * HEIGHT * DEPTH * 4); bDecompTexShouldFree = true;
							rapi->Image_DecodeETC(decompTexData, passOffPointer, (int)currentImageSize, WIDTH, HEIGHT, &*(const char*)rawFormat.c_str());
							passOffPointer = decompTexData;
						}
						rawFormat = "r8g8b8a8";
						bitsPerPixel = 0x20;
						currentImageSize = (uint32_t)(WIDTH * HEIGHT * 4);
					}
					// Decompress DXT - 4x4
					// Format 		Block		 Bits		Notes
					//				Size (bytes) per Pixel			
					// ------------------------------------------			 
					// BC1(DXT1)	8			 4			RGB, optional 1 - bit alpha.Good for color maps and normal maps.
					// BC2(DXT3)	16			 8			RGB + 4 - bit explicit alpha. Generally less used.
					// BC3(DXT5)	16			 8			RGBA, common for textures with full alpha.
					// BC4			8			 4			Single - channel grayscale. Height maps, gloss maps, etc.
					// BC5			16			 8			Two - channel, can be used for normal maps.
					// BC6H			16			 8			RGB, High Dynamic Range(HDR) textures.
					// BC7			16			 8			RGB or RGBA, high - quality compression.
					else if (fourccFormat != -1)
					{
						if (bNormalized)
						{
							decompTexData = rapi->Noesis_ConvertDXT(WIDTH, HEIGHT, passOffPointer, fourccFormat); bDecompTexShouldFree = true;
							passOffPointer = decompTexData;
						}
						else
						{
							convertDxtExParams_t* params = (convertDxtExParams_t*)rapi->Noesis_UnpooledAlloc(sizeof(convertDxtExParams_t));
							params->ati2ZScale = texHeader.Z_SCALE;
							params->ati2NoNormalize = true;
							params->decodeAsSigned = bSigned;
							params->resvBB = false;
							params->resvBC = false;
							memset(params->resv, 0, 15 * sizeof(TResvInt));
							decompTexData = rapi->Noesis_ConvertDXTEx(WIDTH, HEIGHT, passOffPointer, fourccFormat, currentImageSize, params, 0); bDecompTexShouldFree = true;
							rapi->Noesis_UnpooledFree(params);
							passOffPointer = decompTexData;
						}
						rawFormat = "r8g8b8a8";
						bitsPerPixel = 0x20;
						currentImageSize = (uint32_t)(WIDTH * HEIGHT * 4);
					}
					// Need to find samples to test these
					//else if (bPalette4)
					//{
					//	bitsPerPixel = 0x20;
					//	decompTexData = (BYTE*)rapi->Noesis_UnpooledAlloc(WIDTH * HEIGHT * (int)(bitsPerPixel / 8));
					//	// need to find where the palette is
					//	ConvertP4_to_RGBA32(passOffPointer, PALETTE, decompTexData, WIDTH, HEIGHT);
					//	rawFormat = "r8g8b8a8";
					//	currentImageSize = (uint32_t)((WIDTH * HEIGHT * bitsPerPixel) / 8);
					//  passOffPointer = decompTexData;
					//}
					//else if (bPalette8)
					//{
					//	bitsPerPixel = 0x20;
					//	decompTexData = (BYTE*)rapi->Noesis_UnpooledAlloc(WIDTH * HEIGHT * (int)(bitsPerPixel / 8));
					//	// need to find where the palette is
					//	ConvertP8_to_RGBA32(passOffPointer, PALETTE, decompTexData, WIDTH, HEIGHT);
					//	rawFormat = "r8g8b8a8";
					//	currentImageSize = (uint32_t)((WIDTH * HEIGHT * bitsPerPixel) / 8);
					//  passOffPointer = decompTexData;
					//}
					//else if (bPalette14)
					//{
					//	bitsPerPixel = 0x20;
					//	decompTexData = (BYTE*)rapi->Noesis_UnpooledAlloc(WIDTH * HEIGHT * (int)(bitsPerPixel / 8));
					//	// need to find where the tlut is
					//	ConvertP14_to_RGBA32(passOffPointer, TLUT, decompTexData, WIDTH, HEIGHT);
					//	rawFormat = "r8g8b8a8";
					//	currentImageSize = (uint32_t)((WIDTH * HEIGHT * bitsPerPixel) / 8);
					//  passOffPointer = decompTexData;
					//}
				}
				

				///////////////////////////////
				//       POST CONVERTS       //
				///////////////////////////////

				BYTE* postConvertTexData = nullptr;
				bool bPostConvertTexShouldFree = false;

				// Alpha stored under image
				if (bHasAlphaAtlas)
				{
					// data should be r8g8b8a8 by now
					HEIGHT /= 2;
					uint32_t alphaOffset = WIDTH * HEIGHT * DEPTH * 4;
					for (uint32_t j = 0; j < WIDTH * HEIGHT; j++)
					{
						passOffPointer[4 * j + 3] = passOffPointer[alphaOffset + 4 * j];
					}
					bitsPerPixel = 0x20;
					rawFormat = "r8g8b8a8";
					currentImageSize = (uint32_t)((WIDTH * HEIGHT * bitsPerPixel) / 8);
				}

				// Depth & Stecil convert
				if (bD24_8Convert)
				{
					bitsPerPixel = 0x50;
					postConvertTexData = (BYTE*)rapi->Noesis_UnpooledAlloc(WIDTH * HEIGHT * (int)(bitsPerPixel / 8)); bPostConvertTexShouldFree = true;
					ConvertDepth24Stencil8_to_24_24_24_8(passOffPointer, postConvertTexData, WIDTH, HEIGHT);
					rawFormat = "r24g24b24a8";
					currentImageSize = (uint32_t)((WIDTH * HEIGHT * bitsPerPixel) / 8);
					passOffPointer = postConvertTexData;
				}
				else if (bD16Convert)
				{
					bitsPerPixel = 0x30;
					postConvertTexData = (BYTE*)rapi->Noesis_UnpooledAlloc(WIDTH * HEIGHT * (int)(bitsPerPixel / 8)); bPostConvertTexShouldFree = true;
					ConvertDepth16_to_16_16_16(passOffPointer, postConvertTexData, WIDTH, HEIGHT);
					rawFormat = "r16g16b16";
					currentImageSize = (uint32_t)((WIDTH * HEIGHT * bitsPerPixel) / 8);
					passOffPointer = postConvertTexData;
				}
				else if (bD32Convert)
				{
					bitsPerPixel = 0x60;
					postConvertTexData = (BYTE*)rapi->Noesis_UnpooledAlloc(WIDTH * HEIGHT * (int)(bitsPerPixel / 8)); bPostConvertTexShouldFree = true;
					ConvertDepth32_to_32_32_32(passOffPointer, postConvertTexData, WIDTH, HEIGHT);
					rawFormat = "r#F32g#F32b#F32";
					currentImageSize = (uint32_t)((WIDTH * HEIGHT * bitsPerPixel) / 8);
					passOffPointer = postConvertTexData;
				}
				// weird HDR bit float converts
				else if (bConvert11Bit10BitFloat)
				{
					bitsPerPixel = 0x60;
					postConvertTexData = (BYTE*)rapi->Noesis_UnpooledAlloc(WIDTH * HEIGHT * (int)(bitsPerPixel / 8)); bPostConvertTexShouldFree = true;
					ConvertR11FG11FB10F_to_RGB32F(passOffPointer, postConvertTexData, WIDTH, HEIGHT);
					rawFormat = "r#F32g#F32b#F32";
					currentImageSize = (uint32_t)((WIDTH * HEIGHT * bitsPerPixel) / 8);
					passOffPointer = postConvertTexData;
				}
				else if (bConvert10BitFloat)
				{
					bitsPerPixel = 0x80;
					postConvertTexData = (BYTE*)rapi->Noesis_UnpooledAlloc(WIDTH * HEIGHT * (int)(bitsPerPixel / 8)); bPostConvertTexShouldFree = true;
					ConvertU2F10F10F10_to_RGBA32F(passOffPointer, postConvertTexData, WIDTH, HEIGHT);
					rawFormat = "r#F32g#F32b#F32a#F32";
					currentImageSize = (uint32_t)((WIDTH * HEIGHT * bitsPerPixel) / 8);
					passOffPointer = postConvertTexData;
				}
				else if (bD32FloatConvert)
				{
					bitsPerPixel = 0x80;
					postConvertTexData = (BYTE*)rapi->Noesis_UnpooledAlloc(WIDTH * HEIGHT * (int)(bitsPerPixel / 8)); bPostConvertTexShouldFree = true;
					ConvertDepth32Stencil8_to_RGBA32f(passOffPointer, postConvertTexData, WIDTH, HEIGHT);
					rawFormat = "r#F32g#F32b#F32a#F32";
					currentImageSize = (uint32_t)((WIDTH * HEIGHT * bitsPerPixel) / 8);
					passOffPointer = postConvertTexData;
				}
				// signed to unsigned
				else if (bSigned)
				{
					if (rawFormat == "r16")
					{
						postConvertTexData = (BYTE*)rapi->Noesis_UnpooledAlloc(currentImageSize); bPostConvertTexShouldFree = true;
						ConvertR16I_to_R16(passOffPointer, postConvertTexData, WIDTH, HEIGHT);
						passOffPointer = postConvertTexData;
					}
					else if (rawFormat == "r32")
					{
						postConvertTexData = (BYTE*)rapi->Noesis_UnpooledAlloc(currentImageSize); bPostConvertTexShouldFree = true;
						ConvertR32I_to_R32UI(passOffPointer, postConvertTexData, WIDTH, HEIGHT);
						passOffPointer = postConvertTexData;
					}
				}

				///////////////////////////////
				//         HAND OFF          //
				///////////////////////////////

				std::string inFile = rapi->Noesis_GetLastCheckedName();
				std::filesystem::path pathObj(inFile);
				std::string fileName = pathObj.stem().string();

				// Decode the data
				BYTE* finalTexData = rapi->Noesis_ImageDecodeRaw(passOffPointer, (int)currentImageSize, WIDTH, HEIGHT, &rawFormat[0]);

				// Flip Vertically
				if (bFlipVertically)
				{
					flip_vertically(finalTexData, WIDTH, HEIGHT, (int)(bitsPerPixel / 8));
				}

				// Flip Horizontally
				if (bFlipHorizontally)
				{
					flip_horizontally(finalTexData, WIDTH, HEIGHT, (int)(bitsPerPixel / 8));
				}

				//Create the texture
				char texName[250];
				if (bPlanes)
				{
					fileName.append("out%d_%d.dds");
					const char* fileNameC = fileName.c_str();
					snprintf(texName, 250, fileNameC, i, p + 1);
				}
				else
				{
					fileName.append("out%d.dds");
					const char* fileNameC = fileName.c_str();
					snprintf(texName, 250, fileNameC, noeTex.Num());
				}
				noesisTex_t* texture = rapi->Noesis_TextureAlloc(texName, WIDTH, HEIGHT, finalTexData, NOESISTEX_RGBA32);
				texture->shouldFreeData = true;
				texture->globalIdx = noeTex.Num();
				noeTex.Append(texture);

				///////////////////////////////
				//      FREE POINTERS        //
				///////////////////////////////

				if (bPostConvertTexShouldFree)
				{
					// now check if it was used
					if (postConvertTexData)
					{
						rapi->Noesis_UnpooledFree(postConvertTexData);
					}
				}
				if (bDecompTexShouldFree)
				{
					// now check if it was used
					if (decompTexData)
					{
						rapi->Noesis_UnpooledFree(decompTexData);
					}
				}
				if (bUntiledTexShouldFree)
				{
					// now check if it was used
					if (untiledTexData)
					{
						rapi->Noesis_UnpooledFree(untiledTexData);
					}
				}

				// reset loop
				texStart += firstMipSize;
				currentImageSize = firstMipSize;
			}
		}
	}
};


#endif // !G1T_H