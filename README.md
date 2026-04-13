# PEB Module Unlinking — DLL Hiding

A C++ proof-of-concept demonstrating the **PEB Unlinking** technique: hiding a loaded DLL from userland module enumeration by removing it from the `PEB_LDR_DATA` linked lists at runtime.

## How it works

When Windows loads a DLL into a process, it registers it in three doubly-linked lists stored inside the **Process Environment Block (PEB)**:

- `InLoadOrderModuleList`
- `InMemoryOrderModuleList`
- `InInitializationOrderModuleList`

By unlinking a module's `LIST_ENTRY` nodes from all three lists, and from the internal hash table, the DLL becomes **invisible to any userland API** that enumerates modules (e.g. `CreateToolhelp32Snapshot`, `EnumProcessModules`, `psapi`).

The DLL remains fully loaded and executing in memory — it is simply no longer listed.

## Project structure

```
├── Source.cpp       # The DLL — implements PEB unlinking on itself at runtime
├── Prototypes.h     # PEB/LDR_MODULE structure definitions
├── TestApp.cpp      # Standalone EXE to load the DLL and verify it disappears
├── CMakeLists.txt   # Build system
└── README.md
```

## Building

Requires **MSVC** (Visual Studio) on Windows. The inline PEB access uses MSVC intrinsics (`__readgsqword` / `__readfsdword`) and is compatible with both **x64** and **x86**.

```bat
mkdir build
cd build
cmake .. 
cmake --build . --config Release
```

Binaries are output to `build/bin/`. The DLL is named `exemple.dll` and the test executable is `TestApp.exe`.

## Testing

Run `TestApp.exe`. It will:

1. List all loaded modules before injecting the DLL
2. Load the DLL (`LoadLibraryA`) — it appears in the list
3. Wait 4 seconds while the DLL hides itself (the DLL has a 3-second delay before unlinking)
4. List modules again — the DLL is gone from userland enumeration

## Limitations

This technique only affects **userland** module enumeration. It does **not** hide the DLL from:

- Kernel-level tools (Process Explorer, WinObj, etc.) which query `NtQuerySystemInformation` or Virtual Address Descriptors (VADs)
- Kernel-mode anti-cheats (BattlEye, Easy Anti-Cheat, etc.) operating at ring-0

It is effective only against basic userland enumeration via Win32 APIs.

## References

- [Inside Windows — PEB & LDR structures](https://en.wikipedia.org/wiki/Process_Environment_Block)
- [Hiding DLLs from Process Lists — ired.team](https://www.ired.team/offensive-security/defense-evasion/unlinking-dlls-from-the-peb)
