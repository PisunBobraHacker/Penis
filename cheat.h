/*
 * ============================================================
 * CHEAT.H — TF2 Cheat Header
 * Complete header for tf2cheat.c
 * 15,000+ lines of pure power
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
#include <d3dx9.h>
#include <wininet.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <wincrypt.h>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shell32.lib")

// ============================================================
// 1. СМЕЩЕНИЯ TF2
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
#define OFFSET_DEATH_TIME    0x1B8
#define OFFSET_SPAWN_TIME    0x1B0
#define OFFSET_DEFUSING      0x1AC
#define OFFSET_PLANTING      0x1B0
#define OFFSET_HAS_BOMB      0x1B4
#define OFFSET_HAS_DEFUSE    0x1B8
#define OFFSET_HAS_HELMET    0x1BC
#define OFFSET_HAS_VEST      0x1C0
#define OFFSET_HAS_VESTHELM  0x1C4

// ============================================================
// 2. СТРУКТУРЫ
// ============================================================

typedef struct {
    float x, y, z;
} Vector3;

typedef struct {
    float pitch, yaw, roll;
} Angles;

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
    char weapon_name[32];
    char skin_name[64];
    char glove_name[32];
    char knife_name[32];
    char model_name[64];
} Player;

// ============================================================
// 3. НАСТРОЙКИ (ВСЕ ФИЧИ)
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
    bool aimbot_knife;
    bool aimbot_taser;
    
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
    bool esp_bomb;
    bool esp_defuse;
    bool esp_hostage;
    bool esp_zone;
    bool esp_radar;
    bool esp_sound;
    bool esp_velocity;
    bool esp_head_dot;
    bool esp_chams;
    int esp_chams_visible;
    int esp_chams_invisible;
    
    // Misc
    bool bhop_enabled;
    bool bhop_auto_strafe;
    bool bhop_auto_jump;
    bool bhop_always;
    bool triggerbot_enabled;
    int triggerbot_delay;
    int triggerbot_hitbox;
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
    bool anti_aim_leg;
    bool anti_aim_body;
    bool no_recoil;
    int no_recoil_stand;
    int no_recoil_crouch;
    int no_recoil_air;
    bool no_spread;
    int no_spread_stand;
    int no_spread_crouch;
    int no_spread_air;
    bool auto_peek;
    int auto_peek_distance;
    int auto_peek_height;
    bool auto_rocket_jump;
    int auto_rocket_jump_height;
    int auto_rocket_jump_distance;
    bool auto_pipe_det;
    int auto_pipe_det_radius;
    bool auto_air_stuck;
    bool fake_lag;
    int fake_lag_type;
    int fake_lag_amount;
    bool fake_duck;
    bool third_person;
    int third_person_distance;
    int third_person_angle;
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
    bool no_black;
    bool asphalt;
    bool glass;
    bool flat;
    int color_mod;
    int color_r;
    int color_g;
    int color_b;
    
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
    bool rage_force_unsafe;
    int rage_force_hitbox;
    int rage_force_target;
    int rage_force_damage;
    int rage_force_accuracy;
    int rage_force_hitchance;
    bool rage_force_autopeek;
    bool rage_force_autofire;
    bool rage_force_autoscope;
    bool rage_force_baim;
    bool rage_force_hs;
    bool rage_force_safe_point;
    
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
    int misc_spectator_list_style;
    bool misc_watermark;
    char misc_watermark_text[64];
    bool misc_fps_counter;
    int misc_fps_counter_style;
    bool misc_ping_counter;
    bool misc_net_graph;
    bool misc_console;
    bool misc_log;
    char misc_log_file[64];
    bool misc_config_save;
    bool misc_config_load;
    char misc_config_name[64];
    bool misc_screenshot;
    int misc_screenshot_format;
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
    bool misc_unlock_badge;
    bool misc_unlock_xp;
    bool misc_unlock_achievement_all;
    bool misc_unlock_achievement_auto;
    bool misc_unlock_achievement_random;
    bool misc_unlock_achievement_stealth;
    bool misc_unlock_achievement_legit;
    int misc_unlock_level;
    int misc_unlock_prestige;
    int misc_unlock_kills;
    int misc_unlock_heads;
    int misc_unlock_domination;
    int misc_unlock_revenge;
    int misc_unlock_playtime;
    int misc_unlock_wins;
    int misc_unlock_losses;
    int misc_unlock_ties;
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
    int misc_unlock_achievement_points;
    int misc_unlock_achievement_count;
    int misc_unlock_achievement_delay;
} Settings;

// ============================================================
// 4. ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ
// ============================================================

extern Settings g_settings;
extern Player g_players[64];
extern Player g_local;
extern Player g_target;
extern Player g_spectators[64];
extern Player g_achievements[1000];
extern Player g_weapons[100];
extern Player g_skins[1000];
extern Player g_models[100];
extern Player g_sounds[100];

extern uintptr_t g_client_base;
extern uintptr_t g_engine_base;
extern uintptr_t g_server_base;
extern uintptr_t g_entity_list;
extern uintptr_t g_view_matrix;
extern uintptr_t g_glow_manager;
extern uintptr_t g_local_player;
extern uintptr_t g_global_vars;

extern Angles g_view_angles;
extern Angles g_aim_angles;
extern Angles g_manual_angles;
extern Angles g_silent_angles;
extern Angles g_rage_angles;
extern Angles g_legit_angles;

extern bool g_menu_open;
extern bool g_loaded;
extern bool g_initialized;
extern bool g_hooked;
extern bool g_injected;
extern bool g_hidden;
extern bool g_protected;
extern bool g_bypassed;
extern bool g_updated;

extern HWND g_hwnd;
extern HWND g_console_hwnd;
extern HWND g_log_hwnd;
extern HWND g_menu_hwnd;

extern HANDLE g_process;
extern HANDLE g_thread;
extern HANDLE g_driver;
extern HANDLE g_mutex;

extern DWORD g_pid;
extern DWORD g_tid;
extern DWORD g_cpu;
extern DWORD g_ram;
extern DWORD g_vram;
extern DWORD g_disk;
extern DWORD g_network;
extern DWORD g_ping;
extern DWORD g_fps;
extern DWORD g_tick;
extern DWORD g_frame;
extern DWORD g_second;
extern DWORD g_minute;
extern DWORD g_hour;
extern DWORD g_day;
extern DWORD g_month;
extern DWORD g_year;

extern char g_config_path[MAX_PATH];
extern char g_log_path[MAX_PATH];
extern char g_update_path[MAX_PATH];
extern char g_unlock_path[MAX_PATH];
extern char g_rage_path[MAX_PATH];
extern char g_legit_path[MAX_PATH];

// ============================================================
// 5. ПРОТОТИПЫ ФУНКЦИЙ
// ============================================================

// ===== CORE =====
bool find_tf2();
uintptr_t get_module_base(const char* module);
void setup_hooks();
void remove_hooks();
void entry();

// ===== MEMORY =====
template<typename T> T read_mem(uintptr_t addr);
template<typename T> void write_mem(uintptr_t addr, T val);
std::string read_string(uintptr_t addr, size_t len);
void write_string(uintptr_t addr, const char* str);

// ===== ENTITY =====
uintptr_t get_entity(int index);
int get_entity_index(uintptr_t ent);
bool is_visible(uintptr_t ent);
void update_players();

// ===== MATH =====
Vector3 vector_add(Vector3 a, Vector3 b);
Vector3 vector_sub(Vector3 a, Vector3 b);
Vector3 vector_mul(Vector3 a, float s);
Vector3 vector_div(Vector3 a, float s);
float vector_dot(Vector3 a, Vector3 b);
Vector3 vector_cross(Vector3 a, Vector3 b);
float vector_length(Vector3 v);
float vector_distance(Vector3 a, Vector3 b);
Vector3 vector_normalize(Vector3 v);
Vector3 vector_lerp(Vector3 a, Vector3 b, float t);
Vector3 vector_smooth(Vector3 a, Vector3 b, float smooth);
Angles calc_angle(Vector3 src, Vector3 dst);
Vector3 calc_vector(Angles ang);
void normalize_angle(Angles* ang);
void clamp_angle(Angles* ang);
void smooth_angle(Angles* from, Angles* to, float smooth);
float get_fov(Angles from, Angles to);
float get_fov_to_pos(Vector3 src, Angles ang, Vector3 dst);
bool world_to_screen(Vector3 world, Vector3* screen);

// ===== AIMBOT =====
bool is_target_valid(Player* player);
Vector3 get_hitbox_pos(Player* player, int bone);
float get_target_damage(Player* player, Vector3 src, Vector3 dst);
bool can_hit_player(Player* player, Vector3 src, Vector3 dst);
Player* get_best_target(int* index);
void apply_aimbot();

// ===== ESP =====
void render_esp();
void render_glow();

// ===== MISC =====
void run_bhop();
void run_triggerbot();
void run_auto_strafe();
void run_anti_aim();
void run_no_recoil();
void run_no_spread();
void run_auto_peek();
void run_auto_rocket_jump();
void run_auto_pipe_det();
void run_auto_air_stuck();
void run_fake_lag();
void run_fake_duck();
void run_third_person();
void run_slow_walk();
void run_quick_switch();

// ===== VISUALS =====
void apply_no_scope();
void apply_no_zoom();
void apply_full_bright();
void apply_transparent_players();
void apply_wireframe_smoke();
void apply_remove_fog();
void apply_remove_skybox();
void apply_night_mode();
void apply_no_flash();
void apply_no_smoke();
void apply_no_blood();
void apply_no_visual_recoil();
void apply_no_hands();
void apply_no_world();
void apply_no_sky();
void apply_no_water();
void apply_no_black();
void apply_asphalt();
void apply_glass();
void apply_flat();
void apply_color_mod();

// ===== RAGE =====
void run_rage_mode();

// ===== SKINS =====
void run_skin_changer();
void run_model_changer();

// ===== MEME =====
void draw_penis(ImDrawList* draw, Vector3 pos);
void draw_breasts(ImDrawList* draw, Vector3 pos);
void draw_butt(ImDrawList* draw, Vector3 pos);
void draw_hearts(ImDrawList* draw, Vector3 pos);
void apply_rainbow();
void draw_custom_crosshair(ImDrawList* draw);

// ===== HIT SOUNDS =====
void play_hit_sound();
void play_kill_sound();
void draw_hit_marker(ImDrawList* draw);
void draw_damage_indicator(ImDrawList* draw);

// ===== SPECTATORS =====
void render_spectator_list();

// ===== WATERMARK =====
void render_watermark();
void render_fps_counter();
void render_ping_counter();
void render_net_graph();

// ===== UNLOCK =====
void unlock_all();

// ===== CONFIG =====
void save_config();
void load_config();

// ===== LOG =====
void log_message(const char* msg, ...);

// ===== UPDATE =====
void check_updates();

// ===== RENDER =====
void render_all();

// ===== MENU =====
void render_menu();
void setup_style();

// ============================================================
// 6. ХУКИ
// ============================================================

typedef void (*oCreateMove_t)(void*, float, void*);
typedef long (*oEndScene_t)(IDirect3DDevice9*);

extern oCreateMove_t oCreateMove;
extern oEndScene_t oEndScene;

void __fastcall hkCreateMove(void* ecx, void* edx, float sample, void* cmd);
long __stdcall hkEndScene(IDirect3DDevice9* device);

// ============================================================
// 7. IMGUI ВКЛЮЧЕНИЯ
// ============================================================

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx9.h"

// ============================================================
// КОНЕЦ HEADER
// ============================================================

#endif // CHEAT_H
