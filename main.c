#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <stdlib.h>
#include <time.h>

#define INV_SIZE     10
#define MAX_ITEMS    10
#define MAX_NAME     64

// для карты (лаба 4)
#define MAP_MIN      5
#define MAP_MAX      30
#define OBST_MIN     15   // мин. процент препятствий
#define OBST_MAX     30   // макс. процент препятствий
#define PERCENT      100

#define CELL_WALL    '#'
#define CELL_FLOOR   '.'
#define CELL_TREE    'T'
#define CELL_ROCK    'R'
#define CELL_SWITCH  '*'
#define CELL_EXIT    'E'
#define CELL_HERO    '@'

#define SPLIT_VERTICAL   0
#define SPLIT_HORIZONTAL 1


int current_day  = 1;
int current_hour = 8;
int inventory[INV_SIZE] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

char farmer_name[33] = "Фермер";

char item_names[MAX_ITEMS][MAX_NAME] = {
    "Пусто", "Дерево", "Камень", "Семена",
    "Железо", "Золото", "Зелье", "Факел",
    "Верёвка", "Хлеб"
};

// карта башни (зеркальная комната)
char **map = NULL;
int map_w = 0;
int map_h = 0;
int map_mid = 0;       // номер средней стены (столбец или строка)
int map_split = SPLIT_VERTICAL;
int wall_open = 0;     // 1 если стена посередине исчезла
int hero_x = 0;
int hero_y = 0;
int alloc_count = 0;   // сколько блоков памяти сейчас выделено (для проверки утечек)
int tower_level = 0;


int  read_int(int *out);
void read_line(char *out, int size);
void load_items(void);
void ask_name(void);
void print_menu(void);
void show_clock(void);
void pass_time(void);
void show_inventory(void);
void put_item(void);
void drop_item(void);
void varik(void);
void find_by_name(void);
void write_diary(void);
void decode_records(void);

// лаба 4
void adventure(void);
int  create_map(int w, int h);
void free_map(void);
int  random_range(int a, int b);
int  in_first_half(int x, int y);
void mirror_cell(int x, int y, int *mx, int *my);
void random_first_half_cell(int *x, int *y);
void fill_map(void);
void carve_path(int x1, int y1, int x2, int y2);
void mirror_map(void);
int  generate_tower(void);
void print_map(void);
char read_direction(void);
int  is_passable(int x, int y);
void open_middle_wall(void);
int  move_hero(char dir);


int main(void) {
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    srand((unsigned)time(NULL));

    int choice;

    load_items();
    ask_name();

    do {
        print_menu();
        if (!read_int(&choice)) continue;
        switch (choice) {
            case 0: printf("Пока, %s!\n", farmer_name); break;
            case 1: show_clock(); break;
            case 2: pass_time(); break;
            case 3: show_inventory(); break;
            case 4: put_item(); break;
            case 5: drop_item(); break;
            case 6: varik(); break;
            case 7: find_by_name(); break;
            case 8: write_diary(); break;
            case 9: decode_records(); break;
            case 10: adventure(); break;
            default: printf("Нет такой команды.\n"); break;
        }
    } while (choice != 0);
    return 0;
}


int read_int(int *out) {
    char buf[100];
    if (fgets(buf, sizeof(buf), stdin) == NULL) return 0;
    if (sscanf(buf, "%d", out) != 1) {
        printf("Это не число!\n");
        return 0;
    }
    return 1;
}


void read_line(char *out, int size) {
    size_t len;
    if (fgets(out, size, stdin) == NULL) { out[0] = '\0'; return; }
    len = strlen(out);
    if (len > 0 && out[len - 1] == '\n') out[len - 1] = '\0';
}


void load_items(void) {
    FILE *f;
    char line[100];
    char *p;
    size_t len;
    int i;

    f = fopen("items.txt", "r");
    if (!f) {
        printf("items.txt не найден — используются базовые названия.\n");
        return;
    }
    i = 0;
    while (i < MAX_ITEMS && fgets(line, sizeof(line), f)) {
        p = strchr(line, ' ');
        if (!p) continue;
        p++;
        len = strlen(p);
        if (len > 0 && p[len - 1] == '\n') p[len - 1] = '\0';
        strncpy(item_names[i], p, MAX_NAME - 1);
        item_names[i][MAX_NAME - 1] = '\0';
        i++;
    }
    fclose(f);
    printf("Каталог загружен из items.txt.\n");
}


