/*
 * ============================================================
 * TF2LAUNCHER.C — Универсальный лаунчер
 * Установка + запуск TF2 + инжект чита
 * ============================================================
 */

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tlhelp32.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <time.h>

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "advapi32.lib")

// ============================================================
// 1. ПУТИ
// ============================================================

#define PROGRAM_DATA "C:\\ProgramData\\TF2CHEAT"
#define UTILS_FOLDER "C:\\ProgramData\\TF2CHEAT\\utils"
#define DLL_NAME "tf2cheat.dll"

char g_dll_path[MAX_PATH];
char g_current_exe[MAX_PATH];
bool g_silent = false;
bool g_hide = false;
bool g_debug = false;

// ============================================================
// 2. ЛОГИРОВАНИЕ
// ============================================================

void write_log(const char* msg, ...) {
    char log_path[MAX_PATH];
    strcpy(log_path, UTILS_FOLDER);
    strcat(log_path, "\\inject.log");
    
    char buffer[1024];
    va_list args;
    va_start(args, msg);
    vsprintf(buffer, msg, args);
    va_end(args);
    
    if (g_debug) {
        printf("[LOG] %s\n", buffer);
    }
    
    HANDLE file = CreateFileA(log_path, FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_HIDDEN, NULL);
    if (file != INVALID_HANDLE_VALUE) {
        DWORD written;
        WriteFile(file, buffer, strlen(buffer), &written, NULL);
        WriteFile(file, "\r\n", 2, &written, NULL);
        CloseHandle(file);
    }
}

// ============================================================
// 3. СОЗДАНИЕ ПАПОК
// ============================================================

bool create_directories() {
    write_log("[*] Creating directories...");
    
    if (!CreateDirectoryA(PROGRAM_DATA, NULL)) {
        if (GetLastError() != ERROR_ALREADY_EXISTS) {
            write_log("[!] Failed to create %s", PROGRAM_DATA);
            return false;
        }
    }
    SetFileAttributesA(PROGRAM_DATA, FILE_ATTRIBUTE_HIDDEN);
    
    if (!CreateDirectoryA(UTILS_FOLDER, NULL)) {
        if (GetLastError() != ERROR_ALREADY_EXISTS) {
            write_log("[!] Failed to create %s", UTILS_FOLDER);
            return false;
        }
    }
    SetFileAttributesA(UTILS_FOLDER, FILE_ATTRIBUTE_HIDDEN);
    
    write_log("[+] Directories created");
    return true;
}

// ============================================================
// 4. КОПИРОВАНИЕ DLL
// ============================================================

bool copy_dll() {
    write_log("[*] Looking for DLL...");
    
    char source_dll[MAX_PATH];
    
    // Проверяем в папке с лаунчером
    strcpy(source_dll, g_current_exe);
    char* last_slash = strrchr(source_dll, '\\');
    if (last_slash) {
        strcpy(last_slash + 1, DLL_NAME);
    }
    
    if (GetFileAttributesA(source_dll) == INVALID_FILE_ATTRIBUTES) {
        // Проверяем в текущей папке
        GetCurrentDirectoryA(MAX_PATH, source_dll);
        strcat(source_dll, "\\");
        strcat(source_dll, DLL_NAME);
    }
    
    if (GetFileAttributesA(source_dll) == INVALID_FILE_ATTRIBUTES) {
        write_log("[!] DLL not found");
        return false;
    }
    
    write_log("[+] Found DLL: %s", source_dll);
    
    if (!CopyFileA(source_dll, g_dll_path, FALSE)) {
        write_log("[!] Failed to copy DLL");
        return false;
    }
    
    SetFileAttributesA(g_dll_path, FILE_ATTRIBUTE_HIDDEN);
    write_log("[+] DLL installed: %s", g_dll_path);
    
    return true;
}

// ============================================================
// 5. СОЗДАНИЕ ЯРЛЫКА
// ============================================================

bool create_shortcut() {
    write_log("[*] Creating shortcut...");
    
    IShellLinkA* pShellLink = NULL;
    IPersistFile* pPersistFile = NULL;
    
    CoInitialize(NULL);
    
    if (FAILED(CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, &IID_IShellLinkA, (LPVOID*)&pShellLink))) {
        CoUninitialize();
        write_log("[!] Failed to create shell link");
        return false;
    }
    
    pShellLink->lpVtbl->SetPath(pShellLink, g_current_exe);
    pShellLink->lpVtbl->SetWorkingDirectory(pShellLink, PROGRAM_DATA);
    pShellLink->lpVtbl->SetDescription(pShellLink, "TF2 Cheat Launcher v2.0");
    pShellLink->lpVtbl->SetShowCmd(pShellLink, SW_HIDE);
    
    char desktop[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_DESKTOP, NULL, 0, desktop))) {
        strcat(desktop, "\\TF2Launcher.lnk");
        
        if (FAILED(pShellLink->lpVtbl->QueryInterface(pShellLink, &IID_IPersistFile, (LPVOID*)&pPersistFile))) {
            pShellLink->lpVtbl->Release(pShellLink);
            CoUninitialize();
            write_log("[!] Failed to get persist file");
            return false;
        }
        
        WCHAR wide_desktop[MAX_PATH];
        MultiByteToWideChar(CP_ACP, 0, desktop, -1, wide_desktop, MAX_PATH);
        pPersistFile->lpVtbl->Save(pPersistFile, wide_desktop, TRUE);
        pPersistFile->lpVtbl->Release(pPersistFile);
        
        write_log("[+] Shortcut created: %s", desktop);
    }
    
    pShellLink->lpVtbl->Release(pShellLink);
    CoUninitialize();
    return true;
}

