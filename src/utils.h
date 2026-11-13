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
#define DEADLINE_HOUR  26
#define HOURS_IN_DAY   24

enum InputStatus {
    INPUT_EOF = -1,
    INPUT_BAD = 0,
    INPUT_OK = 1
};

struct GameTime {
    int day;
    int hour;
};

int  read_int(int *out);
int  read_int_range(const char *prompt, int min, int max, int *out);
int  read_line(char *out, int size);
int  ask_yes_no(const char *question);
int  random_range(int a, int b);

void time_init(struct GameTime *t);
void time_show(const struct GameTime *t);
int  time_clock_hour(const struct GameTime *t);
int  time_hours_left(const struct GameTime *t);
int  time_is_night(const struct GameTime *t);
void time_add_hours(struct GameTime *t, int hours);
void time_next_morning(struct GameTime *t);
void time_wait(struct GameTime *t);

void set_default_item_names(char names[][MAX_NAME]);
void load_item_names(char names[][MAX_NAME]);
int  load_prices(char names[][MAX_NAME], int prices[], int max);
FILE *open_file(const char *path, const char *mode);

#endif