void ask_name(void) {
    char buf[100];
    printf("Введите имя фермера: ");
    read_line(buf, sizeof(buf));
    if (strlen(buf) > 0) {
        strncpy(farmer_name, buf, 32);
        farmer_name[32] = '\0';
    }
}


void print_menu(void) {
    printf("\n--- МЕНЮ: %s ---\n", farmer_name);
    printf("0) Выход\n");
    printf("1) Часы\n");
    printf("2) Поработать\n");
    printf("3) Инвентарь\n");
    printf("4) Положить предмет\n");
    printf("5) Выбросить предмет\n");
    printf("6) Быстрый доступ\n");
    printf("7) Поиск по названию\n");
    printf("8) Записать в дневник\n");
    printf("9) Оценка у скупщика\n");
    printf("10) Отправиться в приключение (Башня)\n");
    printf("Выбор: ");
}


void show_clock(void) {
    printf("День %d, %02d:00\n", current_day, current_hour);
}

void pass_time(void) {
    int h;
    printf("Сколько часов работать? ");
    if (!read_int(&h)) return;
    if (h < 0 || h > 100) { printf("Недопустимое кол-во часов.\n"); return; }
    current_hour += h;
    while (current_hour >= 24) { current_hour -= 24; current_day++; }
    show_clock();
}

void show_inventory(void) {
    int i;
    for (i = 0; i < INV_SIZE; i++)
        printf("Слот %d: [%d] - %s\n", i, inventory[i], item_names[inventory[i]]);
}

void put_item(void) {
    int idx, id;
    printf("Индекс слота (0-9): ");
    if (!read_int(&idx)) return;
    if (idx < 0 || idx >= INV_SIZE) { printf("Неверный слот!\n"); return; }
    printf("ID предмета (0-9): ");
    if (!read_int(&id)) return;
    if (id < 0 || id > 9) { printf("Неверный ID!\n"); return; }
    inventory[idx] = id;
    printf("В слот %d положен [%d] %s.\n", idx, id, item_names[id]);
}

void drop_item(void) {
    int idx;
    printf("Индекс слота (0-9): ");
    if (!read_int(&idx)) return;
    if (idx < 0 || idx >= INV_SIZE) { printf("Неверный слот!\n"); return; }
    inventory[idx] = 0;
    printf("Слот %d очищен.\n", idx);
}

void varik(void) {
    int id, found, i, t;

    printf("ID предмета (0-9): ");
    if (!read_int(&id)) return;
    if (id <= 0 || id > 9) { printf("Неверный ID!\n"); return; }

    found = -1;
    for (i = 0; i < INV_SIZE; i++)
        if (inventory[i] == id) { found = i; break; }

    if (found == -1) { printf("Такого предмета нет.\n"); return; }
    if (found == 0)  { printf("Уже в слоте 0.\n"); return; }

    t = inventory[0];
    inventory[0] = inventory[found];
    inventory[found] = t;
    printf("Обменян со слотом %d.\n", found);
    show_inventory();
}

void find_by_name(void) {
    char query[MAX_NAME];
    int id, any, i;

    printf("Название предмета: ");
    read_line(query, sizeof(query));

    id = -1;
    for (i = 0; i < MAX_ITEMS; i++)
        if (strcmp(item_names[i], query) == 0) { id = i; break; }

    if (id <= 0) { printf("Такого предмета в каталоге нет.\n"); return; }

    printf("«%s» лежит в слотах: ", query);
    any = 0;
    for (i = 0; i < INV_SIZE; i++)
        if (inventory[i] == id) { printf("%d ", i); any = 1; }
    if (!any) printf("(нет)");
    printf("\n");
}