// ============================================================
// 6. ПОИСК STEAM
// ============================================================

char* find_steam() {
    write_log("[*] Looking for Steam...");
    
    static char steam_path[MAX_PATH];
    DWORD size = MAX_PATH;
    HKEY hKey;
    
    // HKEY_CURRENT_USER
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Valve\\Steam", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueExA(hKey, "SteamPath", NULL, NULL, (LPBYTE)steam_path, &size) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            write_log("[+] Steam found: %s", steam_path);
            return steam_path;
        }
        RegCloseKey(hKey);
    }
    
    // HKEY_LOCAL_MACHINE
    size = MAX_PATH;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Valve\\Steam", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueExA(hKey, "InstallPath", NULL, NULL, (LPBYTE)steam_path, &size) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            write_log("[+] Steam found: %s", steam_path);
            return steam_path;
        }
        RegCloseKey(hKey);
    }
    
    // Стандартные пути
    const char* paths[] = {
        "C:\\Program Files (x86)\\Steam",
        "C:\\Program Files\\Steam",
        "D:\\Program Files (x86)\\Steam",
        "D:\\Program Files\\Steam"
    };
    
    for (int i = 0; i < 4; i++) {
        char test[MAX_PATH];
        sprintf(test, "%s\\steam.exe", paths[i]);
        if (GetFileAttributesA(test) != INVALID_FILE_ATTRIBUTES) {
            strcpy(steam_path, paths[i]);
            write_log("[+] Steam found: %s", steam_path);
            return steam_path;
        }
    }
    
    write_log("[!] Steam not found!");
    return NULL;
}

// ============================================================
// 7. ПОИСК ПРОЦЕССА
// ============================================================

DWORD find_process(const char* name) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return 0;
    
    PROCESSENTRY32 entry = { sizeof(PROCESSENTRY32) };
    DWORD pid = 0;
    
    if (Process32First(snap, &entry)) {
        do {
            if (strcmp(entry.szExeFile, name) == 0) {
                pid = entry.th32ProcessID;
                break;
            }
        } while (Process32Next(snap, &entry));
    }
    
    CloseHandle(snap);
    return pid;
}

// ============================================================
// 8. ИНЖЕКТ (3 метода)
// ============================================================

bool inject_loadlibrary(HANDLE process) {
    write_log("[*] Injecting via LoadLibraryA...");
    
    HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
    if (!kernel32) return false;
    
    LPVOID load_library = (LPVOID)GetProcAddress(kernel32, "LoadLibraryA");
    if (!load_library) return false;
    
    size_t path_len = strlen(g_dll_path) + 1;
    LPVOID remote_mem = VirtualAllocEx(process, NULL, path_len, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remote_mem) return false;
    
    if (!WriteProcessMemory(process, remote_mem, g_dll_path, path_len, NULL)) {
        VirtualFreeEx(process, remote_mem, 0, MEM_RELEASE);
        return false;
    }
    
    HANDLE thread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)load_library, remote_mem, 0, NULL);
    if (!thread) {
        VirtualFreeEx(process, remote_mem, 0, MEM_RELEASE);
        return false;
    }
    
    WaitForSingleObject(thread, 5000);
    CloseHandle(thread);
    VirtualFreeEx(process, remote_mem, 0, MEM_RELEASE);
    
    write_log("[+] Injection successful (LoadLibrary)");
    return true;
}

