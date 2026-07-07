/*
 * ============================================================
 * TF2 INJECTOR — СКРЫТЫЙ ИНЖЕКТОР С GUI
 * Показывает статус: ожидание, инжект, готово, ошибка
 * ============================================================
 */

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tlhelp32.h>
#include <time.h>

// ============================================================
// 1. КОНСТАНТЫ
// ============================================================

#define DLL_NAME "tf2cheat.dll"
#define MAX_ATTEMPTS 30
#define RETRY_DELAY 500

// ============================================================
// 2. СТРУКТУРЫ
// ============================================================

typedef struct {
    HWND hwnd;
    HWND status_text;
    HWND status_icon;
    HWND progress_bar;
    HWND btn_inject;
    HWND btn_exit;
    HBRUSH brush_ready;
    HBRUSH brush_injecting;
    HBRUSH brush_done;
    HBRUSH brush_error;
    int status;
    char dll_path[MAX_PATH];
    DWORD pid;
    HANDLE process;
} InjectorUI;

// ============================================================
// 3. ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ
// ============================================================

InjectorUI g_ui = {0};
const char* g_status_messages[] = {
    "Waiting for TF2...",
    "TF2 found! Injecting...",
    "✅ Injection successful!",
    "❌ Injection failed!"
};

// ============================================================
// 4. ФУНКЦИИ ПОИСКА TF2
// ============================================================

DWORD find_process(const char* name) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32 entry = {sizeof(PROCESSENTRY32)};
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
// 5. ИНЖЕКТ DLL
// ============================================================

bool inject_dll(DWORD pid, const char* dll_path) {
    HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!process) {
        process = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | 
                             PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, pid);
    }
    if (!process) return false;

    HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
    if (!kernel32) {
        CloseHandle(process);
        return false;
    }

    LPVOID load_library = (LPVOID)GetProcAddress(kernel32, "LoadLibraryA");
    if (!load_library) {
        CloseHandle(process);
        return false;
    }

    size_t path_len = strlen(dll_path) + 1;
    LPVOID remote_mem = VirtualAllocEx(process, NULL, path_len, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remote_mem) {
        CloseHandle(process);
        return false;
    }

    if (!WriteProcessMemory(process, remote_mem, dll_path, path_len, NULL)) {
        VirtualFreeEx(process, remote_mem, 0, MEM_RELEASE);
        CloseHandle(process);
        return false;
    }

    HANDLE thread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)load_library, remote_mem, 0, NULL);
    if (!thread) {
        VirtualFreeEx(process, remote_mem, 0, MEM_RELEASE);
        CloseHandle(process);
        return false;
    }

    WaitForSingleObject(thread, 5000);
    CloseHandle(thread);
    VirtualFreeEx(process, remote_mem, 0, MEM_RELEASE);
    CloseHandle(process);

    return true;
}

// ============================================================
// 6. UI ФУНКЦИИ
// ============================================================

void update_status(int status, const char* msg) {
    g_ui.status = status;
    SetWindowTextA(g_ui.status_text, msg);

    // Меняем цвет индикатора
    HBRUSH brush = NULL;
    switch(status) {
        case 0: brush = g_ui.brush_ready; break;
        case 1: brush = g_ui.brush_injecting; break;
        case 2: brush = g_ui.brush_done; break;
        case 3: brush = g_ui.brush_error; break;
    }
    if (brush) {
        InvalidateRect(g_ui.status_icon, NULL, TRUE);
    }

    // Обновляем прогресс-бар
    if (status == 1) {
        SendMessageA(g_ui.progress_bar, PBM_SETMARQUEE, TRUE, 0);
    } else {
        SendMessageA(g_ui.progress_bar, PBM_SETMARQUEE, FALSE, 0);
        SendMessageA(g_ui.progress_bar, PBM_SETPOS, status == 2 ? 100 : 0, 0);
    }

    // Кнопка инжекта
    EnableWindow(g_ui.btn_inject, status == 2 || status == 3);
}

