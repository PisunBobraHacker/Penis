/*
 * ============================================================
 * CHEAT.H — TF2 Cheat Header
 * ============================================================
 */

#ifndef CHEAT_H
#define CHEAT_H

#include <windows.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <intrin.h>
#include <d3d9.h>

// ============================================================
// 1. СТРУКТУРЫ
// ============================================================

typedef struct {
    float x, y, z;
} Vector3;

typedef struct {
    float pitch, yaw, roll;
} Angles;

// ============================================================
// 2. СТРУКТУРА ИГРОКА
// ============================================================

typedef struct {
    uintptr_t ptr;
    int health;
    int armor;
    int team;
    int flags;
    int ammo;
    int weapon_id;
    Vector3 origin;
    Vector3 view_offset;
    Vector3 velocity;
    Vector3 punch_angle;
    bool alive;
    bool dormant;
    bool is_enemy;
    bool visible;
    bool scoped;
    bool zoomed;
    char name[32];
} Player;

// ============================================================
// 3. НАСТРОЙКИ
// ============================================================

typedef struct {
    bool aimbot_enabled;
    bool aimbot_visible_check;
    bool aimbot_auto_shoot;
    int aimbot_fov;
    int aimbot_smooth;
    int aimbot_bone;
    
    bool esp_enabled;
    bool esp_box;
    bool esp_health_bar;
    bool esp_name;
    bool esp_snaplines;
    bool esp_glow;
    
    bool bhop_enabled;
    bool triggerbot_enabled;
    bool no_recoil;
    bool no_spread;
    bool anti_aim;
    bool fake_lag;
    bool third_person;
    
    bool rage_mode;
    bool rage_auto_wall;
    
    bool no_scope;
    bool full_bright;
    bool remove_fog;
    bool night_mode;
    
    bool skin_changer;
    int skin_changer_weapon;
    int skin_changer_skin;
    
    bool misc_watermark;
    bool misc_fps_counter;
    bool misc_rainbow;
} Settings;

// ============================================================
// 4. ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ
// ============================================================

extern Settings g_settings;
extern Player g_players[64];
extern Player g_local;
extern uintptr_t g_client_base;
extern uintptr_t g_engine_base;
extern uintptr_t g_entity_list;
extern uintptr_t g_view_matrix;
extern Angles g_view_angles;
extern bool g_menu_open;
extern HWND g_hwnd;

// ============================================================
// 5. ПРОТОТИПЫ
// ============================================================

// Core
bool find_tf2();
uintptr_t get_module_base(const char* module);
void setup_hooks();

// Entity
uintptr_t get_entity(int index);
void update_players();
bool is_visible(uintptr_t ent);

// Math
Vector3 vector_add(Vector3 a, Vector3 b);
Vector3 vector_sub(Vector3 a, Vector3 b);
float vector_length(Vector3 v);
float vector_distance(Vector3 a, Vector3 b);
Vector3 vector_normalize(Vector3 v);
Angles calc_angle(Vector3 src, Vector3 dst);
void normalize_angle(Angles* ang);
bool world_to_screen(Vector3 world, Vector3* screen);

// Aimbot
void apply_aimbot();

// ESP
void render_esp();
void render_glow();

// Misc
void run_bhop();
void run_triggerbot();
void run_anti_aim();
void run_rage_mode();

// Visuals
void apply_no_scope();
void apply_full_bright();

// Menu
void render_menu();
void setup_style();

// Hooks
typedef void (*oCreateMove_t)(void*, float, void*);
typedef long (*oEndScene_t)(IDirect3DDevice9*);

extern oCreateMove_t oCreateMove;
extern oEndScene_t oEndScene;

void __fastcall hkCreateMove(void* ecx, void* edx, float sample, void* cmd);
long __stdcall hkEndScene(IDirect3DDevice9* device);

// ============================================================
// 6. IMGUI
// ============================================================

#ifdef __cplusplus
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx9.h"
#endif

#endif // CHEAT_H