#include <stdio.h>

#define INV_SIZE 10
#define BUF_SIZE 100

int current_day  = 1;
int current_hour = 8;
int inventory[INV_SIZE] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

const char *item_name(int id) {
    switch (id) {
        case 0: return "Пусто";
        case 1: return "Дерево";
        case 2: return "Камень";
        case 3: return "Семена";
        case 4: return "Железо";
        case 5: return "Золото";
        case 6: return "Зелье";
        case 7: return "Факел";
        case 8: return "Верёвка";
        case 9: return "Хлеб";
        default: return "Неизвестно";
    }
}

int read_int(int *out) {
    char buf[BUF_SIZE];
    if (fgets(buf, sizeof(buf), stdin) == NULL) {
        return 0;
    }
    if (sscanf(buf, "%d", out) != 1) {
        printf("Это не число! Попробуйте ещё раз.\n");
        return 0;
    }
    return 1;
}

void print_menu() {
    printf("\n===== МЕНЮ =====\n");
    printf("[0] Выход\n");
    printf("[1] Посмотреть на часы\n");
    printf("[2] Промотать время (Поработать)\n");
    printf("[3] Посмотреть инвентарь\n");
    printf("[4] Положить предмет в слот\n");
    printf("[5] Выбросить предмет\n");
    printf("[6] Быстрый доступ\n");
    printf("Ваш выбор: ");
}

void show_clock() {
    printf("Текущее время: День %d, %02d:00\n", current_day, current_hour);
}

void pass_time() {
    int hours;
    printf("Сколько часов потратить на работу? ");
    if (!read_int(&hours)) {
        return;
    }
    if (hours < 0) {
        printf("Часы не могут быть отрицательными!\n");
        return;
    }
    if (hours > 100) {
        printf("Слишком много часов за раз! (максимум 100)\n");
        return;
    }
    current_hour = current_hour + hours;
    while (current_hour >= 24) {
        current_hour = current_hour - 24;
        current_day = current_day + 1;
    }
    show_clock();
}

void show_inventory() {
    int i;
    for (i = 0; i < INV_SIZE; i++) {
        printf("Слот %d: [%d] (%s)\n", i, inventory[i], item_name(inventory[i]));
    }
}

void put_item() {
    int idx, id;
    printf("Индекс слота (0-%d): ", INV_SIZE - 1);
    if (!read_int(&idx)) return;
    if (idx < 0 || idx >= INV_SIZE) {
        printf("Ошибка: индекс вне массива!\n");
        return;
    }
    printf("ID предмета (0-9): ");
    if (!read_int(&id)) return;
    if (id < 0 || id > 9) {
        printf("Ошибка: такого ID нет!\n");
        return;
    }
    inventory[idx] = id;
    printf("В слот %d размещён предмет [%d] (%s).\n", idx, id, item_name(id));
}

void drop_item() {
    int idx;
    printf("Индекс слота (0-%d): ", INV_SIZE - 1);
    if (!read_int(&idx)) return;
    if (idx < 0 || idx >= INV_SIZE) {
        printf("Ошибка: индекс вне массива (0-%d).\n", INV_SIZE - 1);
        return;
    }
    if (inventory[idx] == 0) {
        printf("Слот %d пуст!\n", idx);
        return;
    }
    inventory[idx] = 0;
    printf("Слот %d очищен.\n", idx);
}

void varik() {
    int id;
    int found = -1;
    int temp;
    int i;
    printf("\n--- Инвентарь до ---\n");
    for (i = 0; i < INV_SIZE; i++) {
        printf("Слот %d: [%d] (%s)\n", i, inventory[i], item_name(inventory[i]));
    }
    printf("\nВведите ID предмета (0-9): ");
    if (!read_int(&id)) return;
    if (id < 0 || id > 9) {
        printf("Ошибка: такого ID нет (допустимо 0-9)!\n");
        return;
    }
    if (id == 0) {
        printf("ID 0 - это пустой слот!\n");
        return;
    }
    for (i = 0; i < INV_SIZE; i++) {
        if (inventory[i] == id) {
            found = i;
            break;
        }
    }
    if (found == -1) {
        printf("Предмета с ID %d (%s) нет в инвентаре!\n", id, item_name(id));
        return;
    }
    if (found == 0) {
        printf("Предмет уже в слоте 0!\n");
        return;
    }

    temp = inventory[0];
    inventory[0] = inventory[found];
    inventory[found] = temp;

    printf("\nНайден в слоте %d, поменян местами со слотом 0.\n", found);
    printf("\n--- Инвентарь после изменений ---\n");
    for (i = 0; i < INV_SIZE; i++) {
        printf("Слот %d: [%d] (%s)\n", i, inventory[i], item_name(inventory[i]));
    }
}

int main() {
    int choice;
    do {
        print_menu();
        if (!read_int(&choice)) {
            continue;
        }
        switch (choice) {
            case 0:
                printf("Выход из игры.\n");
                break;
            case 1:
                show_clock();
                break;
            case 2:
                pass_time();
                break;
            case 3:
                show_inventory();
                break;
            case 4:
                put_item();
                break;
            case 5:
                drop_item();
                break;
            case 6:
                varik();
                break;
            default:
                printf("Неизвестная команда. Попробуйте снова.\n");
                break;
        }
    } while (choice != 0);
    return 0;
}