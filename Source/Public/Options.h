#pragma once

#ifndef G1MOPT_H
#define G1MOPT_H

//bMerge
void getMerge(int handle)
{
	BYTE buffer[1];
	if (g_nfn->NPAPI_UserSettingRead(const_cast<wchar_t*>(L"g1m::merge"), buffer, 1))
	{
		bMerge = buffer[0] == 1;
	}
	g_nfn->NPAPI_CheckToolMenuItem(handle, bMerge);
}
int setMerge(int handle, void* userData)
{
	bMerge = !bMerge;
	BYTE buffer[1] = { bMerge };
	g_nfn->NPAPI_UserSettingWrite(const_cast<wchar_t*>(L"g1m::merge"), buffer, 1);
	g_nfn->NPAPI_CheckToolMenuItem(handle, bMerge);
	return 1;
}

//bMergeG1MOnly
void getMergeG1MOnly(int handle)
{
	BYTE buffer[1];
	if (g_nfn->NPAPI_UserSettingRead(const_cast<wchar_t*>(L"g1m::mergeG1MOnly"), buffer, 1))
	{
		bMergeG1MOnly = buffer[0] == 1;
	}
	g_nfn->NPAPI_CheckToolMenuItem(handle, bMergeG1MOnly);
}
int setMergeG1MOnly(int handle, void* userData)
{
	bMergeG1MOnly = !bMergeG1MOnly;
	BYTE buffer[1] = { bMergeG1MOnly };
	g_nfn->NPAPI_UserSettingWrite(const_cast<wchar_t*>(L"g1m::mergeG1MOnly"), buffer, 1);
	g_nfn->NPAPI_CheckToolMenuItem(handle, bMergeG1MOnly);
	return 1;
}

//bG1TMergeG1MOnly
void getG1TMergeG1MOnly(int handle)
{
	BYTE buffer[1];
	if (g_nfn->NPAPI_UserSettingRead(const_cast<wchar_t*>(L"g1m::G1TMergeG1MOnly"), buffer, 1))
	{
		bG1TMergeG1MOnly = buffer[0] == 1;
	}
	g_nfn->NPAPI_CheckToolMenuItem(handle, bG1TMergeG1MOnly);
}
int setG1TMergeG1MOnly(int handle, void* userData)
{
	bG1TMergeG1MOnly = !bG1TMergeG1MOnly;
	BYTE buffer[1] = { bG1TMergeG1MOnly };
	g_nfn->NPAPI_UserSettingWrite(const_cast<wchar_t*>(L"g1m::G1TMergeG1MOnly"), buffer, 1);
	g_nfn->NPAPI_CheckToolMenuItem(handle, bG1TMergeG1MOnly);
	return 1;
}

//bAnimations
void getAnimations(int handle)
{
	BYTE buffer[1];
	if (g_nfn->NPAPI_UserSettingRead(const_cast<wchar_t*>(L"g1m::animations"), buffer, 1))
	{
		bAnimations = buffer[0] == 1;
	}
	g_nfn->NPAPI_CheckToolMenuItem(handle, bAnimations);
}
int setAnimations(int handle, void* userData)
{
	bAnimations = !bAnimations;
	BYTE buffer[1] = { bAnimations };
	g_nfn->NPAPI_UserSettingWrite(const_cast<wchar_t*>(L"g1m::animations"), buffer, 1);
	g_nfn->NPAPI_CheckToolMenuItem(handle, bAnimations);
	return 1;
}

//bMatch
void getMatch(int handle)
{
	BYTE buffer[1];
	if (g_nfn->NPAPI_UserSettingRead(const_cast<wchar_t*>(L"g1m::match"), buffer, 1))
	{
		bMatch = buffer[0] == 1;
	}
	g_nfn->NPAPI_CheckToolMenuItem(handle, bMatch);
}
int setMatch(int handle, void* userData)
{
	bMatch = !bMatch;
	BYTE buffer[1] = { bMatch };
	g_nfn->NPAPI_UserSettingWrite(const_cast<wchar_t*>(L"g1m::match"), buffer, 1);
	g_nfn->NPAPI_CheckToolMenuItem(handle, bMatch);
	return 1;
}

//bAdditive
void getAdditive(int handle)
{
	BYTE buffer[1];
	if (g_nfn->NPAPI_UserSettingRead(const_cast<wchar_t*>(L"g1m::additive"), buffer, 1))
	{
		bAdditive = buffer[0] == 1;
	}
	g_nfn->NPAPI_CheckToolMenuItem(handle, bAdditive);
}
int setAdditive(int handle, void* userData)
{
	bAdditive = !bAdditive;
	BYTE buffer[1] = { bAdditive };
	g_nfn->NPAPI_UserSettingWrite(const_cast<wchar_t*>(L"g1m::additive"), buffer, 1);
	g_nfn->NPAPI_CheckToolMenuItem(handle, bAdditive);
	return 1;
}

