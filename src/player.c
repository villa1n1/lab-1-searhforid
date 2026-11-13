#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "player.h"


static int ask_player_name(char *out) {
    char buf[INPUT_SIZE];

    while (1) {
        printf("Введите имя фермера: ");
        if (!read_line(buf, sizeof(buf))) return 0;
        if (strlen(buf) == 0) {
            strcpy(out, DEFAULT_NAME);
            return 1;
        }
        if (strlen(buf) >= NAME_LEN) {
            printf("Слишком длинное имя (максимум %d байт, русская буква - 2 байта)!\n", NAME_LEN - 1);
            continue;
        }
        strcpy(out, buf);
        return 1;
    }
}


struct Player *player_create(void) {
    struct Player *p;
    int i;

    p = (struct Player*)malloc(sizeof(struct Player));
    if (p == NULL) {
        printf("Ошибка: не хватает памяти!\n");
        return NULL;
    }

    if (!ask_player_name(p->name)) {
        free(p);
        return NULL;
    }

    p->base_max_hp = START_MAX_HP;
    p->max_hp = START_MAX_HP;
    p->hp = START_MAX_HP;
    p->base_max_stamina = START_MAX_STAMINA;
    p->max_stamina = START_MAX_STAMINA;
    p->stamina = START_MAX_STAMINA;
    p->gold = START_GOLD;
    p->x = 0;
    p->y = 0;
    for (i = 0; i < INV_SIZE; i++) p->inventory[i] = i;

    printf("Персонаж создан по адресу %p.\n", (void*)p);
    return p;
}


void player_free(struct Player **player) {
    if (player == NULL || *player == NULL) return;

    printf("Память структуры по адресу %p освобождена\n", (void*)*player);
    free(*player);
    *player = NULL;
}


void player_print_card(const struct Player *p, const struct GameTime *t, char names[][MAX_NAME]) {
    printf("\n===== Карточка персонажа =====\n");
    printf("Имя: %s\n", p->name);
    printf("Здоровье: %d/%d\n", p->hp, p->max_hp);
    printf("Выносливость: %d/%d\n", p->stamina, p->max_stamina);
    printf("Золото: %d\n", p->gold);
    time_show(t);
    inventory_show(p, names);
}


void player_print_hud(const struct Player *p, int floor, const struct GameTime *t) {
    printf("HP: %d/%d | Выносливость: %d/%d | Золото: %d | Этаж: %d | Время: %02d:00\n",
           p->hp, p->max_hp, p->stamina, p->max_stamina, p->gold, floor, time_clock_hour(t));
}


void player_rest(struct Player *p, struct GameTime *t) {
    time_next_morning(t);
    p->hp = p->max_hp;
    p->stamina = p->max_stamina;
    printf("%s выспался. Здоровье и выносливость восстановлены.\n", p->name);
    time_show(t);
}


void player_spend_stamina(struct Player *p, int amount) {
    p->stamina -= amount;
    if (p->stamina < 0) p->stamina = 0;
}


void inventory_show(const struct Player *p, char names[][MAX_NAME]) {
    int i;

    printf("Инвентарь:\n");
    for (i = 0; i < INV_SIZE; i++)
        printf("  Слот %d: [%d] - %s\n", i, p->inventory[i], names[p->inventory[i]]);
}


void inventory_put(struct Player *p, char names[][MAX_NAME]) {
    int idx, id;
    char prompt[INPUT_SIZE];

    sprintf(prompt, "Индекс слота (0-%d): ", INV_SIZE - 1);
    if (read_int_range(prompt, 0, INV_SIZE - 1, &idx) != INPUT_OK) return;
    sprintf(prompt, "ID предмета (0-%d): ", MAX_ITEM_ID);
    if (read_int_range(prompt, 0, MAX_ITEM_ID, &id) != INPUT_OK) return;

    p->inventory[idx] = id;
    printf("В слот %d положен [%d] %s.\n", idx, id, names[id]);
}


