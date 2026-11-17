#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

#define INPUT_SIZE     100
#define LINE_SIZE      200
#define MAX_NAME       64
#define MAX_ITEMS      10
#define MAX_ITEM_ID    9
#define INV_SIZE       10
#define MAX_PRICES     64

#define ITEMS_FILE     "data/items.txt"
#define PRICES_FILE    "data/input.txt"
#define DIARY_FILE     "data/diary.txt"
#define REPORT_FILE    "data/output.txt"

#define START_HOUR     8
// в 02:00 ночи (26-й час) героя клонит в сон
#define DEADLINE_HOUR  26
#define HOURS_IN_DAY   24

enum InputStatus {
    INPUT_EOF = -1,
    INPUT_BAD = 0,
    INPUT_OK = 1
};

// время игры: час может доходить до 26 - это 02:00 следующей ночи
struct GameTime {
    int day;
    int hour;
};

// Читаем целое число из строки (мусор после числа не принимаем)
int  read_int(int *out);
// Читаем число и проверяем, что оно в диапазоне от min до max
int  read_int_range(const char *prompt, int min, int max, int *out);
// Читаем строку без символа переноса в конце
int  read_line(char *out, int size);
// Задаём вопрос и ждём ответ да или нет
int  ask_yes_no(const char *question);
// Случайное число от a до b включительно
int  random_range(int a, int b);

// Начало игры: день 1, 08:00
void time_init(struct GameTime *t);
// Печатаем день и время
void time_show(const struct GameTime *t);
// Час на часах (после полуночи счёт идёт с 0)
int  time_clock_hour(const struct GameTime *t);
// Сколько часов осталось до 02:00 ночи
int  time_hours_left(const struct GameTime *t);
// Уже ночь (02:00 и позже)?
int  time_is_night(const struct GameTime *t);
// Прибавляем часы
void time_add_hours(struct GameTime *t, int hours);
// Новый день, утро в 08:00
void time_next_morning(struct GameTime *t);
// Ждать: можно промотать время, но не дальше 02:00
void time_wait(struct GameTime *t);

// Названия предметов по умолчанию, если нет файла
void set_default_item_names(char names[][MAX_NAME]);
// Загружаем названия предметов из файла (если файла нет - остаются по умолчанию)
void load_item_names(char names[][MAX_NAME]);
// Читаем прайс-лист из файла, возвращаем количество записей (-1 если файла нет)
int  load_prices(char names[][MAX_NAME], int prices[], int max);
// Открываем файл и сообщаем, если не получилось
FILE *open_file(const char *path, const char *mode);

#endif