//bColor
void getColor(int handle)
{
	BYTE buffer[1];
	if (g_nfn->NPAPI_UserSettingRead(const_cast<wchar_t*>(L"g1m::color"), buffer, 1))
	{
		bColor = buffer[0] == 1;
	}
	g_nfn->NPAPI_CheckToolMenuItem(handle, bColor);
}
int setColor(int handle, void* userData)
{
	bColor = !bColor;
	BYTE buffer[1] = { bColor };
	g_nfn->NPAPI_UserSettingWrite(const_cast<wchar_t*>(L"g1m::color"), buffer, 1);
	g_nfn->NPAPI_CheckToolMenuItem(handle, bColor);
	return 1;
}

//bDisplayDriver
void getDisplayDriver(int handle)
{
	BYTE buffer[1];
	if (g_nfn->NPAPI_UserSettingRead(const_cast<wchar_t*>(L"g1m::displayDriver"), buffer, 1))
	{
		bDisplayDriver = buffer[0] == 1;
	}
	g_nfn->NPAPI_CheckToolMenuItem(handle, bDisplayDriver);
}
int setDisplayDriver(int handle, void* userData)
{
	bDisplayDriver = !bDisplayDriver;
	BYTE buffer[1] = { bDisplayDriver };
	g_nfn->NPAPI_UserSettingWrite(const_cast<wchar_t*>(L"g1m::displayDriver"), buffer, 1);
	g_nfn->NPAPI_CheckToolMenuItem(handle, bDisplayDriver);
	return 1;
}

//bDisableNUNNodes
void getDisableNUNNodes(int handle)
{
	BYTE buffer[1];
	if (g_nfn->NPAPI_UserSettingRead(const_cast<wchar_t*>(L"g1m::disableNUN"), buffer, 1))
	{
		bDisableNUNNodes = buffer[0] == 1;
	}
	g_nfn->NPAPI_CheckToolMenuItem(handle, bDisableNUNNodes);
}
int setDisableNUNNodes(int handle, void* userData)
{
	bDisableNUNNodes = !bDisableNUNNodes;
	BYTE buffer[1] = { bDisableNUNNodes };
	g_nfn->NPAPI_UserSettingWrite(const_cast<wchar_t*>(L"g1m::disableNUN"), buffer, 1);
	g_nfn->NPAPI_CheckToolMenuItem(handle, bDisableNUNNodes);
	return 1;
}

//bNoTextureRename
void getNoTextureRename(int handle)
{
	BYTE buffer[1];
	if (g_nfn->NPAPI_UserSettingRead(const_cast<wchar_t*>(L"g1m::textureRename"), buffer, 1))
	{
		bNoTextureRename = buffer[0] == 1;
	}
	g_nfn->NPAPI_CheckToolMenuItem(handle, bNoTextureRename);
}
int setNoTextureRename(int handle, void* userData)
{
	bNoTextureRename = !bNoTextureRename;
	BYTE buffer[1] = { bNoTextureRename };
	g_nfn->NPAPI_UserSettingWrite(const_cast<wchar_t*>(L"g1m::textureRename"), buffer, 1);
	g_nfn->NPAPI_CheckToolMenuItem(handle, bNoTextureRename);
	return 1;
}

//bDisableNUNNodes
void getEnableNUNAutoRig(int handle)
{
	BYTE buffer[1];
	if (g_nfn->NPAPI_UserSettingRead(const_cast<wchar_t*>(L"g1m::enableNUNAutoRig"), buffer, 1))
	{
		bEnableNUNAutoRig = buffer[0] == 1;
	}
	g_nfn->NPAPI_CheckToolMenuItem(handle, bEnableNUNAutoRig);
}
int setEnableNUNAutoRig(int handle, void* userData)
{
	bEnableNUNAutoRig = !bEnableNUNAutoRig;
	BYTE buffer[1] = { bEnableNUNAutoRig };
	g_nfn->NPAPI_UserSettingWrite(const_cast<wchar_t*>(L"g1m::enableNUNAutoRig"), buffer, 1);
	g_nfn->NPAPI_CheckToolMenuItem(handle, bEnableNUNAutoRig);
	return 1;
}

//bLoadAllLODs
void getEnableLOD(int handle)
{
	BYTE buffer[1];
	if (g_nfn->NPAPI_UserSettingRead(const_cast<wchar_t*>(L"g1m::enableLOD"), buffer, 1))
	{
		bLoadAllLODs = buffer[0] == 1;
	}
	g_nfn->NPAPI_CheckToolMenuItem(handle, bLoadAllLODs);
}
int setEnableLOD(int handle, void* userData)
{
	bLoadAllLODs = !bLoadAllLODs;
	BYTE buffer[1] = { bLoadAllLODs };
	g_nfn->NPAPI_UserSettingWrite(const_cast<wchar_t*>(L"g1m::enableLOD"), buffer, 1);
	g_nfn->NPAPI_CheckToolMenuItem(handle, bLoadAllLODs);
	return 1;
}