void write_diary(void) {
    FILE *f;
    int i;

    f = fopen("diary.txt", "a");
    if (!f) { printf("Не удалось открыть diary.txt.\n"); return; }
    fprintf(f, "Фермер: %s | День %d, %02d:00\n", farmer_name, current_day, current_hour);
    for (i = 0; i < INV_SIZE; i++)
        fprintf(f, "  Слот %d: %s\n", i, item_names[inventory[i]]);
    fprintf(f, "\n");
    fclose(f);
    printf("Записано в diary.txt.\n");
}


void decode_records(void) {
    char pname[64][64];
    int  pprice[64];
    int  count, i, j, price, total, best, best_slot;
    char line[200];
    char *sp;
    char *nm;
    size_t len;
    FILE *in;
    FILE *out;

    count = 0;
    in = fopen("input.txt", "r");
    if (!in) { printf("input.txt не найден.\n"); return; }

    while (fgets(line, sizeof(line), in) && count < 64) {
        len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') line[len - 1] = '\0';
        if (len == 0) continue;

        sp = strrchr(line, ' ');
        if (!sp) continue;
        *sp = '\0';
        strncpy(pname[count], line, 63);
        pname[count][63] = '\0';
        pprice[count] = atoi(sp + 1);
        count++;
    }
    fclose(in);

    if (count == 0) { printf("Прайс пуст.\n"); 
        return; }

    out = fopen("output.txt", "w");
    if (!out) { printf("Не создать output.txt.\n"); 
        return; }

    fprintf(out, "--- Прайс: %s ---\n", farmer_name);
    fprintf(out, "День %d, %02d:00\n\n", current_day, current_hour);
    printf("--- Прайс: %s ---\n", farmer_name);
    printf("День %d, %02d:00\n\n", current_day, current_hour);

    total = 0;
    best = -1;
    best_slot = -1;

    for (i = 0; i < INV_SIZE; i++) {
        if (inventory[i] == 0) continue;
        nm = item_names[inventory[i]];

        price = 0;
        for (j = 0; j < count; j++)
            if (strcmp(pname[j], nm) == 0) { price = pprice[j]; break; }

        fprintf(out, "Слот %d: %-16s — %d\n", i, nm, price);
        printf ("Слот %d: %-16s — %d\n", i, nm, price);

        total += price;
        if (price > best) { best = price; best_slot = i; }
    }

    if (best_slot == -1) {
        fprintf(out, "\nСамый дорогой: (нет)\n");
        printf ("\nСамый дорогой: (нет)\n");
    } else {
        fprintf(out, "\nСамый дорогой: %s (%d)\n", item_names[inventory[best_slot]], best);
        printf ("\nСамый дорогой: %s (%d)\n", item_names[inventory[best_slot]], best);
    }
    fprintf(out, "Итог: %d\n", total);
    printf ("Итог: %d\n", total);

    fclose(out);
    printf("\nСохранено в output.txt.\n");
}



// ===================== ЛАБА 4: ЗЕРКАЛЬНАЯ КОМНАТА =====================

// главная функция приключения
void adventure(void) {
    char dir;
    int result;

    tower_level = 1;
    if (!generate_tower()) return;

    printf("\n%s входит в Зеркальную башню!\n", farmer_name);
    printf("Найдите переключатель (%c), чтобы убрать стену посередине.\n", CELL_SWITCH);

    while (1) {
        print_map();
        dir = read_direction();
        if (dir == 'Q') {
            printf("%s сбежал домой.\n", farmer_name);
            break;
        }
        result = move_hero(dir);
        if (result == 1) {
            // нашли выход - старую карту чистим и делаем новую
            printf("Вы нашли выход! Переход на следующий этаж башни...\n");
            free_map();
            tower_level++;
            if (!generate_tower()) return;
        }
    }

    free_map();
}


// выделяем память под карту w на h
int create_map(int w, int h) {
    int i, j;

    map = (char**)malloc(h * sizeof(char*));
    if (map == NULL) {
        printf("Ошибка: не хватает памяти!\n");
        return 0;
    }
    alloc_count++;

    for (i = 0; i < h; i++) {
        map[i] = (char*)malloc(w * sizeof(char));
        if (map[i] == NULL) {
            printf("Ошибка: не хватает памяти!\n");
            // чистим то что уже успели выделить
            for (j = 0; j < i; j++) {
                free(map[j]);
                alloc_count--;
            }
            free(map);
            alloc_count--;
            map = NULL;
            return 0;
        }
        alloc_count++;
    }

    map_w = w;
    map_h = h;
    return 1;
}


