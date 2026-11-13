#ifndef MAP_H
#define MAP_H

#include "player.h"

#define MAP_MIN      5
#define MAP_MAX      30
#define OBST_MIN     15
#define OBST_MAX     30
#define PERCENT      100

#define CELL_WALL    '#'
#define CELL_FLOOR   '.'
#define CELL_TREE    'T'
#define CELL_ROCK    'R'
#define CELL_LEVER   'L'
#define CELL_MIRROR  'M'
#define CELL_EXIT    'E'
#define CELL_HERO    '@'

enum Direction {
    DIR_NONE = 0,
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT,
    DIR_QUIT
};

enum MoveResult {
    MOVE_BLOCKED = 0,
    MOVE_OK,
    MOVE_EXIT
};

struct Map {
    char **cells;
    int width;
    int height;
    int floor;
    int mirror_x;
    int mirror_open;
};

struct Map *map_create(int floor, struct Player *player);
void map_free(struct Map **map);
void map_print(const struct Map *map, const struct Player *player);
enum Direction read_direction(void);
enum MoveResult map_move(struct Map *map, struct Player *player, enum Direction dir);

#endif
