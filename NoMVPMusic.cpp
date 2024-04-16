/**
 * =============================================================================
 * Movement Unlocker
 * Copyright (C) 2024 Source2ZE
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include "NoMVPMusic.h"
#include <sh_memory.h>
#ifdef _WIN32
#include <Windows.h>
#elif __linux__
#include <dlfcn.h>
#endif

NoMVPMusic g_NoMVPMusic;

#ifdef _WIN32
const unsigned char *pPatchSignature = (unsigned char *)"\x48\x85\xC0\x74\x2A\x65\x48\x8B\x0C\x25";
const char *pPatchPattern = "xxxx?xxxxx";
int PatchLen = 1;
#elif __linux__
const unsigned char * pPatchSignature = (unsigned char *)"\x48\x85\xC0\x0F\x84\x2A\x2A\x2A\x2A\x0F\xB6\x05\x2A\x2A\x2A\x2A\x84\xC0\x0F\x84\x2A\x2A\x2A\x2A\x41\x80\x7E\x2A\x00";
const char* pPatchPattern = "xxxxx????xxx????xxxx????xxx?x";
int PatchLen = 99;
#endif

// From https://git.botox.bz/CSSZombieEscape/sm-ext-PhysHooks
uintptr_t FindPattern(uintptr_t BaseAddr, const unsigned char* pData, const char* pPattern, size_t MaxSize, bool Reverse)
{
	unsigned char* pMemory;
	uintptr_t PatternLen = strlen(pPattern);

	pMemory = reinterpret_cast<unsigned char*>(BaseAddr);

	if (!Reverse)
	{
		for (uintptr_t i = 0; i < MaxSize; i++)
		{
			uintptr_t Matches = 0;
			while (*(pMemory + i + Matches) == pData[Matches] || pPattern[Matches] != 'x')
			{
				Matches++;
				if (Matches == PatternLen)
					return (uintptr_t)(pMemory + i);
			}
		}
	}
	else
	{
		for (uintptr_t i = 0; i < MaxSize; i++)
		{
			uintptr_t Matches = 0;
			while (*(pMemory - i + Matches) == pData[Matches] || pPattern[Matches] != 'x')
			{
				Matches++;
				if (Matches == PatternLen)
					return (uintptr_t)(pMemory - i);
			}
		}
	}

	return 0x00;
}

PLUGIN_EXPOSE(NoMVPMusic, g_NoMVPMusic);
bool NoMVPMusic::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();

	char pBinPath[MAX_PATH];
#ifdef _WIN32
	V_snprintf(pBinPath, MAX_PATH, "%s%s", Plat_GetGameDirectory(), "/csgo/bin/win64/server.dll");
	auto *pBin = LoadLibrary(pBinPath);
#elif __linux__
	V_snprintf(pBinPath, MAX_PATH, "%s%s", Plat_GetGameDirectory(), "/csgo/bin/linuxsteamrt64/libserver.so");
	auto *pBin = dlopen(pBinPath, RTLD_NOW);
#endif

	if (!pBin)
	{
		snprintf(error, maxlen, "Could not open %s", pBinPath);
		return false;
	}

#ifdef _WIN32
	uintptr_t pPatchAddress = (uintptr_t)GetProcAddress(pBin, "CreateInterface");
#elif __linux__
	uintptr_t pPatchAddress = (uintptr_t)dlsym(pBin, "CreateInterface");
#endif

	pPatchAddress = FindPattern(pPatchAddress, pPatchSignature, pPatchPattern, ULLONG_MAX, true);

	if (!pPatchAddress)
	{
		snprintf(error, maxlen, "Could not find NoMVPMusic patch signature!");
		return false;
	}

	SourceHook::SetMemAccess((void*)pPatchAddress, PatchLen, SH_MEM_READ | SH_MEM_WRITE | SH_MEM_EXEC);

#ifdef _WIN32
	*(unsigned char*)(pPatchAddress) = ((unsigned char*)"\xEB")[0];
#elif __linux__
	for (int i = 0; i < PatchLen; i++)
        *(unsigned char*)(pPatchAddress + i) = ((unsigned char*)"\x90")[0];
#endif

	SourceHook::SetMemAccess((void*)pPatchAddress, PatchLen, SH_MEM_READ | SH_MEM_EXEC);
	META_CONPRINTF( "[NoMVPMusic] Successfully patched NoMVPMusic!\n" );

	return true;
}

bool NoMVPMusic::Unload(char *error, size_t maxlen)
{
	return true;
}

void NoMVPMusic::AllPluginsLoaded()
{
}

bool NoMVPMusic::Pause(char *error, size_t maxlen)
{
	return true;
}

bool NoMVPMusic::Unpause(char *error, size_t maxlen)
{
	return true;
}

const char *NoMVPMusic::GetLicense()
{
	return "GNU General Public License v3.0";
}

const char *NoMVPMusic::GetVersion()
{
	return "1.0";
}

const char *NoMVPMusic::GetDate()
{
	return __DATE__;
}

const char *NoMVPMusic::GetLogTag()
{
	return "NoMVPMusic";
}

const char *NoMVPMusic::GetAuthor()
{
	return "Vauff";
}

const char *NoMVPMusic::GetDescription()
{
	return "NoMVPMusic";
}

const char *NoMVPMusic::GetName()
{
	return "NoMVPMusic";
}

const char *NoMVPMusic::GetURL()
{
	return "";
}
