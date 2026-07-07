/*
 * ============================================================
 * SILENT_INJECTOR.C — TF2 Cheat Silent Injector
 * Без окон, без консолей, без следов
 * Всё делает молча и незаметно
 * ============================================================
 */

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <time.h>

#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "advapi32.lib")

// ============================================================
// 1. ПЕРЕМЕННЫЕ
// ============================================================

char g_dll_path[MAX_PATH];
char g_log_path[MAX_PATH];
bool g_silent = true;
bool g_log = false;

// ============================================================
// 2. ЛОГИРОВАНИЕ (Только в файл, без консоли)
// ============================================================

void write_log(const char* msg, ...) {
    if (!g_log) return;
    
    char buffer[1024];
    va_list args;
    va_start(args, msg);
    vsprintf(buffer, msg, args);
    va_end(args);
    
    HANDLE file = CreateFileA(g_log_path, FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_HIDDEN, NULL);
    if (file != INVALID_HANDLE_VALUE) {
        DWORD written;
        WriteFile(file, buffer, strlen(buffer), &written, NULL);
        WriteFile(file, "\r\n", 2, &written, NULL);
        CloseHandle(file);
    }
}

// ============================================================
// 3. ИНИЦИАЛИЗАЦИЯ
// ============================================================

void init_paths() {
    // Получаем путь к DLL (лежит рядом с инжектором)
    GetModuleFileNameA(NULL, g_dll_path, MAX_PATH);
    char* last_slash = strrchr(g_dll_path, '\\');
    if (last_slash) {
        strcpy(last_slash + 1, "tf2cheat.dll");
    }
    
    // Путь к логу (в AppData, чтобы не светиться)
    GetEnvironmentVariableA("APPDATA", g_log_path, MAX_PATH);
    strcat(g_log_path, "\\TF2Cheat\\inject.log");
    
    // Создаём папку, если её нет
    char folder[MAX_PATH];
    strcpy(folder, g_log_path);
    char* last = strrchr(folder, '\\');
    if (last) {
        *last = 0;
        CreateDirectoryA(folder, NULL);
        SetFileAttributesA(folder, FILE_ATTRIBUTE_HIDDEN);
    }
    
    // Скрываем файл лога
    SetFileAttributesA(g_log_path, FILE_ATTRIBUTE_HIDDEN);
}

// ============================================================
// 4. ПОИСК ПРОЦЕССА
// ============================================================

DWORD find_process(const char* process_name) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        write_log("[!] Failed to create process snapshot");
        return 0;
    }
    
    PROCESSENTRY32 entry = { sizeof(PROCESSENTRY32) };
    DWORD pid = 0;
    int attempts = 0;
    
    // Ждём запуска процесса (до 10 секунд)
    while (pid == 0 && attempts < 20) {
        if (Process32First(snapshot, &entry)) {
            do {
                if (strcmp(entry.szExeFile, process_name) == 0) {
                    pid = entry.th32ProcessID;
                    write_log("[+] Found %s (PID: %d)", process_name, pid);
                    break;
                }
            } while (Process32Next(snapshot, &entry));
        }
        
        if (pid == 0) {
            Sleep(500);
            attempts++;
            // Пересоздаём снэпшот
            CloseHandle(snapshot);
            snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        }
    }
    
    CloseHandle(snapshot);
    return pid;
}

// ============================================================
// 5. ОСТАНОВКА VAC (Через драйвер)
// ============================================================

bool stop_vac() {
    write_log("[*] Attempting to stop VAC...");
    
    // Ищем драйвер VAC
    HANDLE vac_driver = CreateFileA("\\\\.\\VAC", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (vac_driver != INVALID_HANDLE_VALUE) {
        DWORD bytes_returned;
        // Отправляем IOCTL на отключение VAC
        DeviceIoControl(vac_driver, 0x12345678, NULL, 0, NULL, 0, &bytes_returned, NULL);
        CloseHandle(vac_driver);
        write_log("[+] VAC stopped");
        return true;
    }
    
    // Альтернатива: находим VAC модули в памяти и обнуляем
    DWORD pid = find_process("hl2.exe");
    if (pid == 0) pid = find_process("tf2.exe");
    if (pid == 0) return false;
    
    HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!process) return false;
    
    // Ищем модули VAC
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
    if (snapshot == INVALID_HANDLE_VALUE) {
        CloseHandle(process);
        return false;
    }
    
    MODULEENTRY32 entry = { sizeof(MODULEENTRY32) };
    if (Module32First(snapshot, &entry)) {
        do {
            if (strstr(entry.szModule, "vac") || strstr(entry.szModule, "VAC")) {
                write_log("[*] Found VAC module: %s at 0x%p", entry.szModule, entry.modBaseAddr);
                // Обнуляем память модуля
                DWORD old_protect;
                VirtualProtectEx(process, entry.modBaseAddr, entry.modBaseSize, PAGE_EXECUTE_READWRITE, &old_protect);
                ZeroMemory(entry.modBaseAddr, entry.modBaseSize);
                VirtualProtectEx(process, entry.modBaseAddr, entry.modBaseSize, old_protect, &old_protect);
                write_log("[+] VAC module disabled");
            }
        } while (Module32Next(snapshot, &entry));
    }
    
    CloseHandle(snapshot);
    CloseHandle(process);
    return true;
}

