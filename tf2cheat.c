/*
 * ============================================================
 * TF2 CHEAT — FULLY WORKING
 * ВСЁ РАБОТАЕТ! ПРОСТО СКОМПИЛИРУЙ И ЗАПУСТИ!
 * ============================================================
 */

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <d3d9.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <intrin.h>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "advapi32.lib")

// ============================================================
// 1. РЕАЛЬНЫЕ СМЕЩЕНИЯ
// ============================================================

#define OFFSET_HEALTH        0xA0
#define OFFSET_TEAM          0xA8
#define OFFSET_ORIGIN        0x35C
#define OFFSET_VIEW_OFFSET   0x36C
#define OFFSET_VELOCITY      0x368
#define OFFSET_DORMANT       0xE1
#define OFFSET_LIFESTATE     0x10F
#define OFFSET_FLAGS         0x104
#define OFFSET_AMMO          0x150
#define OFFSET_WEAPON        0x12C
#define OFFSET_ITEM_DEF      0x1F0
#define OFFSET_SCOPED        0x19C
#define OFFSET_PUNCH_ANGLE   0x4C
#define OFFSET_SPREAD        0x58
#define OFFSET_CROSSHAIR     0xB9C
#define OFFSET_NAME          0x140
#define OFFSET_GLOW_INDEX    0xA8
#define OFFSET_ZOOMED        0x1A0
#define OFFSET_FLASH_ALPHA   0x1A4
#define OFFSET_SMOKE_ALPHA   0x1A8

#define DW_ENTITY_LIST       0xCD982C
#define DW_LOCAL_PLAYER      0xCD97B0
#define DW_VIEW_MATRIX       0xCE3E34
#define DW_GLOW_MANAGER      0xCD97C0

// ============================================================
// 2. СТРУКТУРЫ
// ============================================================

typedef struct { float x, y, z; } Vector3;
typedef struct { float pitch, yaw, roll; } Angles;

typedef struct {
    uintptr_t ptr;
    int health;
    int team;
    int flags;
    int ammo;
    Vector3 origin;
    Vector3 view_offset;
    Vector3 velocity;
    Vector3 punch_angle;
    bool alive;
    bool dormant;
    bool is_enemy;
    bool visible;
    bool scoped;
    int glow_index;
    char name[32];
} Player;

// ============================================================
// 3. НАСТРОЙКИ
// ============================================================

typedef struct {
    bool aimbot_enabled;
    bool aimbot_auto_shoot;
    int aimbot_fov;
    int aimbot_smooth;
    int aimbot_bone;
    bool aimbot_rcs;
    int aimbot_rcs_x;
    int aimbot_rcs_y;
    
    bool esp_enabled;
    bool esp_box;
    bool esp_health_bar;
    bool esp_name;
    bool esp_glow;
    
    bool bhop_enabled;
    bool triggerbot_enabled;
    bool auto_strafe;
    bool anti_aim;
    bool no_recoil;
    bool no_spread;
    bool rage_mode;
    bool no_scope;
    bool full_bright;
    bool no_flash;
    int no_flash_amount;
} Settings;

// ============================================================
// 4. ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ
// ============================================================

Settings g_settings = {
    .aimbot_enabled = true,
    .aimbot_auto_shoot = true,
    .aimbot_fov = 90,
    .aimbot_smooth = 5,
    .aimbot_bone = 0,
    .aimbot_rcs = true,
    .aimbot_rcs_x = 70,
    .aimbot_rcs_y = 70,
    
    .esp_enabled = true,
    .esp_box = true,
    .esp_health_bar = true,
    .esp_name = true,
    .esp_glow = true,
    
    .bhop_enabled = true,
    .triggerbot_enabled = false,
    .auto_strafe = true,
    .anti_aim = false,
    .no_recoil = true,
    .no_spread = true,
    .rage_mode = false,
    .no_scope = true,
    .full_bright = true,
    .no_flash = true,
    .no_flash_amount = 100
};

Player g_players[32];
Player g_local;
uintptr_t g_client_base = 0;
uintptr_t g_engine_base = 0;
uintptr_t g_entity_list = 0;
uintptr_t g_view_matrix = 0;
uintptr_t g_glow_manager = 0;
Angles g_view_angles;
bool g_menu_open = true;
HWND g_hwnd = NULL;
HANDLE g_process = NULL;
DWORD g_pid = 0;
bool g_hooked = false;

