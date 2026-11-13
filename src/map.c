#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "map.h"


static int allocate_cells(struct Map *map, int w, int h) {
    int i, j;

    map->cells = (char**)malloc(h * sizeof(char*));
    if (map->cells == NULL) return 0;

    for (i = 0; i < h; i++) {
        map->cells[i] = (char*)malloc(w * sizeof(char));
        if (map->cells[i] == NULL) {
            for (j = 0; j < i; j++) free(map->cells[j]);
            free(map->cells);
            map->cells = NULL;
            return 0;
        }
    }
    return 1;
}


static void fill_map(struct Map *map) {
    int x, y, chance;

    chance = random_range(OBST_MIN, OBST_MAX);

    for (y = 0; y < map->height; y++) {
        for (x = 0; x < map->width; x++) {
            if (x == 0 || y == 0 || x == map->width - 1 || y == map->height - 1) {
                map->cells[y][x] = CELL_WALL;
            } else if (x == map->mirror_x) {
                map->cells[y][x] = CELL_MIRROR;
            } else if (x < map->mirror_x && rand() % PERCENT < chance) {
                if (rand() % 2 == 0) map->cells[y][x] = CELL_TREE;
                else map->cells[y][x] = CELL_ROCK;
            } else {
                map->cells[y][x] = CELL_FLOOR;
            }
        }
    }
}


static void carve_path(struct Map *map, int x1, int y1, int x2, int y2) {
    int x = x1;
    int y = y1;

    map->cells[y][x] = CELL_FLOOR;
    while (x != x2) {
        if (x < x2) x++;
        else x--;
        map->cells[y][x] = CELL_FLOOR;
    }
    while (y != y2) {
        if (y < y2) y++;
        else y--;
        map->cells[y][x] = CELL_FLOOR;
    }
}


static void mirror_left_half(struct Map *map) {
    int x, y;

    for (y = 1; y < map->height - 1; y++) {
        for (x = 1; x < map->mirror_x; x++) {
            map->cells[y][map->width - 1 - x] = map->cells[y][x];
        }
    }
}


static void place_objects(struct Map *map, struct Player *player) {
    int lever_x, lever_y;

    player->x = random_range(1, map->mirror_x - 1);
    player->y = random_range(1, map->height - 2);

    do {
        lever_x = random_range(1, map->mirror_x - 1);
        lever_y = random_range(1, map->height - 2);
    } while (lever_x == player->x && lever_y == player->y);

    carve_path(map, player->x, player->y, lever_x, lever_y);
    carve_path(map, lever_x, lever_y, map->mirror_x - 1, lever_y);

    mirror_left_half(map);

    map->cells[lever_y][lever_x] = CELL_LEVER;
    map->cells[player->y][map->width - 1 - player->x] = CELL_EXIT;
}


struct Map *map_create(int floor, struct Player *player) {
    struct Map *map;
    int w, h;

    map = (struct Map*)malloc(sizeof(struct Map));
    if (map == NULL) {
        printf("Ошибка: не хватает памяти!\n");
        return NULL;
    }

    w = random_range(MAP_MIN, MAP_MAX);
    h = random_range(MAP_MIN, MAP_MAX);
    if (w % 2 == 0) w--;

    if (!allocate_cells(map, w, h)) {
        printf("Ошибка: не хватает памяти!\n");
        free(map);
        return NULL;
    }

    map->width = w;
    map->height = h;
    map->floor = floor;
    map->mirror_x = w / 2;
    map->mirror_open = 0;

    fill_map(map);
    place_objects(map, player);

    printf("\n[Карта %dx%d создана по адресу %p]\n", w, h, (void*)map);
    return map;
}


void map_free(struct Map **map) {
    int i;

    if (map == NULL || *map == NULL) return;

    for (i = 0; i < (*map)->height; i++) free((*map)->cells[i]);
    free((*map)->cells);

    printf("Память структуры по адресу %p освобождена\n", (void*)*map);
    free(*map);
    *map = NULL;
}


static char cell_to_draw(const struct Map *map, const struct Player *player, int x, int y) {
    int mirror_pos;

    if (x == player->x && y == player->y) return CELL_HERO;
    if (!map->mirror_open && x > map->mirror_x) {
        mirror_pos = map->width - 1 - x;
        if (mirror_pos == player->x && y == player->y) return CELL_HERO;
        if (map->cells[y][mirror_pos] == CELL_LEVER) return CELL_LEVER;
    }
    return map->cells[y][x];
}


void map_print(const struct Map *map, const struct Player *player) {
    int x, y;

    printf("\n=== Зеркальная башня, этаж %d (%dx%d) ===\n", map->floor, map->width, map->height);
    for (y = 0; y < map->height; y++) {
        for (x = 0; x < map->width; x++) printf("%c", cell_to_draw(map, player, x, y));
        printf("\n");
    }
    printf("%c-герой %c-стена %c-зеркало %c-дерево %c-камень %c-рычаг %c-выход\n",
           CELL_HERO, CELL_WALL, CELL_MIRROR, CELL_TREE, CELL_ROCK, CELL_LEVER, CELL_EXIT);
    if (!map->mirror_open) printf("За зеркалом - отражение героя и рычага.\n");
    else printf("Зеркало исчезло! Идите к выходу.\n");
}


enum Direction read_direction(void) {
    char buf[INPUT_SIZE];
    char c;

    while (1) {
        printf("Куда идти? (W-вверх, A-влево, S-вниз, D-вправо, Q-домой): ");
        if (!read_line(buf, sizeof(buf))) return DIR_QUIT;
        if (strlen(buf) != 1) {
            printf("Введите одну латинскую букву!\n");
            continue;
        }
        c = buf[0];
        if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
        if (c == 'W') return DIR_UP;
        if (c == 'S') return DIR_DOWN;
        if (c == 'A') return DIR_LEFT;
        if (c == 'D') return DIR_RIGHT;
        if (c == 'Q') return DIR_QUIT;
        printf("Нет такого направления!\n");
    }
}


static int is_passable(const struct Map *map, int x, int y) {
    char c;

    if (x < 0 || y < 0 || x >= map->width || y >= map->height) return 0;
    c = map->cells[y][x];
    return c == CELL_FLOOR || c == CELL_LEVER || c == CELL_EXIT;
}


static void open_mirror(struct Map *map) {
    int y;

    for (y = 1; y < map->height - 1; y++) map->cells[y][map->mirror_x] = CELL_FLOOR;
    map->mirror_open = 1;
}


enum MoveResult map_move(struct Map *map, struct Player *player, enum Direction dir) {
    int nx = player->x;
    int ny = player->y;

    if (dir == DIR_UP) ny--;
    else if (dir == DIR_DOWN) ny++;
    else if (dir == DIR_LEFT) nx--;
    else if (dir == DIR_RIGHT) nx++;

    if (map->cells[ny][nx] == CELL_MIRROR) {
        printf("Это зеркало, сквозь него не пройти. Найдите рычаг!\n");
        return MOVE_BLOCKED;
    }
    if (!is_passable(map, nx, ny)) {
        printf("Туда нельзя, там препятствие!\n");
        return MOVE_BLOCKED;
    }

    player->x = nx;
    player->y = ny;

    if (map->cells[ny][nx] == CELL_LEVER) {
        printf("Щёлк! Вы дёрнули рычаг - зеркало исчезло, проход во вторую половину открыт!\n");
        map->cells[ny][nx] = CELL_FLOOR;
        open_mirror(map);
    } else if (map->cells[ny][nx] == CELL_EXIT) {
        return MOVE_EXIT;
    }
    return MOVE_OK;
}