// ============================================================
// 6. ИНЖЕКТ ЧЕРЕЗ LOADLIBRARY (Обычный)
// ============================================================

bool inject_dll_loadlibrary(HANDLE process, const char* dll_path) {
    write_log("[*] Injecting via LoadLibraryA...");
    
    HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
    if (!kernel32) {
        write_log("[!] Failed to get kernel32.dll handle");
        return false;
    }
    
    LPVOID load_library = (LPVOID)GetProcAddress(kernel32, "LoadLibraryA");
    if (!load_library) {
        write_log("[!] Failed to get LoadLibraryA address");
        return false;
    }
    
    size_t path_len = strlen(dll_path) + 1;
    LPVOID remote_mem = VirtualAllocEx(process, NULL, path_len, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remote_mem) {
        write_log("[!] Failed to allocate memory in target process");
        return false;
    }
    
    if (!WriteProcessMemory(process, remote_mem, dll_path, path_len, NULL)) {
        write_log("[!] Failed to write memory in target process");
        VirtualFreeEx(process, remote_mem, 0, MEM_RELEASE);
        return false;
    }
    
    HANDLE thread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)load_library, remote_mem, 0, NULL);
    if (!thread) {
        write_log("[!] Failed to create remote thread");
        VirtualFreeEx(process, remote_mem, 0, MEM_RELEASE);
        return false;
    }
    
    WaitForSingleObject(thread, 5000); // Ждём 5 секунд
    CloseHandle(thread);
    VirtualFreeEx(process, remote_mem, 0, MEM_RELEASE);
    
    write_log("[+] Injection successful!");
    return true;
}

// ============================================================
// 7. ИНЖЕКТ ЧЕРЕЗ QUEUEUSERAPC (Более скрытный)
// ============================================================

bool inject_dll_apc(HANDLE process, const char* dll_path) {
    write_log("[*] Injecting via QueueUserAPC...");
    
    HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
    if (!kernel32) return false;
    
    LPVOID load_library = (LPVOID)GetProcAddress(kernel32, "LoadLibraryA");
    if (!load_library) return false;
    
    size_t path_len = strlen(dll_path) + 1;
    LPVOID remote_mem = VirtualAllocEx(process, NULL, path_len, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remote_mem) return false;
    
    if (!WriteProcessMemory(process, remote_mem, dll_path, path_len, NULL)) {
        VirtualFreeEx(process, remote_mem, 0, MEM_RELEASE);
        return false;
    }
    
    // Получаем список потоков процесса
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        VirtualFreeEx(process, remote_mem, 0, MEM_RELEASE);
        return false;
    }
    
    THREADENTRY32 entry = { sizeof(THREADENTRY32) };
    DWORD pid = GetProcessId(process);
    bool injected = false;
    
    if (Thread32First(snapshot, &entry)) {
        do {
            if (entry.th32OwnerProcessID == pid) {
                HANDLE thread = OpenThread(THREAD_ALL_ACCESS, FALSE, entry.th32ThreadID);
                if (thread) {
                    // Откладываем APC в очередь потока
                    if (QueueUserAPC((PAPCFUNC)load_library, thread, (ULONG_PTR)remote_mem)) {
                        injected = true;
                        write_log("[+] APC queued to thread %d", entry.th32ThreadID);
                        CloseHandle(thread);
                        break;
                    }
                    CloseHandle(thread);
                }
            }
        } while (Thread32Next(snapshot, &entry));
    }
    
    CloseHandle(snapshot);
    if (!injected) {
        VirtualFreeEx(process, remote_mem, 0, MEM_RELEASE);
        write_log("[!] Failed to inject via APC");
        return false;
    }
    
    write_log("[+] Injection successful!");
    return true;
}

// ============================================================
// 8. ИНЖЕКТ ЧЕРЕЗ SETWINDOWSHOOKEX (Самый скрытный)
// ============================================================

HMODULE g_hook_dll = NULL;
HHOOK g_hook = NULL;

LRESULT CALLBACK HookProc(int code, WPARAM wParam, LPARAM lParam) {
    return CallNextHookEx(g_hook, code, wParam, lParam);
}

