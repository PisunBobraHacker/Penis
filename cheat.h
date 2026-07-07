/*
 * ============================================================
 * CHEAT.H — TF2 Cheat Header
 * Исправлено: убраны устаревшие заголовки
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

// ТОЛЬКО d3d9.h, БЕЗ d3dx9.h
#include <d3d9.h>

// Заменяем устаревшие функции на свои
typedef struct {
    float x, y, z;
} Vector3;

typedef struct {
    float pitch, yaw, roll;
} Angles;

// D3DCOLOR для цветов
#ifndef D3DCOLOR
typedef DWORD D3DCOLOR;
#endif

// Структуры для ImGui
typedef struct {
    float x, y;
} ImVec2;

typedef struct {
    float r, g, b, a;
} ImColor;

// Остальные структуры...
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
// НАСТРОЙКИ (все фичи)
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
    bool aimbot_priority_visible;
    bool aimbot_priority_distance;
    bool aimbot_priority_health;
    int aimbot_fov;
    int aimbot_smooth;
    int aimbot_bone;
    int aimbot_min_damage;
    bool aimbot_auto_switch;
    bool aimbot_auto_fire;
    bool aimbot_rcs;
    int aimbot_rcs_x;
    int aimbot_rcs_y;
    int aimbot_accuracy;
    int aimbot_hitchance;
    int aimbot_shot_delay;
    
    // ESP
    bool esp_enabled;
    bool esp_box;
    bool esp_box_3d;
    bool esp_box_corner;
    bool esp_health_bar;
    bool esp_health_text;
    bool esp_armor_bar;
    bool esp_name;
    bool esp_weapon;
    bool esp_distance;
    bool esp_snaplines;
    bool esp_glow;
    int esp_glow_color;
    bool esp_visible_only;
    bool esp_team_check;
    bool esp_rank;
    bool esp_scope;
    bool esp_head_dot;
    
    // Misc
    bool bhop_enabled;
    bool bhop_auto_strafe;
    bool bhop_always;
    bool triggerbot_enabled;
    int triggerbot_delay;
    bool triggerbot_scope;
    bool triggerbot_zoom;
    bool auto_strafe;
    int auto_strafe_type;
    bool anti_aim;
    int anti_aim_pitch;
    int anti_aim_yaw;
    int anti_aim_type;
    bool anti_aim_desync;
    bool anti_aim_fake;
    bool no_recoil;
    bool no_spread;
    bool auto_peek;
    int auto_peek_distance;
    bool auto_rocket_jump;
    bool auto_pipe_det;
    int auto_pipe_det_radius;
    bool auto_air_stuck;
    bool fake_lag;
    int fake_lag_amount;
    bool fake_duck;
    bool third_person;
    int third_person_distance;
    bool slow_walk;
    int slow_walk_speed;
    bool quick_switch;
    int quick_switch_weapon;
    
    // Visuals
    bool no_scope;
    bool no_zoom;
    bool full_bright;
    bool transparent_players;
    int transparent_players_amount;
    bool wireframe_smoke;
    bool remove_fog;
    bool remove_skybox;
    bool night_mode;
    int night_mode_brightness;
    bool no_flash;
    int no_flash_amount;
    bool no_smoke;
    bool no_blood;
    bool no_visual_recoil;
    bool no_hands;
    bool no_world;
    bool no_sky;
    bool no_water;
    
    // Rage
    bool rage_mode;
    bool rage_auto_wall;
    int rage_auto_wall_min_damage;
    bool rage_resolver;
    int rage_resolver_type;
    bool rage_double_tap;
    int rage_double_tap_shift;
    bool rage_anti_exploit;
    bool rage_force_safe;
    bool rage_force_autofire;
    bool rage_force_autoscope;
    bool rage_force_hs;
    
    // Skins
    bool skin_changer;
    int skin_changer_weapon;
    int skin_changer_skin;
    int skin_changer_quality;
    float skin_changer_wear;
    int skin_changer_seed;
    int skin_changer_stattrak;
    bool model_changer;
    int model_changer_player;
    int model_changer_weapon;
    int model_changer_hands;
    int model_changer_knife;
    int model_changer_glove;
    
    // Meme
    bool misc_draw_penis;
    int misc_draw_penis_size;
    int misc_draw_penis_color;
    bool misc_draw_breasts;
    int misc_draw_breasts_size;
    int misc_draw_breasts_color;
    bool misc_draw_butt;
    int misc_draw_butt_size;
    int misc_draw_butt_color;
    bool misc_draw_hearts;
    int misc_draw_hearts_size;
    int misc_draw_hearts_color;
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
    bool misc_damage_indicator;
    int misc_damage_indicator_time;
    bool misc_spectator_list;
    bool misc_watermark;
    char misc_watermark_text[64];
    bool misc_fps_counter;
    int misc_fps_counter_style;
    bool misc_ping_counter;
    bool misc_net_graph;
    bool misc_log;
    char misc_log_file[64];
    bool misc_config_save;
    bool misc_config_load;
    char misc_config_name[64];
    bool misc_auto_update;
    char misc_update_url[256];
    bool misc_debug;
    bool misc_debug_log;
    bool misc_debug_console;
    
    // Unlock
    bool misc_unlock_all;
    bool misc_unlock_crates;
    bool misc_unlock_paints;
    bool misc_unlock_stickers;
    bool misc_unlock_music;
    bool misc_unlock_achievements;
    bool misc_unlock_stats;
    bool misc_unlock_rank;
    int misc_unlock_level;
    int misc_unlock_prestige;
    int misc_unlock_kills;
    int misc_unlock_heads;
    int misc_unlock_domination;
    int misc_unlock_revenge;
    int misc_unlock_playtime;
    int misc_unlock_wins;
    int misc_unlock_streak;
    int misc_unlock_best_streak;
    int misc_unlock_hs;
    int misc_unlock_shots;
    int misc_unlock_hits;
    int misc_unlock_damage;
    float misc_unlock_accuracy;
    float misc_unlock_rating;
    char misc_unlock_rank_name[32];
    char misc_unlock_medal[32];
    char misc_unlock_trophy[32];
    bool misc_unlock_achievement_all;
} Settings;

// ============================================================
// ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ
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
extern HANDLE g_process;

// ============================================================
// ПРОТОТИПЫ ФУНКЦИЙ
// ============================================================

// Core
bool find_tf2();
uintptr_t get_module_base(const char* module);
void setup_hooks();
void remove_hooks();
void entry();

// Memory
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
void apply_transparent_players();
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
// IMGUI ВКЛЮЧЕНИЯ
// ============================================================

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx9.h"

#endif // CHEAT_H