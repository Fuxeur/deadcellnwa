/* DEAD CELLS - NUMWORKS EDITION : en-tete commun. C99, entiers uniquement (pas de flottants), pas d'allocation dynamique.
   Projet de fan inspire du genre "roguelite metroidvania" ; code, sprites et niveaux 100 % originaux. */
#ifndef DC_H
#define DC_H
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "dc_assets.h"

#define SCR_W 320
#define SCR_H 240
#define STRIP_H 16                 /* rendu par bandes de 16 lignes : aucun tampon plein ecran en RAM */
#define TS 16
#define LV_H 15
#define ROOM_W 20
#define MAX_ROOMS 8
#define LV_W (ROOM_W * MAX_ROOMS)
#define NZONES 5
#define F(px) ((px) * 16)          /* pixels -> 1/16 pixel */
#define PX(f) ((f) >> 4)

#define MAX_EN 32
#define MAX_PR 24
#define MAX_PA 56
#define MAX_PK 32

/* touches (masque de bits rempli par la plateforme) */
enum { K_L = 1, K_R = 2, K_U = 4, K_D = 8, K_OK = 16, K_BACK = 32, K_SWAP = 64, K_FLASK = 128, K_PAUSE = 256 };

enum { TL_EMPTY, TL_SOLID, TL_PLAT, TL_SPIKE, TL_TOXIC, TL_DOOR, TL_GATE, TL_TORCH };
enum { SC_TITLE, SC_PLAY, SC_PAUSE, SC_SHOP, SC_OPTIONS, SC_HELP, SC_DEAD, SC_WIN, SC_MUT };
enum { ET_JAIL, ET_XBOW, ET_RAT, ET_BRUTE, ET_SLIME, ET_GREN, ET_SHLD, ET_TOXIC, ET_BAT, ET_GOB, ET_KING, ET_COUNT };
enum { WP_SWORD, WP_DAGGER, WP_RAPIER, WP_HAMMER, WP_SHIELD, WP_BOW, WP_XBOW, WP_FIRE, WP_ELEC, WP_SCYTHE, WP_COUNT };
enum { UP_HP, UP_DMG, UP_FLASK, UP_DODGE, UP_LOOT, UP_QUAL, UP_COUNT };
enum { MU_REVENGE, MU_INSTINCT, MU_BLEED, MU_DISENGAGE, MU_COMBO, MU_CARAPACE, MU_CALM, MU_ADREN, MU_GLUTTON, MU_LUCK, MU_COUNT };
enum { PS_NORMAL, PS_ATK, PS_ROLL, PS_HURT, PS_DEAD };
enum { ES_PATROL, ES_CHASE, ES_WIND, ES_ATK, ES_REC, ES_HURT, ES_DEAD, ES_SLEEP, ES_IDLE, ES_AIR };
enum { PJ_ARROW, PJ_PARROW, PJ_FIRE, PJ_BOLT, PJ_SPIT, PJ_WAVE, PJ_BOMB, PJ_CLOUD, PJ_XBOLT };
enum { PK_COIN, PK_HEART, PK_FLASK, PK_WPN, PK_CHEST, PK_CELL, PK_BLUE, PK_NPC };
enum { PA_SPARK, PA_SMOKE, PA_EMBER, PA_FX, PA_NUM, PA_LINE };
enum { WK_MELEE, WK_BOW, WK_FIRE, WK_ELEC, WK_SHIELD, WK_DUAL };
enum { LG_NONE, LG_IGNITE, LG_VAMP, LG_CRIT3 };
enum { RW_UP, RW_BP, RW_WLVL, RW_WQ, RW_BACK };

/* caracteres accentues de la police (codes octaux) */
#define EAC "\200"
#define EGR "\201"
#define AGR "\202"
#define ECI "\203"
#define CCE "\204"
#define ACI "\205"
#define ICI "\206"
#define ALEFT "\210"
#define ARIGHT "\211"
#define AUP "\212"
#define ADOWN "\213"
#define HEARTC "\214"

typedef struct { uint8_t total, a0, a1; int8_t lunge; uint8_t dmg, reach; int8_t y0, y1; uint8_t kb, stun; } Atk;
typedef struct { const char *name; uint8_t kind, leg; Atk l, h, b; } Wpn;
typedef struct { uint8_t id, lvl, q; } Item;       /* niveau 1..15 (I..XV) ; qualite 0 rien, 1 +, 2 ++, 3 S, 4 L */

typedef struct {
    int32_t x, y, vx, vy;          /* (x,y) = centre-bas de la boite, en 1/16 px */
    int16_t hp, maxhp, t, cd, flash, stun, anim, home, burn;
    uint8_t type, st, skin, aux, alive, ground, hit, elite;
    int8_t dir;
} Ent;

typedef struct {
    int32_t x, y, vx, vy;
    int16_t life, dmg;
    uint8_t type, owner, pierce, anim;
    int8_t dir;
    uint32_t hm;
} Proj;

typedef struct {
    int32_t x, y;
    int16_t vx, vy;
    uint8_t life, max, kind, col, sb, nf, val;
    int16_t num;
} Part;

typedef struct {
    int32_t x, y, vx, vy;
    uint8_t kind, val, life, st, ground, lvl, q;
} Pick;