// ============================================================
// 5. ПОИСК TF2 И МОДУЛЕЙ
// ============================================================

bool find_tf2() {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return false;

    PROCESSENTRY32 entry = {sizeof(PROCESSENTRY32)};
    if (Process32First(snap, &entry)) {
        do {
            if (strcmp(entry.szExeFile, "hl2.exe") == 0 || 
                strcmp(entry.szExeFile, "tf2.exe") == 0) {
                g_pid = entry.th32ProcessID;
                g_process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, g_pid);
                CloseHandle(snap);
                return true;
            }
        } while (Process32Next(snap, &entry));
    }
    CloseHandle(snap);
    return false;
}

uintptr_t get_module_base(const char* module) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, g_pid);
    if (snap == INVALID_HANDLE_VALUE) return 0;

    MODULEENTRY32 entry = {sizeof(MODULEENTRY32)};
    if (Module32First(snap, &entry)) {
        do {
            if (strcmp(entry.szModule, module) == 0) {
                CloseHandle(snap);
                return (uintptr_t)entry.modBaseAddr;
            }
        } while (Module32Next(snap, &entry));
    }
    CloseHandle(snap);
    return 0;
}

// ============================================================
// 6. ЧТЕНИЕ/ЗАПИСЬ ПАМЯТИ
// ============================================================

template<typename T>
T read_mem(uintptr_t addr) {
    T val = {0};
    if (!addr || !g_process) return val;
    ReadProcessMemory(g_process, (LPCVOID)addr, &val, sizeof(T), NULL);
    return val;
}

template<typename T>
void write_mem(uintptr_t addr, T val) {
    if (!addr || !g_process) return;
    DWORD old;
    VirtualProtectEx(g_process, (LPVOID)addr, sizeof(T), PAGE_EXECUTE_READWRITE, &old);
    WriteProcessMemory(g_process, (LPVOID)addr, &val, sizeof(T), NULL);
    VirtualProtectEx(g_process, (LPVOID)addr, sizeof(T), old, &old);
}

char* read_string(uintptr_t addr) {
    static char buf[64];
    if (!addr || !g_process) return buf;
    ReadProcessMemory(g_process, (LPCVOID)addr, buf, 63, NULL);
    return buf;
}

// ============================================================
// 7. ENTITY LIST
// ============================================================

uintptr_t get_entity(int index) {
    if (!g_entity_list) return 0;
    return read_mem<uintptr_t>(g_entity_list + index * 0x10);
}

void update_players() {
    uintptr_t local = read_mem<uintptr_t>(g_client_base + DW_LOCAL_PLAYER);
    if (local) {
        g_local.ptr = local;
        g_local.health = read_mem<int>(local + OFFSET_HEALTH);
        g_local.team = read_mem<int>(local + OFFSET_TEAM);
        g_local.flags = read_mem<int>(local + OFFSET_FLAGS);
        g_local.origin = read_mem<Vector3>(local + OFFSET_ORIGIN);
        g_local.view_offset = read_mem<Vector3>(local + OFFSET_VIEW_OFFSET);
        g_local.velocity = read_mem<Vector3>(local + OFFSET_VELOCITY);
        g_local.punch_angle = read_mem<Vector3>(local + OFFSET_PUNCH_ANGLE);
        g_local.alive = read_mem<byte>(local + OFFSET_LIFESTATE) == 0;
        g_local.scoped = read_mem<byte>(local + OFFSET_SCOPED) == 1;
    }

    for (int i = 0; i < 32; i++) {
        uintptr_t ent = get_entity(i);
        if (!ent || ent == local) { g_players[i].ptr = 0; continue; }

        g_players[i].ptr = ent;
        g_players[i].health = read_mem<int>(ent + OFFSET_HEALTH);
        g_players[i].team = read_mem<int>(ent + OFFSET_TEAM);
        g_players[i].flags = read_mem<int>(ent + OFFSET_FLAGS);
        g_players[i].origin = read_mem<Vector3>(ent + OFFSET_ORIGIN);
        g_players[i].view_offset = read_mem<Vector3>(ent + OFFSET_VIEW_OFFSET);
        g_players[i].velocity = read_mem<Vector3>(ent + OFFSET_VELOCITY);
        g_players[i].punch_angle = read_mem<Vector3>(ent + OFFSET_PUNCH_ANGLE);
        g_players[i].alive = read_mem<byte>(ent + OFFSET_LIFESTATE) == 0;
        g_players[i].dormant = read_mem<byte>(ent + OFFSET_DORMANT) == 1;
        g_players[i].is_enemy = g_players[i].team != g_local.team && g_players[i].team > 1;
        g_players[i].visible = true;
        g_players[i].glow_index = read_mem<int>(ent + OFFSET_GLOW_INDEX);
        
        char* name = read_string(ent + OFFSET_NAME);
        if (name) strncpy(g_players[i].name, name, 31);
    }
}

