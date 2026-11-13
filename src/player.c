#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "player.h"


static void clear_slot(struct Player *p, int slot) {
    p->equipment[slot].name[0] = '\0';
    p->equipment[slot].slot = (enum SlotType)slot;
    p->equipment[slot].bonus_hp = 0;
    p->equipment[slot].bonus_stamina = 0;
}


static const char *slot_title(enum SlotType slot) {
    switch (slot) {
        case SLOT_HEAD:  return "Голова";
        case SLOT_BODY:  return "Тело";
        case SLOT_BOOTS: return "Обувь";
        default:         return "?";
    }
}


static void print_equipment_line(const struct Equipment *e) {
    printf("%s +%d HP, +%d выносливости", e->name, e->bonus_hp, e->bonus_stamina);
}


static void print_slots(const struct Player *p) {
    int i;

    printf("Снаряжение:\n");
    for (i = 0; i < SLOT_COUNT; i++) {
        printf("  %d) %-7s: ", i + 1, slot_title((enum SlotType)i));
        if (p->equipment[i].name[0] == '\0') printf("(пусто)");
        else print_equipment_line(&p->equipment[i]);
        printf("\n");
    }
}


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
    for (i = 0; i < SLOT_COUNT; i++) clear_slot(p, i);

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
    print_slots(p);
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


void player_recalc_stats(struct Player *p) {
    int i;

    p->max_hp = p->base_max_hp;
    p->max_stamina = p->base_max_stamina;
    for (i = 0; i < SLOT_COUNT; i++) {
        p->max_hp += p->equipment[i].bonus_hp;
        p->max_stamina += p->equipment[i].bonus_stamina;
    }
    if (p->hp > p->max_hp) p->hp = p->max_hp;
    if (p->stamina > p->max_stamina) p->stamina = p->max_stamina;
}


static void set_equipment(struct Equipment *e, const char *name, enum SlotType slot, int bonus_hp, int bonus_stamina) {
    strncpy(e->name, name, MAX_NAME - 1);
    e->name[MAX_NAME - 1] = '\0';
    e->slot = slot;
    e->bonus_hp = bonus_hp;
    e->bonus_stamina = bonus_stamina;
}


void equipment_fill_catalog(struct Equipment catalog[]) {
    set_equipment(&catalog[0], "Кожаная шапка", SLOT_HEAD, 5, 0);
    set_equipment(&catalog[1], "Железный шлем", SLOT_HEAD, 15, 0);
    set_equipment(&catalog[2], "Холщовая куртка", SLOT_BODY, 10, 5);
    set_equipment(&catalog[3], "Кольчуга", SLOT_BODY, 30, 0);
    set_equipment(&catalog[4], "Лёгкие сапоги", SLOT_BOOTS, 0, 10);
    set_equipment(&catalog[5], "Сапоги-скороходы", SLOT_BOOTS, 5, 25);
}


static void equip_item(struct Player *p, const struct Equipment catalog[]) {
    int i, choice;
    char prompt[INPUT_SIZE];

    printf("Доступное снаряжение:\n");
    for (i = 0; i < EQUIP_CATALOG_SIZE; i++) {
        printf("  %d) [%s] ", i + 1, slot_title(catalog[i].slot));
        print_equipment_line(&catalog[i]);
        printf("\n");
    }
    sprintf(prompt, "Что надеть (1-%d)? ", EQUIP_CATALOG_SIZE);
    if (read_int_range(prompt, 1, EQUIP_CATALOG_SIZE, &choice) != INPUT_OK) return;

    p->equipment[catalog[choice - 1].slot] = catalog[choice - 1];
    player_recalc_stats(p);
    printf("Надето: %s. Макс. здоровье: %d, макс. выносливость: %d.\n",
           catalog[choice - 1].name, p->max_hp, p->max_stamina);
}


static void unequip_item(struct Player *p) {
    int choice;
    char prompt[INPUT_SIZE];

    sprintf(prompt, "Какой слот освободить (1-%d)? ", SLOT_COUNT);
    if (read_int_range(prompt, 1, SLOT_COUNT, &choice) != INPUT_OK) return;

    if (p->equipment[choice - 1].name[0] == '\0') {
        printf("Этот слот и так пуст.\n");
        return;
    }
    printf("Снято: %s.\n", p->equipment[choice - 1].name);
    clear_slot(p, choice - 1);
    player_recalc_stats(p);
    printf("Макс. здоровье: %d, макс. выносливость: %d.\n", p->max_hp, p->max_stamina);
}


void equipment_menu(struct Player *p, const struct Equipment catalog[]) {
    int choice, status;

    while (1) {
        printf("\n===== Управление снаряжением =====\n");
        print_slots(p);
        printf("Здоровье: %d/%d, выносливость: %d/%d\n", p->hp, p->max_hp, p->stamina, p->max_stamina);
        printf("1) Надеть вещь\n2) Снять вещь\n0) Назад\nВыбор: ");
        status = read_int(&choice);
        if (status == INPUT_EOF) return;
        if (status != INPUT_OK) continue;
        if (choice == 0) return;
        if (choice == 1) equip_item(p, catalog);
        else if (choice == 2) unequip_item(p);
        else printf("Нет такого пункта.\n");
    }
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