typedef struct {
    int32_t x, y, vx, vy;
    int16_t hp, maxhp, t, atk_t, inv, coyote, jbuf, roll_cd, hold, drop_t, atk_cd, dead_t, buf_atk, anim, flip_t, swap_t;
    int16_t guard_t, combo_t, mt_veng, mt_dis, mt_calm, mt_adr, dual_t, buff_pct, kills;
    uint8_t st, ground, jumps, cur, flasks, maxflasks, heavy, atk_b, last_atk, combo, crit_next, guard, crit3;
    int8_t dir;
    Item w[2];
    uint32_t hitmask;
} Player;

typedef struct {                   /* sauvegarde persistante : progression, ameliorations, schemas, stats, parametres */
    uint16_t magic;
    uint8_t version, opt_fx, opt_easy, best_zone;
    uint16_t unlocked, found;
    uint8_t up[UP_COUNT];
    uint8_t pad[4];
    uint32_t cells, kills;
    uint16_t runs, wins, deaths, crc;
} Save;
#define SAVE_MAGIC 0xDC6B

typedef struct {
    uint8_t scene, zone, nrooms, quit, arena_on, boss_alive, victory, save_ok, passage, mut_pending, in_run;
    uint8_t map[LV_H][LV_W];
    int16_t lvw;
    Ent en[MAX_EN];
    Proj pr[MAX_PR];
    Part pa[MAX_PA];
    Pick pk[MAX_PK];
    Player p;
    int32_t cam_x;
    int16_t shake, hitstop, frame, banner_t, hint_t, boss_idx, arena_x0, door_tx, door_ty, msg_t, win_t;
    int16_t torch_n, torch_x[24], torch_y[24];
    int16_t menu_sel, menu_sub;
    uint16_t keys, prev, pressed, bp_carry;
    uint32_t rng, run_time;
    int32_t run_gold, run_kills, cells_run, bank_gain, bank_lost;
    uint8_t mut[4], mut_n, mut_off[3], reroll_n, help_page;
    char banner[40], msg[32];
    Save sv;
} Game;

extern Game G;
extern const Wpn WPN[WP_COUNT];
extern const char *const ZONE_NAME[NZONES];
extern const char *const MU_NAME[MU_COUNT];
extern const char *const MU_DESC[MU_COUNT];
extern const char *const UP_NAME[UP_COUNT];
extern const uint8_t UP_MAX[UP_COUNT];
extern const char DC_ROOMS[14][15][21];
extern Ent *g_src;                  /* ennemi en train d'attaquer (pour la parade) */
extern uint8_t g_pierce, g_crit;    /* drapeaux transitoires : coup perforant le bouclier / coup critique */

static inline int iabs(int v) { return v < 0 ? -v : v; }
static inline int imin(int a, int b) { return a < b ? a : b; }
static inline int imax(int a, int b) { return a > b ? a : b; }
static inline int clampi(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

/* ---- fournis par la plateforme (NumWorks : main_eadk.c / hote : host/host_main.c) ---- */
uint32_t plat_millis(void);
int plat_load(void *buf, int len);          /* octets lus, ou -1 si indisponible */
int plat_store(const void *buf, int len);   /* 0 si OK, -1 si indisponible */

/* ---- API du jeu ---- */
void game_init(uint32_t seed);
void game_update(uint16_t keys);                           /* un pas de logique a 30 Hz */
void game_render_strip(uint16_t *buf, int y0);             /* dessine les lignes y0..y0+15 (320 px de large) */
int  game_quit_requested(void);

/* ---- internes (dc_core.c, dc_ai.c, dc_render.c) ---- */
uint32_t rnd(void);
int rn(int n);
int tile_at(int tx, int ty);
int is_solid(int t);
enum { MB_GROUND = 1, MB_CEIL = 2, MB_WALL = 4 };
int move_box(int32_t *x, int32_t *y, int32_t *vx, int32_t *vy, int w, int h, int drop);
void level_build(int zone);
void spawn_enemy(int type, int tx, int ty, int zone);
void hurt_player(int dmg, int from_x, int kb);
void hurt_enemy(Ent *e, int dmg, int dir, int kb, int stun);
void add_part(int kind, int x, int y, int vx, int vy, int life, int col);
void add_fx(int sb, int nf, int x, int y, int delay);
void add_fxf(int sb, int nf, int x, int y, int delay, int flip);
void add_num(int x, int y, int val, int col);
void add_proj(int type, int owner, int x, int y, int vx, int vy, int dmg, int life, int dir);
void add_pick(int kind, int x, int y, int val, int vx, int vy);
void add_item_pick(int x, int y, const Item *it, int vy);
void burst(int x, int y, int n, int col);
void ai_update(void);
void part_update(void);
void banner(const char *s, int t);
void msg(const char *s);
int  enemy_hp_scale(int base, int zone);
int  boss_trigger_check(void);
void save_store(void);
int  player_box_overlap(int x, int y, int w, int h);
int  player_parry_window(int from_x);
void kill_all_for_test(void);
void spawn_coins(int x, int y, int total);
void spawn_cells(int x, int y, int n);
void drop_blueprint(int x, int y);
void on_kill_hooks(void);
int  ent_w(const Ent *e);
int  ent_h(const Ent *e);
int  has_mut(int id);
void item_make(Item *it, int id, int bonus, int force_q);
int  item_power(const Item *it);
int  random_weapon(int not_a, int not_b);
const char *roman(int n);
void item_label(const Item *it, char *out);   /* ex. "XII++" */
int  shop_rows(uint8_t *kind, uint8_t *arg);
int  shop_price(int kind, int arg, int *gold);   /* -1 : au maximum / indisponible */
#endif