void set_icon_color(HWND hwnd, HBRUSH brush) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    RECT rect;
    GetClientRect(hwnd, &rect);
    FillRect(hdc, &rect, brush);
    EndPaint(hwnd, &ps);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_PAINT:
            if (g_ui.status_icon) {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(g_ui.status_icon, &ps);
                RECT rect;
                GetClientRect(g_ui.status_icon, &rect);
                HBRUSH brush = NULL;
                switch(g_ui.status) {
                    case 0: brush = g_ui.brush_ready; break;
                    case 1: brush = g_ui.brush_injecting; break;
                    case 2: brush = g_ui.brush_done; break;
                    case 3: brush = g_ui.brush_error; break;
                    default: brush = g_ui.brush_ready;
                }
                FillRect(hdc, &rect, brush);
                EndPaint(g_ui.status_icon, &ps);
            }
            return 0;

        case WM_COMMAND:
            if (LOWORD(wParam) == 1) { // Inject button
                EnableWindow(g_ui.btn_inject, FALSE);
                SetWindowTextA(g_ui.btn_inject, "Injecting...");
                update_status(1, "Injecting DLL...");

                // Ищем процесс
                DWORD pid = find_process("hl2.exe");
                if (pid == 0) pid = find_process("tf2.exe");
                if (pid == 0) {
                    update_status(3, "❌ TF2 not found!");
                    SetWindowTextA(g_ui.btn_inject, "Retry");
                    EnableWindow(g_ui.btn_inject, TRUE);
                    return 0;
                }

                // Инжектим
                if (inject_dll(pid, g_ui.dll_path)) {
                    update_status(2, "✅ Injection successful!");
                    SetWindowTextA(g_ui.btn_inject, "✅ Injected");
                } else {
                    update_status(3, "❌ Injection failed!");
                    SetWindowTextA(g_ui.btn_inject, "Retry");
                    EnableWindow(g_ui.btn_inject, TRUE);
                }
            }
            if (LOWORD(wParam) == 2) { // Exit button
                PostQuitMessage(0);
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_CTLCOLORSTATIC:
            if ((HWND)lParam == g_ui.status_icon) {
                return (LRESULT)GetStockObject(NULL_BRUSH);
            }
            return 0;

        default:
            return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
}

// ============================================================
// 7. ИНИЦИАЛИЗАЦИЯ UI
// ============================================================

