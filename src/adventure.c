#include <stdio.h>
#include "adventure.h"


static int can_start(const struct Player *player, const struct GameTime *time) {
    if (player->stamina <= 0) {
        printf("У героя нет сил для похода. Сначала нужно отдохнуть.\n");
        return 0;
    }
    if (time_is_night(time)) {
        printf("Уже 02:00 ночи - в поход идти поздно. Ложитесь спать.\n");
        return 0;
    }
    return 1;
}


static void faint(struct Player *player) {
    int fee = player->gold / FAINT_GOLD_DIVIDER;

    printf("\n%s падает без сил! Спасательная служба доставила героя домой.\n", player->name);
    printf("За услуги спасателей взято золота: %d.\n", fee);
    player->gold -= fee;
}


static struct Map *next_floor(struct Map *map, struct Player *player, struct GameTime *time) {
    int reward = GOLD_PER_FLOOR * map->floor;
    int next_number = map->floor + 1;

    player->gold += reward;
    player_spend_stamina(player, FLOOR_COST);
    time_add_hours(time, FLOOR_HOURS);
    printf("Вы нашли выход! Этаж %d пройден, награда: %d золота.\n", map->floor, reward);

    map_free(&map);
    if (player->stamina <= 0 || time_is_night(time)) return NULL;
    return map_create(next_number, player);
}


static int hero_is_exhausted(const struct Player *player, const struct GameTime *time) {
    return player->stamina <= 0 || time_is_night(time);
}


void adventure_run(struct Player *player, struct GameTime *time) {
    struct Map *map;
    enum Direction dir;
    enum MoveResult result;
    int steps = 0;
    int fainted = 0;

    if (!can_start(player, time)) return;

    map = map_create(1, player);
    if (map == NULL) return;

    printf("\n%s входит в Зеркальную башню!\n", player->name);

    while (1) {
        player_print_hud(player, map->floor, time);
        map_print(map, player);

        dir = read_direction();
        if (dir == DIR_QUIT) {
            printf("%s вернулся домой. Золото осталось при нём: %d. Шагов сделано: %d\n",
                   player->name, player->gold, steps);
            break;
        }

        result = map_move(map, player, dir);
        if (result == MOVE_BLOCKED) continue;

        steps++;
        player_spend_stamina(player, STEP_COST);

        if (result == MOVE_EXIT) {
            map = next_floor(map, player, time);
            if (map == NULL) {
                fainted = hero_is_exhausted(player, time);
                break;
            }
        } else if (hero_is_exhausted(player, time)) {
            fainted = 1;
            break;
        }
    }

    if (fainted) faint(player);
    map_free(&map);
}