// освобождаем память в обратном порядке: сначала строки, потом массив указателей
void free_map(void) {
    int i;

    if (map == NULL) return;

    for (i = 0; i < map_h; i++) {
        free(map[i]);
        alloc_count--;
    }
    free(map);
    alloc_count--;
    map = NULL;

    printf("[Память карты %dx%d освобождена, неосвобождённых блоков: %d]\n",
           map_w, map_h, alloc_count);
    map_w = 0;
    map_h = 0;
}


// случайное число от a до b включительно
int random_range(int a, int b) {
    return a + rand() % (b - a + 1);
}


// клетка в первой половине (там где герой)?
int in_first_half(int x, int y) {
    if (x <= 0 || y <= 0 || x >= map_w - 1 || y >= map_h - 1) return 0;
    if (map_split == SPLIT_VERTICAL) return x < map_mid;
    return y < map_mid;
}


// координаты зеркальной клетки во второй половине
void mirror_cell(int x, int y, int *mx, int *my) {
    if (map_split == SPLIT_VERTICAL) {
        *mx = map_w - 1 - x;
        *my = y;
    } else {
        *mx = x;
        *my = map_h - 1 - y;
    }
}


// случайная внутренняя клетка первой половины
void random_first_half_cell(int *x, int *y) {
    if (map_split == SPLIT_VERTICAL) {
        *x = random_range(1, map_mid - 1);
        *y = random_range(1, map_h - 2);
    } else {
        *x = random_range(1, map_w - 2);
        *y = random_range(1, map_mid - 1);
    }
}


// стены по краям, стена посередине, препятствия в первой половине
void fill_map(void) {
    int x, y, chance;

    chance = random_range(OBST_MIN, OBST_MAX);

    for (y = 0; y < map_h; y++) {
        for (x = 0; x < map_w; x++) {
            if (x == 0 || y == 0 || x == map_w - 1 || y == map_h - 1) {
                map[y][x] = CELL_WALL;
            } else if ((map_split == SPLIT_VERTICAL && x == map_mid) ||
                       (map_split == SPLIT_HORIZONTAL && y == map_mid)) {
                map[y][x] = CELL_WALL;
            } else if (in_first_half(x, y) && rand() % PERCENT < chance) {
                if (rand() % 2 == 0) map[y][x] = CELL_TREE;
                else map[y][x] = CELL_ROCK;
            } else {
                map[y][x] = CELL_FLOOR;
            }
        }
    }
}


// прокладываем дорожку (сначала по x, потом по y), чтобы до цели точно можно было дойти
void carve_path(int x1, int y1, int x2, int y2) {
    int x = x1;
    int y = y1;

    map[y][x] = CELL_FLOOR;
    while (x != x2) {
        if (x < x2) x++;
        else x--;
        map[y][x] = CELL_FLOOR;
    }
    while (y != y2) {
        if (y < y2) y++;
        else y--;
        map[y][x] = CELL_FLOOR;
    }
}


// копируем первую половину во вторую зеркально
void mirror_map(void) {
    int x, y, mx, my;

    for (y = 1; y < map_h - 1; y++) {
        for (x = 1; x < map_w - 1; x++) {
            if (in_first_half(x, y)) {
                mirror_cell(x, y, &mx, &my);
                map[my][mx] = map[y][x];
            }
        }
    }
}