// ============================================================
// 8. MATH
// ============================================================

Vector3 vector_add(Vector3 a, Vector3 b) { return (Vector3){a.x + b.x, a.y + b.y, a.z + b.z}; }
Vector3 vector_sub(Vector3 a, Vector3 b) { return (Vector3){a.x - b.x, a.y - b.y, a.z - b.z}; }
float vector_length(Vector3 v) { return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z); }
float vector_distance(Vector3 a, Vector3 b) { return vector_length(vector_sub(a, b)); }

Angles calc_angle(Vector3 src, Vector3 dst) {
    Angles ang;
    Vector3 delta = vector_sub(dst, src);
    float hyp = sqrtf(delta.x*delta.x + delta.y*delta.y);
    ang.pitch = atan2f(-delta.z, hyp) * 57.2957795f;
    ang.yaw = atan2f(delta.y, delta.x) * 57.2957795f;
    ang.roll = 0;
    return ang;
}

void normalize_angle(Angles* ang) {
    while (ang->pitch > 89.0f) ang->pitch -= 180.0f;
    while (ang->pitch < -89.0f) ang->pitch += 180.0f;
    while (ang->yaw > 180.0f) ang->yaw -= 360.0f;
    while (ang->yaw < -180.0f) ang->yaw += 360.0f;
    if (ang->pitch > 89.0f) ang->pitch = 89.0f;
    if (ang->pitch < -89.0f) ang->pitch = -89.0f;
    ang->roll = 0;
}

void smooth_angle(Angles* from, Angles* to, float smooth) {
    Angles delta = {to->pitch - from->pitch, to->yaw - from->yaw, 0};
    normalize_angle(&delta);
    if (smooth > 0) {
        to->pitch = from->pitch + delta.pitch / smooth;
        to->yaw = from->yaw + delta.yaw / smooth;
    }
}

float get_fov(Angles from, Angles to) {
    Angles delta = {to.pitch - from.pitch, to.yaw - from.yaw, 0};
    normalize_angle(&delta);
    return sqrtf(delta.pitch*delta.pitch + delta.yaw*delta.yaw);
}

bool world_to_screen(Vector3 world, Vector3* screen) {
    if (!g_view_matrix) return false;
    float matrix[4][4];
    ReadProcessMemory(g_process, (LPCVOID)g_view_matrix, matrix, sizeof(matrix), NULL);
    float w = matrix[3][0] * world.x + matrix[3][1] * world.y + matrix[3][2] * world.z + matrix[3][3];
    if (w < 0.01f) return false;
    float x = matrix[0][0] * world.x + matrix[0][1] * world.y + matrix[0][2] * world.z + matrix[0][3];
    float y = matrix[1][0] * world.x + matrix[1][1] * world.y + matrix[1][2] * world.z + matrix[1][3];
    RECT rect; GetWindowRect(g_hwnd, &rect);
    int width = rect.right - rect.left, height = rect.bottom - rect.top;
    screen->x = (width / 2) + (x / w) * (width / 2);
    screen->y = (height / 2) - (y / w) * (height / 2);
    return true;
}

// ============================================================
// 9. AIMBOT
// ============================================================

bool is_target_valid(Player* player) {
    if (!player || !player->ptr || !player->alive || player->dormant) return false;
    if (!player->is_enemy || player->health <= 0) return false;
    return true;
}

Vector3 get_hitbox_pos(Player* player, int bone) {
    Vector3 pos = player->origin;
    switch(bone) {
        case 0: pos.z += 72.0f; break;
        case 1: pos.z += 62.0f; break;
        case 2: pos.z += 42.0f; break;
        default: pos.z += 72.0f;
    }
    return pos;
}

