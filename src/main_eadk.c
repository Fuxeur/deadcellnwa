/* main_eadk.c - couche NumWorks (EADK) : touches, ecran par bandes, chronometrage 30 Hz.
   Compilee uniquement pour la calculatrice (build .nwa). Sur PC, host/host_main.c et host/test_dc.c jouent ce role. */
#include "dc.h"
#include <eadk.h>

const char eadk_app_name[] __attribute__((section(".rodata.eadk_app_name"))) = "Dead Cells";
const uint32_t eadk_api_level __attribute__((section(".rodata.eadk_api_level"))) = 0;

uint32_t plat_millis(void) { return (uint32_t)eadk_timing_millis(); }

/* Sauvegarde : l'API officielle EADK n'offre pas d'ecriture persistante. Par defaut le jeu fonctionne sans (la progression
   reste en memoire tant que l'application tourne). Pour activer la sauvegarde via la bibliotheque non officielle
   "numworks-extapp-storage", compiler avec -DDC_SAVE_EXTAPP et ajouter storage.c/storage.h. */
#ifdef DC_SAVE_EXTAPP
#include "storage.h"
int plat_load(void *buf, int len) {
    size_t n = 0;
    const char *d = extapp_fileRead("deadcells.sav", &n);
    if (!d || (int)n != len) return 0;
    memcpy(buf, d, (size_t)len);
    return len;
}
int plat_store(const void *buf, int len) { return extapp_fileWrite("deadcells.sav", (const char *)buf, (size_t)len) ? 0 : -1; }
#else
int plat_load(void *buf, int len) { (void)buf; (void)len; return -1; }
int plat_store(const void *buf, int len) { (void)buf; (void)len; return -1; }
#endif

static uint16_t poll_keys(int *quit) {
    eadk_keyboard_state_t st = eadk_keyboard_scan();
    uint16_t k = 0;
    if (eadk_keyboard_key_down(st, eadk_key_left)) k |= K_L;
    if (eadk_keyboard_key_down(st, eadk_key_right)) k |= K_R;
    if (eadk_keyboard_key_down(st, eadk_key_up)) k |= K_U;
    if (eadk_keyboard_key_down(st, eadk_key_down)) k |= K_D;
    if (eadk_keyboard_key_down(st, eadk_key_ok)) k |= K_OK;
    if (eadk_keyboard_key_down(st, eadk_key_back)) k |= K_BACK;          /* esquive en jeu, retour dans les menus */
    if (eadk_keyboard_key_down(st, eadk_key_backspace)) k |= K_SWAP;      /* changer d'arme / 2e attaque de la faux-serres */
    if (eadk_keyboard_key_down(st, eadk_key_exe)) k |= K_FLASK;           /* fiole de soin */
    if (eadk_keyboard_key_down(st, eadk_key_toolbox)) k |= K_PAUSE;       /* pause */
    if (eadk_keyboard_key_down(st, eadk_key_home) || eadk_keyboard_key_down(st, eadk_key_on_off)) *quit = 1;
    return k;
}

int main(int argc, char *argv[]) {
    uint16_t strip[SCR_W * STRIP_H];              /* 10 Ko sur la pile : pas de tampon plein ecran (153 Ko) */
    uint64_t next;
    int quit = 0;
    (void)argc; (void)argv;
    game_init((uint32_t)eadk_timing_millis() ^ 0x5EEDu);
    next = eadk_timing_millis();
    while (!quit && !game_quit_requested()) {
        uint64_t now = eadk_timing_millis();
        int steps = 0;
        while (now >= next && steps < 3) {         /* logique a 30 Hz fixe, rattrapage limite a 3 pas */
            uint16_t k = poll_keys(&quit);
            if (quit) break;
            game_update(k);
            next += 33; steps++; now = eadk_timing_millis();
        }
        if (quit) break;
        if (now >= next + 200) next = now;         /* trop de retard : on abandonne le rattrapage */
        eadk_display_wait_for_vblank();
        for (int y = 0; y < SCR_H; y += STRIP_H) {
            eadk_rect_t r;
            game_render_strip(strip, y);
            r.x = 0; r.y = (uint16_t)y; r.width = SCR_W; r.height = STRIP_H;
            eadk_display_push_rect(r, strip);
        }
        if (steps == 0) eadk_timing_msleep(2);
    }
    {   /* attendre que les touches soient relachees avant de rendre la main au systeme */
        uint64_t t0 = eadk_timing_millis();
        while (eadk_keyboard_scan() != 0 && eadk_timing_millis() - t0 < 600) eadk_timing_msleep(10);
    }
    return 0;
}