bool inject_apc(HANDLE process) {
    write_log("[*] Injecting via QueueUserAPC...");
    
    HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
    if (!kernel32) return false;
    
    LPVOID load_library = (LPVOID)GetProcAddress(kernel32, "LoadLibraryA");
    if (!load_library) return false;
    
    size_t path_len = strlen(g_dll_path) + 1;
    LPVOID remote_mem = VirtualAllocEx(process, NULL, path_len, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remote_mem) return false;
    
    if (!WriteProcessMemory(process, remote_mem, g_dll_path, path_len, NULL)) {
        VirtualFreeEx(process, remote_mem, 0, MEM_RELEASE);
        return false;
    }
    
    DWORD pid = GetProcessId(process);
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (snap == INVALID_HANDLE_VALUE) {
        VirtualFreeEx(process, remote_mem, 0, MEM_RELEASE);
        return false;
    }
    
    THREADENTRY32 entry = { sizeof(THREADENTRY32) };
    bool injected = false;
    
    if (Thread32First(snap, &entry)) {
        do {
            if (entry.th32OwnerProcessID == pid) {
                HANDLE thread = OpenThread(THREAD_ALL_ACCESS, FALSE, entry.th32ThreadID);
                if (thread) {
                    if (QueueUserAPC((PAPCFUNC)load_library, thread, (ULONG_PTR)remote_mem)) {
                        injected = true;
                        CloseHandle(thread);
                        break;
                    }
                    CloseHandle(thread);
                }
            }
        } while (Thread32Next(snap, &entry));
    }
    
    CloseHandle(snap);
    if (!injected) {
        VirtualFreeEx(process, remote_mem, 0, MEM_RELEASE);
        return false;
    }
    
    write_log("[+] Injection successful (APC)");
    return true;
}

bool inject_hook() {
    write_log("[*] Injecting via SetWindowsHookEx...");
    
    HWND hwnd = FindWindowA(NULL, "Team Fortress 2");
    if (!hwnd) {
        hwnd = FindWindowA(NULL, "Half-Life 2");
    }
    if (!hwnd) {
        write_log("[!] TF2 window not found");
        return false;
    }
    
    HMODULE dll = LoadLibraryA(g_dll_path);
    if (!dll) {
        write_log("[!] Failed to load DLL");
        return false;
    }
    
    FARPROC proc = GetProcAddress(dll, "DllMain");
    if (!proc) {
        proc = GetProcAddress(dll, "entry");
    }
    if (!proc) {
        write_log("[!] Failed to get entry point");
        FreeLibrary(dll);
        return false;
    }
    
    HHOOK hook = SetWindowsHookExA(WH_GETMESSAGE, (HOOKPROC)proc, dll, GetWindowThreadProcessId(hwnd, NULL));
    if (!hook) {
        write_log("[!] Failed to set hook");
        FreeLibrary(dll);
        return false;
    }
    
    PostMessageA(hwnd, WM_USER + 1, 0, 0);
    Sleep(1000);
    
    UnhookWindowsHookEx(hook);
    FreeLibrary(dll);
    
    write_log("[+] Injection successful (Hook)");
    return true;
}

// ============================================================
// 9. ОСТАНОВКА VAC
// ============================================================

void stop_vac() {
    write_log("[*] Stopping VAC...");
    
    DWORD pid = find_process("hl2.exe");
    if (pid == 0) pid = find_process("tf2.exe");
    if (pid == 0) {
        write_log("[!] TF2 not found, cannot stop VAC");
        return;
    }
    
    HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!process) {
        write_log("[!] Failed to open process for VAC");
        return;
    }
    
    // Ищем VAC модули
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
    if (snap == INVALID_HANDLE_VALUE) {
        CloseHandle(process);
        return;
    }
    
    MODULEENTRY32 entry = { sizeof(MODULEENTRY32) };
    if (Module32First(snap, &entry)) {
        do {
            if (strstr(entry.szModule, "vac") || strstr(entry.szModule, "VAC")) {
                write_log("[*] Found VAC: %s at 0x%p", entry.szModule, entry.modBaseAddr);
                DWORD old_protect;
                VirtualProtectEx(process, entry.modBaseAddr, entry.modBaseSize, PAGE_EXECUTE_READWRITE, &old_protect);
                ZeroMemory(entry.modBaseAddr, entry.modBaseSize);
                VirtualProtectEx(process, entry.modBaseAddr, entry.modBaseSize, old_protect, &old_protect);
                write_log("[+] VAC disabled");
            }
        } while (Module32Next(snap, &entry));
    }
    
    CloseHandle(snap);
    CloseHandle(process);
}

// ============================================================
// 10. ЗАПУСК TF2 И ИНЖЕКТ
// ============================================================

