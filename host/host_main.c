/* host_main.c - banc d'essai sur PC : plateforme factice + captures d'ecran (PPM) de scenes representatives. */
#include "dc.h"
#include <stdio.h>
#include <stdlib.h>

static char save_path[256] = "/tmp/dc_test.sav";
uint32_t plat_millis(void) { static uint32_t t; return t += 33; }
int plat_load(void *buf, int len) {
    FILE *f = fopen(save_path, "rb"); int n;
    if (!f) return 0;
    n = (int)fread(buf, 1, (size_t)len, f); fclose(f); return n;
}
int plat_store(const void *buf, int len) {
    FILE *f = fopen(save_path, "wb");
    if (!f) return -1;
    fwrite(buf, 1, (size_t)len, f); fclose(f); return 0;
}

static uint16_t FB[SCR_W * SCR_H];
static const char *outdir = ".";
static void render(void) { for (int y = 0; y < SCR_H; y += STRIP_H) game_render_strip(FB + y * SCR_W, y); }
static void shot(const char *name) {
    char path[300]; FILE *f;
    render();
    snprintf(path, sizeof path, "%s/%s.ppm", outdir, name);
    f = fopen(path, "wb");
    if (!f) { perror(path); return; }
    fprintf(f, "P6\n%d %d\n255\n", SCR_W, SCR_H);
    for (int i = 0; i < SCR_W * SCR_H; i++) {
        uint16_t c = FB[i];
        fputc(((c >> 11) & 31) * 255 / 31, f); fputc(((c >> 5) & 63) * 255 / 63, f); fputc((c & 31) * 255 / 31, f);
    }
    fclose(f);
}
static void run(uint16_t keys, int frames) { for (int i = 0; i < frames; i++) game_update(keys); }
static void new_run(unsigned seed) { remove(save_path); game_init(seed); game_update(K_OK); game_update(0); }
static void tp(int px, int py) { G.p.x = F(px); G.p.y = F(py); G.p.vx = G.p.vy = 0; G.cam_x = F(px - 160 < 0 ? 0 : px - 160); }

int main(int argc, char **argv) {
    if (argc > 1) outdir = argv[1];
    new_run(7);
    game_init(7); run(0, 40); shot("01_titre");
    new_run(7); run(0, 20); shot("02_depart_zone1");
    run(K_R, 150); shot("03_course_zone1");
    /* combat : un de chaque ennemi autour du joueur */
    new_run(11); tp(100 * 16 + 2 * 16, 13 * 16);
    G.lvw = 100;
    spawn_enemy(ET_JAIL, 100 + 4, 12, 0); spawn_enemy(ET_SHLD, 100 + 7, 12, 0); spawn_enemy(ET_GREN, 100 + 11, 12, 1);
    run(0, 3); G.p.dir = 1; run(K_OK, 3); run(0, 6); shot("04_combat");
    for (int z = 0; z < 5; z++) {
        char nm[32]; new_run(20 + z); level_build(z); G.scene = SC_PLAY;
        tp(2 * 320 + 100, 13 * 16); run(0, 30);
        snprintf(nm, sizeof nm, "05_zone%d", z + 1); shot(nm);
    }
    new_run(31); level_build(2); tp(G.arena_x0 + 100, 13 * 16); run(0, 90); shot("06_boss_gobeur");
    new_run(32); level_build(4); tp(G.arena_x0 + 100, 13 * 16); run(0, 90); shot("07_boss_roi");
    new_run(40); tp((G.nrooms - 1) * 320 + 16 * 16 + 8, 13 * 16); run(0, 4); shot("08_passage");
    G.sv.cells = 40; G.run_gold = 90; G.sv.found |= 0x3FF; G.scene = SC_SHOP; G.in_run = 1; G.menu_sel = 6; shot("09_boutique");
    G.scene = SC_MUT; G.menu_sel = 0; shot("10_mutations");
    G.scene = SC_PLAY; G.p.hp = 40; G.mut[0] = MU_REVENGE; G.mut_n = 1; G.scene = SC_PAUSE; G.menu_sel = 0; shot("11_pause");
    G.scene = SC_HELP; G.menu_sub = 0; shot("12_aide");
    G.scene = SC_OPTIONS; G.menu_sel = 0; shot("13_options");
    G.scene = SC_PLAY; G.p.st = PS_DEAD; G.p.hp = 0; G.p.dead_t = 71; run(0, 2); shot("14_mort");
    printf("captures ecrites dans %s\n", outdir);
    return 0;
}
