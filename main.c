#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <stdlib.h>

#define INV_SIZE     10
#define MAX_ITEMS    10
#define MAX_NAME     64


int current_day  = 1;
int current_hour = 8;
int inventory[INV_SIZE] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

char farmer_name[33] = "Фермер";

char item_names[MAX_ITEMS][MAX_NAME] = {
    "Пусто", "Дерево", "Камень", "Семена",
    "Железо", "Золото", "Зелье", "Факел",
    "Верёвка", "Хлеб"
};


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


int main(void) {
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    
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

    if (count == 0) { printf("Прайс пуст.\n"); return; }

    out = fopen("output.txt", "w");
    if (!out) { printf("Не создать output.txt.\n"); return; }

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
