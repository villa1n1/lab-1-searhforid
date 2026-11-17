// main.c - точка входа и главное меню
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "utils.h"
#include "player.h"
#include "adventure.h"

#define CODEPAGE_UTF8  65001

enum MenuItem {
    MENU_EXIT = 0,
    MENU_CREATE,
    MENU_CARD,
    MENU_TIME,
    MENU_INVENTORY,
    MENU_PUT,
    MENU_DROP,
    MENU_QUICK,
    MENU_FIND,
    MENU_DIARY,
    MENU_ADVENTURE,
    MENU_PRICES,
    MENU_EQUIPMENT
};

enum TimeMenuItem {
    TIME_BACK = 0,
    TIME_CLOCK,
    TIME_WAIT,
    TIME_SLEEP
};

// всё состояние игры хранится здесь, глобальных переменных нет
struct Game {
    struct GameTime time;
    struct Player *player;
    char item_names[MAX_ITEMS][MAX_NAME];
    struct Equipment catalog[EQUIP_CATALOG_SIZE];
};


// Включаем русский язык в консоли (только для Windows)
static void setup_console(void) {
#ifdef _WIN32
    SetConsoleOutputCP(CODEPAGE_UTF8);
    SetConsoleCP(CODEPAGE_UTF8);
#endif
}


// Начальные значения: время, пустой указатель на героя, каталоги предметов
static void game_init(struct Game *game) {
    time_init(&game->time);
    game->player = NULL;
    equipment_fill_catalog(game->catalog);
    load_item_names(game->item_names);
}


// Перед выходом освобождаем память героя
static void game_cleanup(struct Game *game) {
    player_free(&game->player);
}


// Печатаем главное меню
static void print_menu(const struct Game *game) {
    if (game->player != NULL) printf("\n=== МЕНЮ: %s ===\n", game->player->name);
    else printf("\n=== МЕНЮ: персонаж не создан ===\n");
    printf("0) Выход\n");
    printf("1) Создать / пересоздать персонажа\n");
    printf("2) Карточка персонажа\n");
    printf("3) Время (часы, ждать, спать)\n");
    printf("4) Инвентарь\n");
    printf("5) Положить предмет\n");
    printf("6) Выбросить предмет\n");
    printf("7) Быстрый доступ\n");
    printf("8) Поиск по названию\n");
    printf("9) Записать в дневник\n");
    printf("10) Отправиться в приключение (Башня)\n");
    printf("11) Оценка у скупщика\n");
    printf("12) Управление снаряжением\n");
    printf("Выбор: ");
}


// Проверка, что персонаж создан (иначе большинство пунктов недоступны)
static int require_player(const struct Game *game) {
    if (game->player == NULL) {
        printf("Сначала создайте персонажа (пункт 1).\n");
        return 0;
    }
    return 1;
}


// Пункт 1: создать героя. Если он уже есть - спрашиваем и освобождаем старого
static void create_character(struct Game *game) {
    struct Player *fresh;

    if (game->player != NULL) {
        printf("Персонаж %s уже существует.\n", game->player->name);
        if (!ask_yes_no("Точно удалить его и создать нового?")) {
            printf("Оставляем старого персонажа.\n");
            return;
        }
        player_free(&game->player);
    }

    fresh = player_create();
    if (fresh == NULL) return;
    game->player = fresh;
}


// Пункт 2: карточка персонажа
static void show_card(struct Game *game) {
    if (!require_player(game)) return;
    player_print_card(game->player, &game->time, game->item_names);
}


// Лечь спать: утро следующего дня и полное восстановление сил
static void sleep_at_home(struct Game *game) {
    if (!require_player(game)) return;
    player_rest(game->player, &game->time);
}


// Печатаем подменю времени
static void print_time_menu(void) {
    printf("\n--- Время ---\n");
    printf("1) Посмотреть на часы\n");
    printf("2) Ждать\n");
    printf("3) Лечь спать\n");
    printf("0) Назад\n");
    printf("Выбор: ");
}


// Подменю времени: часы, ждать, спать
static void time_menu(struct Game *game) {
    int choice, status;

    while (1) {
        print_time_menu();
        status = read_int(&choice);
        if (status == INPUT_EOF) return;
        if (status != INPUT_OK) continue;

        switch (choice) {
            case TIME_BACK: return;
            case TIME_CLOCK: time_show(&game->time); break;
            case TIME_WAIT: time_wait(&game->time); break;
            case TIME_SLEEP: sleep_at_home(game); break;
            default: printf("Нет такого пункта.\n"); break;
        }
    }
}


// Пункты меню, которым нужен персонаж
static void handle_player_action(struct Game *game, int choice) {
    struct Player *p = game->player;
    char (*names)[MAX_NAME] = game->item_names;

    if (!require_player(game)) return;

    switch (choice) {
        case MENU_INVENTORY: inventory_show(p, names); break;
        case MENU_PUT: inventory_put(p, names); break;
        case MENU_DROP: inventory_drop(p); break;
        case MENU_QUICK: inventory_quick(p, names); break;
        case MENU_FIND: inventory_find(p, names); break;
        case MENU_DIARY: diary_write(p, &game->time, names); break;
        case MENU_ADVENTURE: adventure_run(p, &game->time); break;
        case MENU_PRICES: price_report(p, &game->time, names); break;
        case MENU_EQUIPMENT: equipment_menu(p, game->catalog); break;
        default: break;
    }
}


// Выбираем действие по номеру пункта меню
static void handle_choice(struct Game *game, int choice) {
    switch (choice) {
        case MENU_EXIT:
            printf("Пока!\n");
            break;
        case MENU_CREATE:
            create_character(game);
            break;
        case MENU_CARD:
            show_card(game);
            break;
        case MENU_TIME:
            time_menu(game);
            break;
        case MENU_INVENTORY:
        case MENU_PUT:
        case MENU_DROP:
        case MENU_QUICK:
        case MENU_FIND:
        case MENU_DIARY:
        case MENU_ADVENTURE:
        case MENU_PRICES:
        case MENU_EQUIPMENT:
            handle_player_action(game, choice);
            break;
        default:
            printf("Нет такой команды.\n");
            break;
    }
}


// Точка входа: создаём игру, крутим меню до выхода, очищаем память
int main(void) {
    struct Game game;
    int choice = -1;
    int status;

    setup_console();
    srand((unsigned)time(NULL));

    game_init(&game);

    while (choice != MENU_EXIT) {
        print_menu(&game);
        status = read_int(&choice);
        if (status == INPUT_EOF) break;
        if (status != INPUT_OK) {
            choice = -1;
            continue;
        }
        handle_choice(&game, choice);
    }

    game_cleanup(&game);
    return 0;
}