//bPlanes
void getLayers(int handle)
{
	BYTE buffer[1];
	if (g_nfn->NPAPI_UserSettingRead(const_cast<wchar_t*>(L"g1m::planes"), buffer, 1))
	{
		bLayers = buffer[0] == 1;
	}
	g_nfn->NPAPI_CheckToolMenuItem(handle, bLayers);
}
int setLayers(int handle, void* userData)
{
	bLayers = !bLayers;
	BYTE buffer[1] = { bLayers };
	g_nfn->NPAPI_UserSettingWrite(const_cast<wchar_t*>(L"g1m::planes"), buffer, 1);
	g_nfn->NPAPI_CheckToolMenuItem(handle, bLayers);
	return 1;
}

//bFlipVertically
void getFlipVertically(int handle)
{
	BYTE buffer[1];
	if (g_nfn->NPAPI_UserSettingRead(const_cast<wchar_t*>(L"g1m::flipVertically"), buffer, 1))
	{
		bFlipVertically = buffer[0] == 1;
	}
	g_nfn->NPAPI_CheckToolMenuItem(handle, bFlipVertically);
}
int setFlipVertically(int handle, void* userData)
{
	bFlipVertically = !bFlipVertically;
	BYTE buffer[1] = { bFlipVertically };
	g_nfn->NPAPI_UserSettingWrite(const_cast<wchar_t*>(L"g1m::flipVertically"), buffer, 1);
	g_nfn->NPAPI_CheckToolMenuItem(handle, bFlipVertically);
	return 1;
}

//bFlipHorizontally
void getFlipHorizontally(int handle)
{
	BYTE buffer[1];
	if (g_nfn->NPAPI_UserSettingRead(const_cast<wchar_t*>(L"g1m::flipHorizontally"), buffer, 1))
	{
		bFlipHorizontally = buffer[0] == 1;
	}
	g_nfn->NPAPI_CheckToolMenuItem(handle, bFlipHorizontally);
}
int setFlipHorizontally(int handle, void* userData)
{
	bFlipHorizontally = !bFlipHorizontally;
	BYTE buffer[1] = { bFlipHorizontally };
	g_nfn->NPAPI_UserSettingWrite(const_cast<wchar_t*>(L"g1m::flipHorizontally"), buffer, 1);
	g_nfn->NPAPI_CheckToolMenuItem(handle, bFlipHorizontally);
	return 1;
}

//bG1EMSplitMeshes
void getG1EMSplitMeshes(int handle)
{
	BYTE buffer[1];
	if (g_nfn->NPAPI_UserSettingRead(const_cast<wchar_t*>(L"g1m::G1EMSplitMeshes"), buffer, 1))
	{
		bG1EMSplitMeshes = buffer[0] == 1;
	}
	g_nfn->NPAPI_CheckToolMenuItem(handle, bG1EMSplitMeshes);
}
int setG1EMSplitMeshes(int handle, void* userData)
{
	bG1EMSplitMeshes = !bG1EMSplitMeshes;
	BYTE buffer[1] = { bG1EMSplitMeshes };
	g_nfn->NPAPI_UserSettingWrite(const_cast<wchar_t*>(L"g1m::G1EMSplitMeshes"), buffer, 1);
	g_nfn->NPAPI_CheckToolMenuItem(handle, bG1EMSplitMeshes);
	return 1;
}

//bDebugLog
void getDebugLog(int handle)
{
	BYTE buffer[1];
	if (g_nfn->NPAPI_UserSettingRead(const_cast<wchar_t*>(L"g1m::debug"), buffer, 1))
	{
		bDebugLog = buffer[0] == 1;
	}
	g_nfn->NPAPI_CheckToolMenuItem(handle, bDebugLog);
	if (bDebugLog && !g_nfn->NPAPI_DebugLogIsOpen()) g_nfn->NPAPI_PopupDebugLog(0);
	if (bDebugLog) 
	{
		LogDebug("G1M v%s debugging enabled.\n", PLUGIN_VERSON);
	}
}
int setDebugLog(int handle, void* userData)
{
	bDebugLog = !bDebugLog;
	BYTE buffer[1] = { bDebugLog };
	g_nfn->NPAPI_UserSettingWrite(const_cast<wchar_t*>(L"g1m::debug"), buffer, 1);
	g_nfn->NPAPI_CheckToolMenuItem(handle, bDebugLog);
	if (bDebugLog) 
	{
		if (!g_nfn->NPAPI_DebugLogIsOpen())
		{
			g_nfn->NPAPI_PopupDebugLog(0);
		}
		LogDebug("G1M v%s debugging enabled.\n", PLUGIN_VERSON);
	}
	else 
	{
		LogDebug("G1M v%s debugging disabled.\n", PLUGIN_VERSON);
	}
	return 1;
}

//g1t console command
bool g1tConsoleHandler(const char* arg, unsigned char* store, int storeSize)
{
	if (std::filesystem::exists(std::filesystem::path(std::string(arg))) && (std::filesystem::path(std::string(arg)).extension() == ".g1t"))
	{
		memcpy(g1tConsolePath, arg, storeSize);
		return true;
	}
	else
	{
		strcpy_s(g1tConsolePath, "");
		return false;
	}

}

void g1tConsoleReset(unsigned char* store, int storeSize)
{
	*store = 1;
	strcpy_s(g1tConsolePath, "");
}

#endif // !G1MOPT_H

