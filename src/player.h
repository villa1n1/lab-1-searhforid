#ifndef PLAYER_H
#define PLAYER_H

#include "utils.h"

#define NAME_LEN           33
#define DEFAULT_NAME       "Фермер"
#define START_MAX_HP       100
#define START_MAX_STAMINA  50
#define START_GOLD         0

struct Player {
    char name[NAME_LEN];
    int hp;
    int max_hp;
    int base_max_hp;
    int stamina;
    int max_stamina;
    int base_max_stamina;
    int gold;
    int x, y;
    int inventory[INV_SIZE];
};

struct Player *player_create(void);
void player_free(struct Player **player);
void player_print_card(const struct Player *p, const struct GameTime *t, char names[][MAX_NAME]);
void player_print_hud(const struct Player *p, int floor, const struct GameTime *t);
void player_rest(struct Player *p, struct GameTime *t);
void player_spend_stamina(struct Player *p, int amount);

void inventory_show(const struct Player *p, char names[][MAX_NAME]);
void inventory_put(struct Player *p, char names[][MAX_NAME]);
void inventory_drop(struct Player *p);
void inventory_quick(struct Player *p, char names[][MAX_NAME]);
void inventory_find(const struct Player *p, char names[][MAX_NAME]);

void diary_write(const struct Player *p, const struct GameTime *t, char names[][MAX_NAME]);
void price_report(const struct Player *p, const struct GameTime *t, char names[][MAX_NAME]);

#endif