bool launch_and_inject() {
    char* steam = find_steam();
    if (!steam) {
        if (!g_silent) {
            MessageBoxA(NULL, "Steam not found!\nMake sure Steam is installed.", "Error", MB_OK | MB_ICONERROR);
        }
        return false;
    }
    
    // Запускаем TF2
    char cmd[MAX_PATH];
    sprintf(cmd, "\"%s\\steam.exe\" -applaunch 440", steam);
    
    write_log("[*] Launching TF2...");
    
    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    PROCESS_INFORMATION pi;
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    
    if (!CreateProcessA(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        write_log("[!] Failed to launch TF2");
        return false;
    }
    
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
    write_log("[+] TF2 launched");
    
    // Ждём процесс (до 30 секунд)
    DWORD pid = 0;
    for (int i = 0; i < 60; i++) {
        pid = find_process("hl2.exe");
        if (pid == 0) pid = find_process("tf2.exe");
        if (pid != 0) break;
        Sleep(500);
    }
    
    if (pid == 0) {
        write_log("[!] TF2 process not found");
        return false;
    }
    
    write_log("[+] TF2 process found (PID: %d)", pid);
    
    // Останавливаем VAC
    stop_vac();
    
    // Открываем процесс
    HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!process) {
        process = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, pid);
    }
    
    if (!process) {
        write_log("[!] Failed to open process");
        return false;
    }
    
    // Инжектим
    bool injected = false;
    injected = inject_loadlibrary(process);
    
    if (!injected) {
        injected = inject_apc(process);
    }
    
    CloseHandle(process);
    
    if (!injected) {
        injected = inject_hook();
    }
    
    if (injected) {
        write_log("[+] Cheat injected successfully!");
        return true;
    } else {
        write_log("[!] All injection methods failed!");
        return false;
    }
}

// ============================================================
// 11. УСТАНОВКА
// ============================================================

bool install() {
    write_log("========================================");
    write_log("[*] TF2 Cheat Installation");
    
    // Получаем путь к текущему EXE
    GetModuleFileNameA(NULL, g_current_exe, MAX_PATH);
    
    // Инициализация путей
    strcpy(g_dll_path, UTILS_FOLDER);
    strcat(g_dll_path, "\\");
    strcat(g_dll_path, DLL_NAME);
    
    // Создаём папки
    if (!create_directories()) {
        return false;
    }
    
    // Копируем DLL
    if (!copy_dll()) {
        return false;
    }
    
    // Создаём ярлык
    create_shortcut();
    
    write_log("[+] Installation complete!");
    return true;
}

// ============================================================
// 12. АРГУМЕНТЫ КОМАНДНОЙ СТРОКИ
// ============================================================

void parse_args(char* cmdline) {
    if (strstr(cmdline, "-silent") || strstr(cmdline, "-s")) {
        g_silent = true;
    }
    if (strstr(cmdline, "-hide") || strstr(cmdline, "-h")) {
        g_hide = true;
    }
    if (strstr(cmdline, "-debug") || strstr(cmdline, "-d")) {
        g_debug = true;
        AllocConsole();
        freopen("CONOUT$", "w", stdout);
        printf("=== TF2 Launcher Debug Mode ===\n");
    }
}

// ============================================================
// 13. ТОЧКА ВХОДА
// ============================================================

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Парсим аргументы
    parse_args(lpCmdLine);
    
    // Скрываем консоль
    if (g_hide) {
        FreeConsole();
        ShowWindow(GetConsoleWindow(), SW_HIDE);
    }
    
    // Путь к текущему EXE
    GetModuleFileNameA(NULL, g_current_exe, MAX_PATH);
    strcpy(g_dll_path, UTILS_FOLDER);
    strcat(g_dll_path, "\\");
    strcat(g_dll_path, DLL_NAME);
    
    // Проверяем, установлен ли чит
    bool installed = (GetFileAttributesA(g_dll_path) != INVALID_FILE_ATTRIBUTES);
    
    if (!installed) {
        write_log("[*] First launch - installing...");
        if (!install()) {
            if (!g_silent) {
                MessageBoxA(NULL, "Installation failed!\nMake sure tf2cheat.dll is in the same folder.", "Error", MB_OK | MB_ICONERROR);
            }
            return 1;
        }
        
        if (!g_silent) {
            MessageBoxA(NULL, 
                "TF2 Cheat installed successfully!\n\n"
                "Files installed to:\n"
                "C:\\ProgramData\\TF2CHEAT\\utils\\\n\n"
                "Launcher shortcut created on Desktop.\n\n"
                "Click OK to launch the game.",
                "Installation Complete",
                MB_OK | MB_ICONINFORMATION);
        }
    }
    
    // Запускаем TF2 и инжектим
    if (!launch_and_inject()) {
        if (!g_silent) {
            MessageBoxA(NULL, "Failed to launch/inject TF2!\nCheck the log file.", "Error", MB_OK | MB_ICONERROR);
        }
        return 1;
    }
    
    if (!g_silent && !g_hide) {
        MessageBoxA(NULL, "TF2 Launched with Cheat!\nPress OK to close this window.", "Success", MB_OK | MB_ICONINFORMATION);
    }
    
    return 0;
}

// ============================================================
// 14. ВХОД ДЛЯ КОНСОЛИ
// ============================================================

int main() {
    return WinMain(GetModuleHandle(NULL), NULL, GetCommandLineA(), SW_HIDE);
}

// ============================================================
// КОНЕЦ ФАЙЛА
// ============================================================