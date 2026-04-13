#include <Windows.h>
#include <intrin.h>
#include <array>
#include <iostream>
#include <vector>
#include <synchapi.h>
#include "Prototypes.h"
#include <winnt.h>


using AquireOrRelease = NTSTATUS(NTAPI*)();

typedef std::pair <PLIST_ENTRY, int> Pair;

struct Info {
	PLIST_ENTRY pListHead = nullptr, pListMod = nullptr;
	PEB_LDR_DATA* pLdr = nullptr;
	UNICODE_STRING* pModName = nullptr;
	PLDR_MODULE pLdrMod = NULL;
	uintptr_t PebAddr = NULL;
	AquireOrRelease PebLock;
	AquireOrRelease ReleasePebLock;
	char tModName[MAX_PATH];
	
} UnlinkData;

uintptr_t GetPEB() {
#ifdef _WIN64
	uintptr_t PebAddr = __readgsqword(0x60);
#else
	uintptr_t PebAddr = (uintptr_t)__readfsdword(0x30);
#endif
	return (PebAddr > 0) ? PebAddr : NULL;
}

// Offsets depuis chaque noeud de liste jusqu'au champ FullDllName dans LDR_MODULE.
// Calcules automatiquement via offsetof pour etre corrects sur x86 ET x64.
static const int InLoadOrderModuleList           = (int)(offsetof(LDR_MODULE, FullDllName) - offsetof(LDR_MODULE, InLoadOrderModuleList));
static const int InMemoryOrderModuleList         = (int)(offsetof(LDR_MODULE, FullDllName) - offsetof(LDR_MODULE, InMemoryOrderModuleList));
static const int InInitializationOrderModuleList = (int)(offsetof(LDR_MODULE, FullDllName) - offsetof(LDR_MODULE, InInitializationOrderModuleList));


void ClearListEntries(int EntryOffset, const char* ModName) {
	UnlinkData.PebLock();

	while (UnlinkData.pListMod->Flink != UnlinkData.pListHead) {
		UnlinkData.pListMod = UnlinkData.pListMod->Flink;
		UnlinkData.pModName = (UNICODE_STRING*)(((uintptr_t)(UnlinkData.pListMod)) + EntryOffset);

		WORD DllNameLength = (UnlinkData.pModName->Length) / 2; 

		int n = DllNameLength + 1;

		while (n--) {
			if (n != DllNameLength) {
				UnlinkData.tModName[n] = (CHAR)(*((UnlinkData.pModName->Buffer) +(n)));
				continue;

			}
			UnlinkData.tModName[n] = (char)'\000';
		}

		if (strstr(UnlinkData.tModName, ModName)) {
			if (EntryOffset == InLoadOrderModuleList) {
				UnlinkData.pLdrMod = (PLDR_MODULE)(((uintptr_t)(UnlinkData.pListMod)));

				UnlinkData.pLdrMod->HashTableEntry.Blink->Flink = UnlinkData.pLdrMod->HashTableEntry.Flink;
				UnlinkData.pLdrMod->HashTableEntry.Flink->Blink = UnlinkData.pLdrMod->HashTableEntry.Blink;
			}
			UnlinkData.pListMod->Blink->Flink = UnlinkData.pListMod->Flink;
			UnlinkData.pListMod->Flink->Blink = UnlinkData.pListMod->Blink;
		}
	}
	UnlinkData.ReleasePebLock();
}



void UnlinkModule(const char* TargetModName) {

	UnlinkData.PebLock = (AquireOrRelease)(GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlAcquirePebLock"));
	UnlinkData.ReleasePebLock = (AquireOrRelease)(GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlReleasePebLock"));


	UnlinkData.PebAddr = GetPEB();

#ifdef _WIN64
	UnlinkData.pLdr = *(PEB_LDR_DATA**)(UnlinkData.PebAddr + 0x18);
#else
	UnlinkData.pLdr = *(PEB_LDR_DATA**)(UnlinkData.PebAddr + 0x0C);
#endif

	std::vector <Pair> pairs;

	pairs.push_back(std::make_pair(&(UnlinkData.pLdr->InLoadOrderModuleList), InLoadOrderModuleList));
	pairs.push_back(std::make_pair(&(UnlinkData.pLdr->InMemoryOrderModuleList), InMemoryOrderModuleList));
	pairs.push_back(std::make_pair(&(UnlinkData.pLdr->InInitializationOrderModuleList), InInitializationOrderModuleList));

	for (std::vector<std::pair<PLIST_ENTRY, int>>::const_iterator iterator = pairs.begin(); iterator != pairs.end(); ++iterator)
	{
		UnlinkData.pListHead = UnlinkData.pListMod = iterator->first;
		ClearListEntries(iterator->second, TargetModName);
	}
}

 
DWORD WINAPI TestThread(HMODULE hModule) {

	Sleep(3000);

	UnlinkModule("exemple.dll");

	Sleep(4525223);
	return 0;
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		CreateThread(nullptr, NULL, (LPTHREAD_START_ROUTINE)TestThread, &hModule, NULL, nullptr);
		DisableThreadLibraryCalls(hModule);
		break;
	case DLL_PROCESS_DETACH:

		break;
	}

	return TRUE;
}



