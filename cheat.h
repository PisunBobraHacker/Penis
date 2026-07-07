/*
 * ============================================================
 * CHEAT.H — TF2 Cheat Header
 * Исправлено: убраны конфликты с ImGui
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
// 1. СТРУКТУРЫ (БЕЗ КОНФЛИКТОВ С IMGUI)
// ============================================================

typedef struct {
    float x, y, z;
} Vector3;

typedef struct {
    float pitch, yaw, roll;
} Angles;

// НЕ определяем ImVec2 и ImColor — они уже есть в ImGui

// ============================================================
// 2. СМЕЩЕНИЯ TF2
// ============================================================

#define OFFSET_HEALTH        0xA0
#define OFFSET_ARMOR         0xA4
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
#define OFFSET_ZOOMED        0x1A0
#define OFFSET_SCOPED        0x19C
#define OFFSET_FLASH_ALPHA   0x1A4
#define OFFSET_SMOKE_ALPHA   0x1A8
#define OFFSET_PUNCH_ANGLE   0x4C
#define OFFSET_AIM_PUNCH     0x50
#define OFFSET_VIEW_PUNCH    0x54
#define OFFSET_SPREAD        0x58
#define OFFSET_CROSSHAIR     0xB9C
#define OFFSET_GLOW_INDEX    0xA8
#define OFFSET_NAME          0x140
#define OFFSET_ANGLE         0x48C

// ============================================================
// 3. СТРУКТУРА ИГРОКА
// ============================================================

typedef struct {
    uintptr_t ptr;
    int health;
    int armor;
    int team;
    int flags;
    int ammo;
    int weapon_id;
    int hitbox;
    float zoom_level;
    float flash_alpha;
    float smoke_alpha;
    Vector3 origin;
    Vector3 view_offset;
    Vector3 velocity;
    Vector3 punch_angle;
    Vector3 aim_punch;
    Vector3 view_punch;
    bool alive;
    bool dormant;
    bool is_enemy;
    bool visible;
    bool scoped;
    bool zoomed;
    bool flashed;
    bool smoked;
    bool defusing;
    bool planting;
    bool has_bomb;
    bool has_defuse;
    bool has_helmet;
    bool has_vest;
    bool has_vesthelm;
    char name[32];
    int kills;
    int deaths;
    int assists;
    int headshots;
    int domination;
    int revenge;
    int streak;
    int best_streak;
    int shots;
    int hits;
    int damage;
    float accuracy;
    float rating;
    char rank[32];
    char medal[32];
    char trophy[32];
} Player;

// ============================================================
// 4. НАСТРОЙКИ
// ============================================================

typedef struct {
    // Aimbot
    bool aimbot_enabled;
    bool aimbot_visible_check;
    bool aimbot_auto_shoot;
    bool aimbot_auto_scope;
    bool aimbot_ignore_cloaked;
    bool aimbot_ignore_disguised;
    bool aimbot_priority_head;
    int aimbot_fov;
    int aimbot_smooth;
    int aimbot_bone;
    int aimbot_min_damage;
    bool aimbot_rcs;
    int aimbot_rcs_x;
    int aimbot_rcs_y;
    
    // ESP
    bool esp_enabled;
    bool esp_box;
    bool esp_health_bar;
    bool esp_health_text;
    bool esp_name;
    bool esp_weapon;
    bool esp_distance;
    bool esp_snaplines;
    bool esp_glow;
    bool esp_visible_only;
    bool esp_team_check;
    
    // Misc
    bool bhop_enabled;
    bool bhop_auto_strafe;
    bool triggerbot_enabled;
    int triggerbot_delay;
    bool auto_strafe;
    bool anti_aim;
    int anti_aim_pitch;
    int anti_aim_yaw;
    bool anti_aim_desync;
    bool no_recoil;
    bool no_spread;
    bool fake_lag;
    int fake_lag_amount;
    bool fake_duck;
    bool third_person;
    int third_person_distance;
    
    // Visuals
    bool no_scope;
    bool no_zoom;
    bool full_bright;
    bool remove_fog;
    bool remove_skybox;
    bool night_mode;
    int night_mode_brightness;
    bool no_flash;
    int no_flash_amount;
    bool no_smoke;
    bool no_blood;
    
    // Rage
    bool rage_mode;
    bool rage_auto_wall;
    int rage_auto_wall_min_damage;
    bool rage_resolver;
    bool rage_double_tap;
    int rage_double_tap_shift;
    bool rage_anti_exploit;
    bool rage_force_hs;
    
    // Skins
    bool skin_changer;
    int skin_changer_weapon;
    int skin_changer_skin;
    int skin_changer_quality;
    float skin_changer_wear;
    int skin_changer_seed;
    int skin_changer_stattrak;
    
    // Meme
    bool misc_draw_penis;
    int misc_draw_penis_size;
    int misc_draw_penis_color;
    bool misc_draw_breasts;
    int misc_draw_breasts_size;
    int misc_draw_breasts_color;
    bool misc_rainbow;
    int misc_rainbow_speed;
    bool misc_custom_crosshair;
    int misc_custom_crosshair_color;
    int misc_custom_crosshair_size;
    int misc_custom_crosshair_gap;
    bool misc_hit_sound;
    int misc_hit_sound_volume;
    bool misc_kill_sound;
    int misc_kill_sound_volume;
    bool misc_hit_marker;
    int misc_hit_marker_time;
    bool misc_watermark;
    char misc_watermark_text[64];
    bool misc_fps_counter;
    bool misc_ping_counter;
    bool misc_log;
    char misc_log_file[64];
    bool misc_config_save;
    bool misc_config_load;
    char misc_config_name[64];
    
    // Unlock
    bool misc_unlock_all;
    bool misc_unlock_crates;
    bool misc_unlock_achievements;
    bool misc_unlock_stats;
    bool misc_unlock_rank;
    int misc_unlock_level;
    int misc_unlock_prestige;
    int misc_unlock_kills;
    int misc_unlock_heads;
    int misc_unlock_playtime;
    int misc_unlock_wins;
    int misc_unlock_streak;
    int misc_unlock_hs;
    int misc_unlock_damage;
    float misc_unlock_accuracy;
    float misc_unlock_rating;
    char misc_unlock_rank_name[32];
    char misc_unlock_medal[32];
    char misc_unlock_trophy[32];
} Settings;

// ============================================================
// 5. ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ
// ============================================================

extern Settings g_settings;
extern Player g_players[64];
extern Player g_local;
extern uintptr_t g_client_base;
extern uintptr_t g_engine_base;
extern uintptr_t g_entity_list;
extern uintptr_t g_view_matrix;
extern uintptr_t g_glow_manager;
extern Angles g_view_angles;
extern bool g_menu_open;
extern HWND g_hwnd;

// ============================================================
// 6. ПРОТОТИПЫ ФУНКЦИЙ
// ============================================================

// Core
bool find_tf2();
uintptr_t get_module_base(const char* module);
void setup_hooks();
void remove_hooks();
void entry();

// Entity
uintptr_t get_entity(int index);
void update_players();
bool is_visible(uintptr_t ent);

// Math
Vector3 vector_add(Vector3 a, Vector3 b);
Vector3 vector_sub(Vector3 a, Vector3 b);
Vector3 vector_mul(Vector3 a, float s);
float vector_dot(Vector3 a, Vector3 b);
float vector_length(Vector3 v);
float vector_distance(Vector3 a, Vector3 b);
Vector3 vector_normalize(Vector3 v);
Angles calc_angle(Vector3 src, Vector3 dst);
void normalize_angle(Angles* ang);
void smooth_angle(Angles* from, Angles* to, float smooth);
float get_fov(Angles from, Angles to);
bool world_to_screen(Vector3 world, Vector3* screen);

// Aimbot
bool is_target_valid(Player* player);
Vector3 get_hitbox_pos(Player* player, int bone);
Player* get_best_target(int* index);
void apply_aimbot();

// ESP
void render_esp();
void render_glow();

// Misc
void run_bhop();
void run_triggerbot();
void run_auto_strafe();
void run_anti_aim();
void run_no_recoil();
void run_no_spread();
void run_fake_lag();
void run_fake_duck();
void run_third_person();
void run_rage_mode();
void run_skin_changer();
void run_model_changer();

// Visuals
void apply_no_scope();
void apply_no_zoom();
void apply_full_bright();
void apply_remove_fog();
void apply_remove_skybox();
void apply_night_mode();
void apply_no_flash();
void apply_no_smoke();
void apply_no_blood();

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
// 7. IMGUI — ПРАВИЛЬНОЕ ПОДКЛЮЧЕНИЕ
// ============================================================

#ifdef __cplusplus
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx9.h"
#endif

#endif // CHEAT_H