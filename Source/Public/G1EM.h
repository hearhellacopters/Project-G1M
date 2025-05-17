#pragma once

#ifndef G1EM_H
#define G1EM_H

///////////////////////////////
//     HELPER FUNCTIONS      //
///////////////////////////////

rpgeoDataType_e G1EM_DATATYPES(int value)
{
	switch (value)
	{
	case 0x01: // guessing here
	case 0x02:
	case 0x03:
		return rpgeoDataType_e::RPGEODATA_FLOAT;
	case 0x05:
	case 0x0D:
		return rpgeoDataType_e::RPGEODATA_UBYTE;
	case 0x07:
		return rpgeoDataType_e::RPGEODATA_USHORT;
	case 0x0A:
	case 0x0B:
		return rpgeoDataType_e::RPGEODATA_HALFFLOAT;
	default:
		return rpgeoDataType_e::NUM_RPGEO_DATATYPES;
		break;
	}
};

///////////////////////////////
//      HEADER STRUCTS       //
///////////////////////////////

template <bool bBigEndian>
struct G1EM_HEADER
{
	char MAGIC[4] = {};
	uint32_t VERSION = 0;
	uint32_t VERSION_NUMBER = 0;
	uint32_t HEADER_SIZE = 16;
	uint16_t MODEL_COUNT = 0;
	uint16_t MESH_COUNT = 0;
	bool bPanic = false;
	G1EM_HEADER(BYTE* buffer, uint32_t& offset, int bufferLen)
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

		const char EXPECTED_BIG_MAGIC[4] = { 'G', '1', 'E', 'M' };
		const char EXPECTED_LITTLE_MAGIC[4] = { 'M', 'E', '1', 'G' };

		// panic check
		if (!(memcmp(MAGIC, EXPECTED_BIG_MAGIC, 4) == 0 ||
			memcmp(MAGIC, EXPECTED_LITTLE_MAGIC, 4) == 0)
			)
		{
			bPanic = true;
			return;
		}

		VERSION = (*(uint32_t*)(buffer + offset)); offset += 4; if (bBigEndian) LITTLE_BIG_SWAP(VERSION);

		// get the version number
		VERSION_NUMBER =
			(((VERSION >> 24) & 0xFF) - 0x30) * 1000 +
			(((VERSION >> 16) & 0xFF) - 0x30) * 100 +
			(((VERSION >> 8) & 0xFF) - 0x30) * 10 +
			(VERSION & 0xFF) - 0x30;

		HEADER_SIZE = (*(uint32_t*)(buffer + offset)); offset += 4; if (bBigEndian) LITTLE_BIG_SWAP(HEADER_SIZE);

		// panic check
		if (HEADER_SIZE > (uint32_t)bufferLen)
		{
			bPanic = true;
			return;
		}

		MODEL_COUNT = (*(uint16_t*)(buffer + offset)); offset += 2; if (bBigEndian) LITTLE_BIG_SWAP(MODEL_COUNT);

		MESH_COUNT = (*(uint16_t*)(buffer + offset)); offset += 2; if (bBigEndian) LITTLE_BIG_SWAP(MESH_COUNT);
	}
};

template <bool bBigEndian>
struct G1EM_MESH_ENTRY
{
	int modelID;
	int primType;
	int vOffset;
	int vCount;
	int idxOffset;
	int idxCount;
	G1EM_MESH_ENTRY(G1EM_MESH_ENTRY* ptr) : G1EM_MESH_ENTRY(*ptr)
	{
		if (bBigEndian)
		{
			LITTLE_BIG_SWAP(modelID);
			LITTLE_BIG_SWAP(primType);
			LITTLE_BIG_SWAP(vOffset);
			LITTLE_BIG_SWAP(vCount);
			LITTLE_BIG_SWAP(idxOffset);
			LITTLE_BIG_SWAP(idxCount);
		}
	}
};

template <bool bBigEndian>
struct G1EM_UNK_ENTRY
{
	uint16_t Unknown0;
	uint16_t Unknown1;
	uint16_t Unknown2;
	uint16_t Unknown3;
	uint16_t Unknown4;
	uint16_t Unknown5;
	uint16_t Unknown6;
	uint16_t Unknown7;
	uint16_t Unknown8;
	uint16_t Unknown9;
	G1EM_UNK_ENTRY(G1EM_UNK_ENTRY* ptr) : G1EM_UNK_ENTRY(*ptr)
	{
		if (bBigEndian)
		{
			LITTLE_BIG_SWAP(Unknown0);
			LITTLE_BIG_SWAP(Unknown1);
			LITTLE_BIG_SWAP(Unknown2);
			LITTLE_BIG_SWAP(Unknown3);
			LITTLE_BIG_SWAP(Unknown4);
			LITTLE_BIG_SWAP(Unknown5);
			LITTLE_BIG_SWAP(Unknown6);
			LITTLE_BIG_SWAP(Unknown7);
			LITTLE_BIG_SWAP(Unknown8);
			LITTLE_BIG_SWAP(Unknown9);
		}
	}
};

template <bool bBigEndian>
struct G1EM_SEMANTIC_ENTRY
{
	uint16_t bufferID;
	uint16_t offset;
	uint16_t dataType;
	uint16_t semantic;
	G1EM_SEMANTIC_ENTRY(G1EM_SEMANTIC_ENTRY* ptr) : G1EM_SEMANTIC_ENTRY(*ptr)
	{
		if (bBigEndian)
		{
			LITTLE_BIG_SWAP(bufferID);
			LITTLE_BIG_SWAP(offset);
			LITTLE_BIG_SWAP(dataType);
			LITTLE_BIG_SWAP(semantic);
		}
	}
};