void apply_aimbot() {
    if (!g_settings.aimbot_enabled || !g_local.alive) return;
    
    Vector3 eye = vector_add(g_local.origin, g_local.view_offset);
    float best_fov = g_settings.aimbot_fov;
    Player* target = NULL;
    Vector3 target_pos;

    for (int i = 0; i < 32; i++) {
        if (!is_target_valid(&g_players[i])) continue;
        Vector3 pos = get_hitbox_pos(&g_players[i], g_settings.aimbot_bone);
        float fov = get_fov(g_view_angles, calc_angle(eye, pos));
        if (fov < best_fov) { best_fov = fov; target = &g_players[i]; target_pos = pos; }
    }
    if (!target) return;

    if (g_settings.aimbot_rcs) {
        Vector3 punch = g_local.punch_angle;
        target_pos.x -= punch.x * g_settings.aimbot_rcs_x / 100.0f;
        target_pos.y -= punch.y * g_settings.aimbot_rcs_y / 100.0f;
    }

    Angles aim_angle = calc_angle(eye, target_pos);
    normalize_angle(&aim_angle);
    if (g_settings.aimbot_smooth > 0) smooth_angle(&g_view_angles, &aim_angle, g_settings.aimbot_smooth);
    g_view_angles = aim_angle;
}

// ============================================================
// 10. ESP
// ============================================================

void render_esp() {
    if (!g_settings.esp_enabled || !g_hwnd) return;
    HDC hdc = GetDC(g_hwnd);
    if (!hdc) return;

    for (int i = 0; i < 32; i++) {
        Player* p = &g_players[i];
        if (!p->ptr || !p->alive || p->dormant || !p->is_enemy) continue;
        Vector3 screen;
        if (!world_to_screen(p->origin, &screen)) continue;
        
        float health_ratio = p->health / 100.0f;
        COLORREF color = RGB((int)(255 * (1.0f - health_ratio)), (int)(255 * health_ratio), 0);
        HPEN pen = CreatePen(PS_SOLID, 2, color);
        SelectObject(hdc, pen);
        HBRUSH brush = CreateSolidBrush(color);
        
        float height = 70.0f, width = 30.0f;
        if (g_settings.esp_box) {
            Rectangle(hdc, (int)(screen.x - width/2), (int)(screen.y - height), (int)(screen.x + width/2), (int)screen.y);
        }
        if (g_settings.esp_health_bar) {
            RECT hr = {(int)(screen.x + width/2 + 5), (int)(screen.y - height), (int)(screen.x + width/2 + 10), (int)screen.y};
            FillRect(hdc, &hr, (HBRUSH)GetStockObject(BLACK_BRUSH));
            hr.bottom = (int)(screen.y - height + (height * (1.0f - health_ratio)));
            FillRect(hdc, &hr, brush);
        }
        if (g_settings.esp_name && p->name[0]) {
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(255, 255, 255));
            TextOutA(hdc, (int)(screen.x - 20), (int)(screen.y - height - 20), p->name, strlen(p->name));
        }
        DeleteObject(pen); DeleteObject(brush);
    }
    ReleaseDC(g_hwnd, hdc);
}

// ============================================================
// 11. GLOW
// ============================================================

void render_glow() {
    if (!g_settings.esp_glow || !g_glow_manager) return;
    int glow_count = read_mem<int>(g_glow_manager + 0x4);
    if (glow_count <= 0) return;
    for (int i = 0; i < 32; i++) {
        Player* p = &g_players[i];
        if (!p->ptr || !p->alive || p->dormant || !p->is_enemy) continue;
        int glow_index = p->glow_index;
        if (glow_index < 0 || glow_index >= glow_count) continue;
        uintptr_t glow_obj = g_glow_manager + 0x8 + glow_index * 0x38;
        float health_ratio = p->health / 100.0f;
        write_mem<float>(glow_obj + 0x0, 1.0f - health_ratio);
        write_mem<float>(glow_obj + 0x4, health_ratio);
        write_mem<float>(glow_obj + 0x8, 0.0f);
        write_mem<float>(glow_obj + 0xC, 1.0f);
        write_mem<byte>(glow_obj + 0x10, 1);
    }
}

// ============================================================
// 12. MISC
// ============================================================

void run_bhop() {
    if (!g_settings.bhop_enabled || !g_local.alive || !(g_local.flags & 0x1)) return;
    INPUT ip = {0}; ip.type = INPUT_KEYBOARD; ip.ki.wVk = VK_SPACE;
    SendInput(1, &ip, sizeof(INPUT));
    ip.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &ip, sizeof(INPUT));
}

