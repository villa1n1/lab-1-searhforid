#ifndef PLAYER_H
#define PLAYER_H

#include "utils.h"

#define NAME_LEN           33
#define DEFAULT_NAME       "Фермер"
#define START_MAX_HP       100
#define START_MAX_STAMINA  50
#define START_GOLD         0
#define EQUIP_CATALOG_SIZE 6

enum SlotType {
    SLOT_HEAD = 0,
    SLOT_BODY,
    SLOT_BOOTS,
    SLOT_COUNT
};

// вещь снаряжения: слот, в который она надевается, и бонусы к максимумам
struct Equipment {
    char name[MAX_NAME];
    enum SlotType slot;
    int bonus_hp;
    int bonus_stamina;
};

// структура героя: характеристики, координаты, инвентарь и 3 слота снаряжения
struct Player {
    char name[NAME_LEN];
    int hp;
    int max_hp;
    int base_max_hp;
    int stamina;
    int max_stamina;
    int base_max_stamina;
    int gold;
    int x, y;
    int inventory[INV_SIZE];
    struct Equipment equipment[SLOT_COUNT];
};

// Создаём героя в динамической памяти и задаём стартовые характеристики
struct Player *player_create(void);
// Освобождаем память героя и обнуляем указатель
void player_free(struct Player **player);
// Карточка героя: характеристики, время, снаряжение и инвентарь
void player_print_card(const struct Player *p, const struct GameTime *t, char names[][MAX_NAME]);
// Строка состояния (HUD) во время похода
void player_print_hud(const struct Player *p, int floor, const struct GameTime *t);
// Сон: следующий день, утро, здоровье и выносливость на максимум
void player_rest(struct Player *p, struct GameTime *t);
// Тратим выносливость, но не уходим ниже нуля
void player_spend_stamina(struct Player *p, int amount);
// Пересчитываем максимумы: базовое значение плюс бонусы всей экипировки
void player_recalc_stats(struct Player *p);

// Показать инвентарь
void inventory_show(const struct Player *p, char names[][MAX_NAME]);
// Положить предмет в слот
void inventory_put(struct Player *p, char names[][MAX_NAME]);
// Выбросить предмет из слота
void inventory_drop(struct Player *p);
// Быстрый доступ: меняем найденный предмет местами с нулевым слотом
void inventory_quick(struct Player *p, char names[][MAX_NAME]);
// Поиск предмета по названию
void inventory_find(const struct Player *p, char names[][MAX_NAME]);

// Каталог вещей, которые можно надеть
void equipment_fill_catalog(struct Equipment catalog[]);
// Меню управления снаряжением
void equipment_menu(struct Player *p, const struct Equipment catalog[]);

// Дописываем состояние героя в дневник
void diary_write(const struct Player *p, const struct GameTime *t, char names[][MAX_NAME]);
// Оценка инвентаря по прайс-листу, отчёт пишем в output.txt
void price_report(const struct Player *p, const struct GameTime *t, char names[][MAX_NAME]);

#endif
