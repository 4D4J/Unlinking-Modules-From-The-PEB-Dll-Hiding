#include <windows.h>
#include <tlhelp32.h>
#include <iostream>

void PrintModules() {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, GetCurrentProcessId());
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        MODULEENTRY32 me32;
        me32.dwSize = sizeof(MODULEENTRY32);
        
        if (Module32First(hSnapshot, &me32)) {
            do {
                std::cout << " - " << me32.szModule << "\n";
            } while (Module32Next(hSnapshot, &me32));
        }
        CloseHandle(hSnapshot);
    } else {
        std::cout << "[-] Error: Unable to capture modules.\n";
    }
}

int main() {
    std::cout << "[*] List of modules before loading :\n";
    PrintModules();

    std::cout << "[*] Loading DLL 'exemple.dll'...\n";
    
    // Load the DLL into the current process
    HMODULE hMod = LoadLibraryA("exemple.dll"); 
    
    if (!hMod) {
        std::cout << "[-] Error: Unable to load the DLL.\n";
        std::cout << "    Make sure it is compiled as 'exemple.dll'\n";
        std::cout << "    and located in the same directory as this executable.\n";
        system("pause");
        return 1;
    }

    std::cout << "[+] DLL loaded successfully!\n";
    std::cout << "[*] List of modules IMMEDIATELY AFTER loading:\n";
    PrintModules();

    // Wait 4 seconds, as the DLL has a Sleep(3000) before performing the Unlinking
    std::cout << "[*] Waiting 4 seconds to allow the DLL to hide...\n";
    Sleep(4000);

    std::cout << "\n[*] List of modules AFTER Unlinking (the DLL should have disappeared):\n";
    PrintModules();

    std::cout << "[*] Test completed.\n";
    system("pause");
    return 0;
}