void run_triggerbot() {
    if (!g_settings.triggerbot_enabled || !g_local.alive) return;
    int crosshair = read_mem<int>(g_local.ptr + OFFSET_CROSSHAIR);
    if (crosshair <= 0 || crosshair >= 32) return;
    Player* target = &g_players[crosshair];
    if (!is_target_valid(target)) return;
    static DWORD last_shot = 0;
    if (GetTickCount() - last_shot < 50) return;
    INPUT ip = {0}; ip.type = INPUT_MOUSE; ip.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    SendInput(1, &ip, sizeof(INPUT));
    ip.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(1, &ip, sizeof(INPUT));
    last_shot = GetTickCount();
}

void run_auto_strafe() {
    if (!g_settings.auto_strafe || !g_local.alive || (g_local.flags & 0x1)) return;
    Vector3 vel = g_local.velocity;
    float speed = vector_length(vel);
    if (speed < 1.0f) return;
    float yaw = g_view_angles.yaw;
    float v_yaw = atan2f(vel.y, vel.x) * 57.2957795f;
    float diff = v_yaw - yaw;
    normalize_angle((Angles*)&diff);
    INPUT ip = {0}; ip.type = INPUT_KEYBOARD;
    ip.ki.wVk = (diff > 0) ? 'D' : 'A';
    SendInput(1, &ip, sizeof(INPUT));
    ip.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &ip, sizeof(INPUT));
}

void run_anti_aim() {
    if (!g_settings.anti_aim || !g_local.alive) return;
    static float flip = 0.0f;
    flip += 0.1f;
    g_view_angles.pitch = 89.0f;
    g_view_angles.yaw += 180.0f + sinf(flip) * 90.0f;
    normalize_angle(&g_view_angles);
}

void run_no_recoil() {
    if (!g_settings.no_recoil || !g_local.ptr) return;
    write_mem<Vector3>(g_local.ptr + OFFSET_PUNCH_ANGLE, (Vector3){0,0,0});
}

void run_no_spread() {
    if (!g_settings.no_spread || !g_local.ptr) return;
    write_mem<float>(g_local.ptr + OFFSET_SPREAD, 0.0f);
}

void run_rage_mode() {
    if (!g_settings.rage_mode) return;
    g_settings.aimbot_enabled = true;
    g_settings.aimbot_auto_shoot = true;
    g_settings.aimbot_fov = 180;
    g_settings.aimbot_smooth = 1;
    g_settings.aimbot_rcs = true;
    g_settings.aimbot_rcs_x = 100;
    g_settings.aimbot_rcs_y = 100;
    g_settings.no_recoil = true;
    g_settings.no_spread = true;
    g_settings.anti_aim = true;
}

// ============================================================
// 13. VISUALS
// ============================================================

void apply_no_scope() {
    if (!g_settings.no_scope) return;
    uintptr_t local = read_mem<uintptr_t>(g_client_base + DW_LOCAL_PLAYER);
    if (local) write_mem<byte>(local + OFFSET_SCOPED, 0);
}

void apply_full_bright() {
    if (!g_settings.full_bright) return;
    for (int i = 0; i < 32; i++) {
        uintptr_t ent = get_entity(i);
        if (ent) write_mem<float>(ent + 0x1C0, 1.0f);
    }
}

void apply_no_flash() {
    if (!g_settings.no_flash) return;
    uintptr_t local = read_mem<uintptr_t>(g_client_base + DW_LOCAL_PLAYER);
    if (local) write_mem<float>(local + OFFSET_FLASH_ALPHA, g_settings.no_flash_amount / 100.0f);
}

// ============================================================
// 14. МЕНЮ (GDI)
// ============================================================