// генерация всей башни
int generate_tower(void) {
    int w, h;
    int sx, sy, ex, ey;

    map_split = rand() % 2;
    w = random_range(MAP_MIN, MAP_MAX);
    h = random_range(MAP_MIN, MAP_MAX);

    // чтобы стена была строго посередине, размер должен быть нечётный
    if (map_split == SPLIT_VERTICAL && w % 2 == 0) w--;
    if (map_split == SPLIT_HORIZONTAL && h % 2 == 0) h--;

    if (!create_map(w, h)) return 0;

    if (map_split == SPLIT_VERTICAL) map_mid = w / 2;
    else map_mid = h / 2;

    wall_open = 0;
    fill_map();

    // герой
    random_first_half_cell(&hero_x, &hero_y);

    // переключатель (не там где герой)
    do {
        random_first_half_cell(&sx, &sy);
    } while (sx == hero_x && sy == hero_y);

    // дорожка от героя к переключателю и от переключателя к средней стене
    carve_path(hero_x, hero_y, sx, sy);
    if (map_split == SPLIT_VERTICAL) carve_path(sx, sy, map_mid - 1, sy);
    else carve_path(sx, sy, sx, map_mid - 1);

    mirror_map();

    map[sy][sx] = CELL_SWITCH;
    mirror_cell(hero_x, hero_y, &ex, &ey);
    map[ey][ex] = CELL_EXIT;

    printf("\n[Выделена память под карту %dx%d, блоков: %d]\n", map_w, map_h, alloc_count);
    return 1;
}


void print_map(void) {
    int x, y;

    printf("\n=== Зеркальная башня, этаж %d (%dx%d) ===\n", tower_level, map_w, map_h);
    for (y = 0; y < map_h; y++) {
        for (x = 0; x < map_w; x++) {
            if (x == hero_x && y == hero_y) printf("%c", CELL_HERO);
            else printf("%c", map[y][x]);
        }
        printf("\n");
    }
    printf("%c-герой %c-стена %c-дерево %c-камень %c-переключатель %c-выход\n",
           CELL_HERO, CELL_WALL, CELL_TREE, CELL_ROCK, CELL_SWITCH, CELL_EXIT);
    if (wall_open) printf("Стена открыта! Идите к выходу.\n");
}


// читаем W A S D или Q
char read_direction(void) {
    char buf[100];
    char c;

    while (1) {
        printf("Куда идти? (W-вверх, A-влево, S-вниз, D-вправо, Q-домой): ");
        read_line(buf, sizeof(buf));
        if (strlen(buf) != 1) {
            printf("Введите одну латинскую букву!\n");
            continue;
        }
        c = buf[0];
        if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
        if (c == 'W' || c == 'A' || c == 'S' || c == 'D' || c == 'Q') return c;
        printf("Нет такого направления!\n");
    }
}


// можно ли встать на клетку
int is_passable(int x, int y) {
    char c;

    if (x < 0 || y < 0 || x >= map_w || y >= map_h) return 0;
    c = map[y][x];
    if (c == CELL_FLOOR || c == CELL_SWITCH || c == CELL_EXIT) return 1;
    return 0;
}


// убираем стену посередине (кроме внешних стен)
void open_middle_wall(void) {
    int i;

    if (map_split == SPLIT_VERTICAL) {
        for (i = 1; i < map_h - 1; i++) map[i][map_mid] = CELL_FLOOR;
    } else {
        for (i = 1; i < map_w - 1; i++) map[map_mid][i] = CELL_FLOOR;
    }
    wall_open = 1;
}


// двигаем героя, возвращает 1 если дошли до выхода
int move_hero(char dir) {
    int nx = hero_x;
    int ny = hero_y;

    if (dir == 'W') ny--;
    else if (dir == 'S') ny++;
    else if (dir == 'A') nx--;
    else if (dir == 'D') nx++;

    if (!is_passable(nx, ny)) {
        printf("Туда нельзя, там препятствие!\n");
        return 0;
    }
    // пока стена есть, ходить можно только по своей половине
    if (!wall_open && !in_first_half(nx, ny)) {
        printf("Нельзя уйти со своей половины!\n");
        return 0;
    }

    hero_x = nx;
    hero_y = ny;

    if (map[ny][nx] == CELL_SWITCH) {
        printf("Щёлк! Вы нажали переключатель - стена посередине исчезла!\n");
        map[ny][nx] = CELL_FLOOR;
        open_middle_wall();
    } else if (map[ny][nx] == CELL_EXIT) {
        return 1;
    }
    return 0;
}