void inventory_drop(struct Player *p) {
    int idx;
    char prompt[INPUT_SIZE];

    sprintf(prompt, "Индекс слота (0-%d): ", INV_SIZE - 1);
    if (read_int_range(prompt, 0, INV_SIZE - 1, &idx) != INPUT_OK) return;

    p->inventory[idx] = 0;
    printf("Слот %d очищен.\n", idx);
}


void inventory_quick(struct Player *p, char names[][MAX_NAME]) {
    int id, found, i, t;
    char prompt[INPUT_SIZE];

    sprintf(prompt, "ID предмета (1-%d): ", MAX_ITEM_ID);
    if (read_int_range(prompt, 1, MAX_ITEM_ID, &id) != INPUT_OK) return;

    found = -1;
    for (i = 0; i < INV_SIZE; i++)
        if (p->inventory[i] == id) { found = i; break; }

    if (found == -1) { printf("Такого предмета нет.\n"); return; }
    if (found == 0)  { printf("Уже в слоте 0.\n"); return; }

    t = p->inventory[0];
    p->inventory[0] = p->inventory[found];
    p->inventory[found] = t;
    printf("Обменян со слотом %d.\n", found);
    inventory_show(p, names);
}


void inventory_find(const struct Player *p, char names[][MAX_NAME]) {
    char query[MAX_NAME];
    int id, any, i;

    printf("Название предмета: ");
    read_line(query, sizeof(query));

    id = -1;
    for (i = 0; i < MAX_ITEMS; i++)
        if (strcmp(names[i], query) == 0) { id = i; break; }

    if (id <= 0) { printf("Такого предмета в каталоге нет.\n"); return; }

    printf("«%s» лежит в слотах: ", query);
    any = 0;
    for (i = 0; i < INV_SIZE; i++)
        if (p->inventory[i] == id) { printf("%d ", i); any = 1; }
    if (!any) printf("(нет)");
    printf("\n");
}


void diary_write(const struct Player *p, const struct GameTime *t, char names[][MAX_NAME]) {
    FILE *f;
    int i;

    f = open_file(DIARY_FILE, "a");
    if (!f) return;

    fprintf(f, "Фермер: %s | День %d, %02d:00\n", p->name, t->day, time_clock_hour(t));
    for (i = 0; i < INV_SIZE; i++)
        fprintf(f, "  Слот %d: %s\n", i, names[p->inventory[i]]);
    fprintf(f, "\n");
    fclose(f);
    printf("Записано в %s.\n", DIARY_FILE);
}


void price_report(const struct Player *p, const struct GameTime *t, char names[][MAX_NAME]) {
    char pname[MAX_PRICES][MAX_NAME];
    int pprice[MAX_PRICES];
    int count, i, j, price, total, best, best_slot;
    char *nm;
    FILE *out;

    count = load_prices(pname, pprice, MAX_PRICES);
    if (count < 0) return;
    if (count == 0) { printf("Прайс пуст.\n"); return; }

    out = open_file(REPORT_FILE, "w");
    if (!out) return;

    fprintf(out, "--- Прайс: %s ---\n", p->name);
    fprintf(out, "День %d, %02d:00\n\n", t->day, time_clock_hour(t));
    printf("--- Прайс: %s ---\n", p->name);
    printf("День %d, %02d:00\n\n", t->day, time_clock_hour(t));

    total = 0;
    best = -1;
    best_slot = -1;

    for (i = 0; i < INV_SIZE; i++) {
        if (p->inventory[i] == 0) continue;
        nm = names[p->inventory[i]];

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
        fprintf(out, "\nСамый дорогой: %s (%d)\n", names[p->inventory[best_slot]], best);
        printf ("\nСамый дорогой: %s (%d)\n", names[p->inventory[best_slot]], best);
    }
    fprintf(out, "Итог: %d\n", total);
    printf ("Итог: %d\n", total);

    fclose(out);
    printf("\nСохранено в %s.\n", REPORT_FILE);
}