bool init_ui(HINSTANCE hInstance) {
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(30, 30, 40));
    wc.lpszClassName = "TF2InjectorClass";
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    
    if (!RegisterClassA(&wc)) return false;

    g_ui.hwnd = CreateWindowExA(
        WS_EX_TOOLWINDOW,
        "TF2InjectorClass",
        "TF2 Injector",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 250,
        NULL, NULL, hInstance, NULL
    );

    if (!g_ui.hwnd) return false;

    // Создаём кисти
    g_ui.brush_ready = CreateSolidBrush(RGB(255, 255, 0));
    g_ui.brush_injecting = CreateSolidBrush(RGB(255, 165, 0));
    g_ui.brush_done = CreateSolidBrush(RGB(0, 255, 0));
    g_ui.brush_error = CreateSolidBrush(RGB(255, 0, 0));

    // Статус-индикатор (круг)
    g_ui.status_icon = CreateWindowExA(0, "STATIC", "", 
        WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
        20, 30, 30, 30,
        g_ui.hwnd, NULL, hInstance, NULL);

    // Текст статуса
    g_ui.status_text = CreateWindowExA(0, "STATIC", "Waiting for TF2...",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        70, 35, 280, 25,
        g_ui.hwnd, NULL, hInstance, NULL);
    SendMessageA(g_ui.status_text, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
    SetWindowTextColor(g_ui.status_text, RGB(200, 200, 200));

    // Прогресс-бар
    g_ui.progress_bar = CreateWindowExA(0, PROGRESS_CLASSA, "",
        WS_CHILD | WS_VISIBLE | PBS_MARQUEE,
        20, 80, 340, 15,
        g_ui.hwnd, NULL, hInstance, NULL);
    SendMessageA(g_ui.progress_bar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
    SendMessageA(g_ui.progress_bar, PBM_SETPOS, 0, 0);

    // Кнопка Inject
    g_ui.btn_inject = CreateWindowExA(0, "BUTTON", "Inject",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        100, 120, 80, 30,
        g_ui.hwnd, (HMENU)1, hInstance, NULL);
    SendMessageA(g_ui.btn_inject, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);

    // Кнопка Exit
    g_ui.btn_exit = CreateWindowExA(0, "BUTTON", "Exit",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        200, 120, 80, 30,
        g_ui.hwnd, (HMENU)2, hInstance, NULL);
    SendMessageA(g_ui.btn_exit, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);

    // Версия
    CreateWindowExA(0, "STATIC", "v2.0",
        WS_CHILD | WS_VISIBLE | SS_RIGHT,
        300, 180, 70, 20,
        g_ui.hwnd, NULL, hInstance, NULL);
    SendMessageA(g_ui.status_text, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);

    ShowWindow(g_ui.hwnd, SW_SHOW);
    UpdateWindow(g_ui.hwnd);

    return true;
}

// ============================================================
// 8. ФОНОВЫЙ ПОИСК TF2 (АВТО-ИНЖЕКТ)
// ============================================================

DWORD WINAPI auto_inject_thread(LPVOID param) {
    InjectorUI* ui = (InjectorUI*)param;
    
    // Ждём TF2
    update_status(0, "Waiting for TF2...");
    
    DWORD pid = 0;
    int attempts = 0;
    while (attempts < MAX_ATTEMPTS) {
        pid = find_process("hl2.exe");
        if (pid == 0) pid = find_process("tf2.exe");
        if (pid != 0) break;
        attempts++;
        Sleep(RETRY_DELAY);
    }

    if (pid == 0) {
        update_status(3, "❌ TF2 not found! Click Inject to retry.");
        EnableWindow(ui->btn_inject, TRUE);
        SetWindowTextA(ui->btn_inject, "Retry");
        return 0;
    }

    update_status(1, "TF2 found! Injecting...");
    SetWindowTextA(ui->btn_inject, "Injecting...");

    // Получаем путь к DLL
    char dll_path[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, dll_path);
    strcat(dll_path, "\\");
    strcat(dll_path, DLL_NAME);

    if (GetFileAttributesA(dll_path) == INVALID_FILE_ATTRIBUTES) {
        update_status(3, "❌ tf2cheat.dll not found!");
        EnableWindow(ui->btn_inject, TRUE);
        SetWindowTextA(ui->btn_inject, "Retry");
        return 0;
    }

    strcpy(ui->dll_path, dll_path);

    // Инжектим
    if (inject_dll(pid, dll_path)) {
        update_status(2, "✅ Injection successful! Press INSERT in game.");
        SetWindowTextA(ui->btn_inject, "✅ Injected");
        EnableWindow(ui->btn_inject, FALSE);
        
        // Скрываем окно через 3 секунды
        Sleep(3000);
        ShowWindow(ui->hwnd, SW_HIDE);
        
        // Показываем уведомление
        NOTIFYICONDATAA nid = {0};
        nid.cbSize = sizeof(NOTIFYICONDATAA);
        nid.hWnd = ui->hwnd;
        nid.uID = 1;
        nid.uFlags = NIF_ICON | NIF_TIP | NIF_INFO;
        nid.dwInfoFlags = NIIF_INFO;
        strcpy(nid.szInfoTitle, "TF2 Cheat");
        strcpy(nid.szInfo, "Injection successful! Press INSERT to toggle menu.");
        Shell_NotifyIconA(NIM_ADD, &nid);
    } else {
        update_status(3, "❌ Injection failed!");
        EnableWindow(ui->btn_inject, TRUE);
        SetWindowTextA(ui->btn_inject, "Retry");
    }

    return 0;
}

// ============================================================
// 9. ТОЧКА ВХОДА
// ============================================================

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Скрываем консоль
    FreeConsole();
    ShowWindow(GetConsoleWindow(), SW_HIDE);

    // Инициализируем UI
    if (!init_ui(hInstance)) {
        MessageBoxA(NULL, "Failed to create UI!", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Запускаем фоновый поток инжекта
    CreateThread(NULL, 0, auto_inject_thread, &g_ui, 0, NULL);

    // Главный цикл сообщений
    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    // Очистка
    DeleteObject(g_ui.brush_ready);
    DeleteObject(g_ui.brush_injecting);
    DeleteObject(g_ui.brush_done);
    DeleteObject(g_ui.brush_error);

    return 0;
}

// ============================================================
// КОНЕЦ ФАЙЛА
// ============================================================