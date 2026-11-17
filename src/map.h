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

// карта этажа: двумерный динамический массив и номер столбца с зеркалом
struct Map {
    char **cells;
    int width;
    int height;
    int floor;
    int mirror_x;
    int mirror_open;
};

// Создаём новую карту (структуру и массив) для этажа floor
struct Map *map_create(int floor, struct Player *player);
// Освобождаем память карты: сначала строки, потом массив указателей, потом структуру
void map_free(struct Map **map);
// Рисуем карту на экране
void map_print(const struct Map *map, const struct Player *player);
// Читаем клавишу движения W/A/S/D или Q
enum Direction read_direction(void);
// Пробуем сделать шаг: проверяем зеркало и препятствия, срабатывают рычаг и выход
enum MoveResult map_move(struct Map *map, struct Player *player, enum Direction dir);

#endif