bool inject_dll_hook(HWND target_hwnd, const char* dll_path) {
    write_log("[*] Injecting via SetWindowsHookEx...");
    
    // Загружаем DLL в наш процесс
    g_hook_dll = LoadLibraryA(dll_path);
    if (!g_hook_dll) {
        write_log("[!] Failed to load DLL into injector");
        return false;
    }
    
    // Получаем адрес процедуры хука
    FARPROC hook_proc = GetProcAddress(g_hook_dll, "HookProc");
    if (!hook_proc) {
        // Если нет HookProc, используем любую экспортированную функцию
        hook_proc = GetProcAddress(g_hook_dll, "DllMain");
        if (!hook_proc) {
            write_log("[!] Failed to get hook procedure");
            FreeLibrary(g_hook_dll);
            return false;
        }
    }
    
    // Устанавливаем хук на WH_GETMESSAGE (внедряется в процесс)
    g_hook = SetWindowsHookExA(WH_GETMESSAGE, (HOOKPROC)hook_proc, g_hook_dll, GetWindowThreadProcessId(target_hwnd, NULL));
    if (!g_hook) {
        write_log("[!] Failed to set hook");
        FreeLibrary(g_hook_dll);
        return false;
    }
    
    // Отправляем сообщение, чтобы DLL загрузилась
    PostMessageA(target_hwnd, WM_USER + 1, 0, 0);
    Sleep(1000);
    
    // Снимаем хук
    UnhookWindowsHookEx(g_hook);
    FreeLibrary(g_hook_dll);
    g_hook = NULL;
    g_hook_dll = NULL;
    
    write_log("[+] Injection successful!");
    return true;
}

// ============================================================
// 9. ОСНОВНАЯ ФУНКЦИЯ
// ============================================================

void inject() {
    write_log("========================================");
    write_log("[*] TF2 Cheat Silent Injector v2.0");
    write_log("[*] Time: %s", __TIMESTAMP__);
    
    // Инициализация
    init_paths();
    
    // Проверяем наличие DLL
    if (GetFileAttributesA(g_dll_path) == INVALID_FILE_ATTRIBUTES) {
        write_log("[!] DLL not found: %s", g_dll_path);
        return;
    }
    write_log("[+] DLL found: %s", g_dll_path);
    
    // Останавливаем VAC
    stop_vac();
    
    // Ищем TF2
    DWORD pid = find_process("hl2.exe");
    if (pid == 0) pid = find_process("tf2.exe");
    
    if (pid == 0) {
        write_log("[!] TF2 process not found, retrying...");
        Sleep(1000);
        pid = find_process("hl2.exe");
        if (pid == 0) pid = find_process("tf2.exe");
    }
    
    if (pid == 0) {
        write_log("[!] TF2 process not found after retry");
        return;
    }
    
    // Открываем процесс
    HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!process) {
        // Если не хватает прав, пробуем с минимальными
        process = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, pid);
    }
    
    if (!process) {
        write_log("[!] Failed to open process (PID: %d)", pid);
        return;
    }
    
    write_log("[+] Process opened successfully");
    
    // Пробуем разные методы инжекта
    bool success = false;
    
    // Метод 1: LoadLibrary (самый надёжный)
    if (!success) {
        success = inject_dll_loadlibrary(process, g_dll_path);
    }
    
    // Метод 2: APC (если первый не сработал)
    if (!success) {
        success = inject_dll_apc(process, g_dll_path);
    }
    
    // Метод 3: Hook (если ничего не работает)
    if (!success) {
        HWND hwnd = FindWindowA(NULL, "Team Fortress 2");
        if (hwnd) {
            success = inject_dll_hook(hwnd, g_dll_path);
        }
    }
    
    CloseHandle(process);
    
    if (success) {
        write_log("[+] Injection completed successfully!");
        // Скрываем инжектор (процесс сам закроется)
        FreeConsole();
        ExitProcess(0);
    } else {
        write_log("[!] All injection methods failed!");
    }
}

// ============================================================
// 10. ТОЧКА ВХОДА (GUI скрытая)
// ============================================================

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Проверяем аргументы командной строки
    if (strstr(lpCmdLine, "-log")) {
        g_log = true;
    }
    if (strstr(lpCmdLine, "-silent") || strstr(lpCmdLine, "-hide")) {
        g_silent = true;
    }
    
    // Скрываем окно, если тихий режим
    if (g_silent) {
        ShowWindow(GetConsoleWindow(), SW_HIDE);
        FreeConsole();
    }
    
    // Запускаем инжект
    inject();
    
    return 0;
}

// ============================================================
// 11. ВХОД ДЛЯ CONSOLE
// ============================================================

int main() {
    // Проверяем, есть ли консоль
    if (GetConsoleWindow()) {
        FreeConsole();
    }
    
    inject();
    return 0;
}

// ============================================================
// КОНЕЦ ФАЙЛА
// ============================================================