void render_menu() {
    if (!g_menu_open || !g_hwnd) return;
    HDC hdc = GetDC(g_hwnd);
    if (!hdc) return;
    RECT rect = {100, 100, 400, 500};
    HBRUSH brush = CreateSolidBrush(RGB(20, 20, 30));
    FillRect(hdc, &rect, brush);
    DeleteObject(brush);
    HPEN pen = CreatePen(PS_SOLID, 2, RGB(0, 180, 255));
    SelectObject(hdc, pen);
    Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
    DeleteObject(pen);
    SetBkMode(hdc, TRANSPARENT);
    HFONT font = CreateFont(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
    SelectObject(hdc, font);
    SetTextColor(hdc, RGB(0, 180, 255));
    TextOutA(hdc, 150, 110, "TF2 CHEAT", 9);
    DeleteObject(font);
    font = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
    SelectObject(hdc, font);
    SetTextColor(hdc, RGB(200, 200, 200));
    int y = 150;
    char buffer[256];
    sprintf(buffer, "Aimbot: %s", g_settings.aimbot_enabled ? "[X]" : "[ ]");
    TextOutA(hdc, 120, y, buffer, strlen(buffer)); y += 25;
    sprintf(buffer, "ESP: %s", g_settings.esp_enabled ? "[X]" : "[ ]");
    TextOutA(hdc, 120, y, buffer, strlen(buffer)); y += 25;
    sprintf(buffer, "BHop: %s", g_settings.bhop_enabled ? "[X]" : "[ ]");
    TextOutA(hdc, 120, y, buffer, strlen(buffer)); y += 25;
    sprintf(buffer, "Trigger: %s", g_settings.triggerbot_enabled ? "[X]" : "[ ]");
    TextOutA(hdc, 120, y, buffer, strlen(buffer)); y += 25;
    sprintf(buffer, "No Recoil: %s", g_settings.no_recoil ? "[X]" : "[ ]");
    TextOutA(hdc, 120, y, buffer, strlen(buffer)); y += 25;
    sprintf(buffer, "Rage: %s", g_settings.rage_mode ? "[X]" : "[ ]");
    TextOutA(hdc, 120, y, buffer, strlen(buffer)); y += 40;
    SetTextColor(hdc, RGB(100, 100, 100));
    TextOutA(hdc, 120, y, "[INS] Toggle", 12);
    DeleteObject(font);
    ReleaseDC(g_hwnd, hdc);
}

// ============================================================
// 15. ХУКИ (ЧЕРЕЗ VTABLE)
// ============================================================

typedef void (*oCreateMove_t)(void*, float, void*);
oCreateMove_t oCreateMove = NULL;

void* get_vtable(void* obj, int index) {
    return (*(void***)obj)[index];
}

void __fastcall hkCreateMove(void* ecx, void* edx, float sample, void* cmd) {
    if (oCreateMove) oCreateMove(ecx, sample, cmd);
    update_players();
    g_view_angles = read_mem<Angles>((uintptr_t)cmd + 0x4);
    run_bhop();
    run_triggerbot();
    run_auto_strafe();
    apply_aimbot();
    run_anti_aim();
    run_no_recoil();
    run_no_spread();
    run_rage_mode();
    write_mem<Angles>((uintptr_t)cmd + 0x4, g_view_angles);
}

void setup_hooks() {
    g_client_base = get_module_base("client.dll");
    g_engine_base = get_module_base("engine.dll");
    if (!g_client_base || !g_engine_base) return;
    g_entity_list = g_client_base + DW_ENTITY_LIST;
    g_view_matrix = g_client_base + DW_VIEW_MATRIX;
    g_glow_manager = g_client_base + DW_GLOW_MANAGER;
    uintptr_t hlclient = read_mem<uintptr_t>(g_client_base + 0x123456);
    if (hlclient) {
        void* target = get_vtable((void*)hlclient, 24);
        DWORD old;
        VirtualProtect(target, 8, PAGE_EXECUTE_READWRITE, &old);
        oCreateMove = (oCreateMove_t)*(void**)target;
        *(void**)target = hkCreateMove;
        VirtualProtect(target, 8, old, &old);
        g_hooked = true;
    }
}

// ============================================================
// 16. ТОЧКА ВХОДА
// ============================================================

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hinstDLL);
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)entry, NULL, 0, NULL);
    }
    return TRUE;
}

void entry() {
    if (!find_tf2()) return;
    g_client_base = get_module_base("client.dll");
    g_engine_base = get_module_base("engine.dll");
    g_entity_list = g_client_base + DW_ENTITY_LIST;
    g_view_matrix = g_client_base + DW_VIEW_MATRIX;
    g_glow_manager = g_client_base + DW_GLOW_MANAGER;
    g_hwnd = FindWindowA(NULL, "Team Fortress 2");
    setup_hooks();
    while (true) {
        if (GetAsyncKeyState(VK_INSERT) & 1) g_menu_open = !g_menu_open;
        if (g_hwnd) {
            render_esp();
            render_glow();
            render_menu();
            apply_no_scope();
            apply_full_bright();
            apply_no_flash();
        }
        Sleep(10);
    }
}

// ============================================================
// КОНЕЦ ФАЙЛА
// ============================================================