template <bool bBigEndian>
struct G1EM_MODEL_ENTRY
{
	int semanticCount = 0;
	std::vector<G1EM_SEMANTIC_ENTRY<bBigEndian>> SEMANTIC_ENTRIES;
	int vertexCount = 0;
	int vertexStride = 0;
	int idxCount = 0;
	int unk = 0;
	BYTE* vBuffer;   // unsure if these need endian switch
	BYTE* idxBuffer; // unsure if these need endian switch
	int ENTRY_SIZE = 0;
	G1EM_MODEL_ENTRY(BYTE* buffer)
	{
		semanticCount = *(int*)(buffer + ENTRY_SIZE); ENTRY_SIZE += 4; if (bBigEndian) LITTLE_BIG_SWAP(semanticCount);
		for (size_t i = 0; i < semanticCount; i++)
		{
			G1EM_SEMANTIC_ENTRY<bBigEndian> entry = reinterpret_cast<G1EM_SEMANTIC_ENTRY<bBigEndian>*>(buffer + ENTRY_SIZE); ENTRY_SIZE += 8;
			SEMANTIC_ENTRIES.push_back(entry);
		}
		vertexCount = *(int*)(buffer + ENTRY_SIZE); ENTRY_SIZE += 4; if (bBigEndian) LITTLE_BIG_SWAP(vertexCount);
		vertexStride = *(int*)(buffer + ENTRY_SIZE); ENTRY_SIZE += 4; if (bBigEndian) LITTLE_BIG_SWAP(vertexStride);
		idxCount = *(int*)(buffer + ENTRY_SIZE); ENTRY_SIZE += 4; if (bBigEndian) LITTLE_BIG_SWAP(idxCount);
		unk = *(int*)(buffer + ENTRY_SIZE); ENTRY_SIZE += 4; if (bBigEndian) LITTLE_BIG_SWAP(unk);
		vBuffer = buffer + ENTRY_SIZE; ENTRY_SIZE += (vertexCount * vertexStride);
		//for (size_t i = 0; i < ((vertexCount * vertexStride) / 2); i++)
		//{
		//	uint16_t shortRead = *(uint16_t*)(buffer + ENTRY_SIZE); ENTRY_SIZE += 2; if (bBigEndian) LITTLE_BIG_SWAP(shortRead);
		//	vBuffer.push_back(shortRead);
		//}
		idxBuffer = buffer + ENTRY_SIZE; ENTRY_SIZE += idxCount * 2;
		//for (size_t i = 0; i < idxCount; i++)
		//{
		//	uint16_t shortRead = *(uint16_t*)(buffer + ENTRY_SIZE); ENTRY_SIZE += 2; if (bBigEndian) LITTLE_BIG_SWAP(shortRead);
		//	idxBuffer.push_back(shortRead);
		//}
	}
};

///////////////////////////////
//      MAIN FUNCTION        //
///////////////////////////////

template <bool bBigEndian>
struct G1EM_MODEL
{
	uint16_t MODEL_COUNT = 0;
	uint16_t MESH_COUNT = 0;
	std::vector<G1EM_MODEL_ENTRY<bBigEndian>> MODEL_ENTRIES;
	std::vector<G1EM_MESH_ENTRY<bBigEndian>> MESH_ENTRIES;
	std::vector<G1EM_UNK_ENTRY<bBigEndian>> UNK_ENTRIES;
	G1EM_MODEL(BYTE* buffer, uint32_t& offset,  int bufferLen)
	{

		///////////////////////////////
		//   READ BASIC HEADER INFO  //
		///////////////////////////////
		
		// Panic check on header size
		if (bufferLen < 16)
		{
			PopUpMessage(L"G1EM file too small!\nRestart app with 'Enable debug log' on in 'Tools/Project G1M' menu for info.");
			assert(0 && "G1EM too small to be a file!");
		}

		G1EM_HEADER<bBigEndian> header = G1EM_HEADER<bBigEndian>(buffer, offset, bufferLen);

		MODEL_COUNT = header.MODEL_COUNT;
		MESH_COUNT = header.MESH_COUNT;

		// Panic check on header read
		if (header.bPanic)
		{
			PopUpMessage(L"G1EM file header read error!\nRestart app with 'Enable debug log' on in 'Tools/Project G1M' menu for info.");
			assert(0 && "G1EM file header read error!");
		}

		for (size_t i = 0; i < MODEL_COUNT; i++)
		{
			G1EM_MODEL_ENTRY<bBigEndian> modelEntry = G1EM_MODEL_ENTRY<bBigEndian>(buffer + offset); offset += modelEntry.ENTRY_SIZE;
			MODEL_ENTRIES.push_back(modelEntry); 
		}

		for (size_t i = 0; i < header.MESH_COUNT; i++)
		{
			G1EM_MESH_ENTRY<bBigEndian> meshEntry = reinterpret_cast<G1EM_MESH_ENTRY<bBigEndian>*>(buffer + offset); offset += 0x18;
			MESH_ENTRIES.push_back(meshEntry);
		}

		for (size_t i = 0; i < header.MESH_COUNT; i++)
		{
			G1EM_UNK_ENTRY<bBigEndian> unkEntry = reinterpret_cast<G1EM_UNK_ENTRY<bBigEndian>*>(buffer + offset); offset += 0x14;
			UNK_ENTRIES.push_back(unkEntry);
		}
	}
};

#endif // !G1EM_H