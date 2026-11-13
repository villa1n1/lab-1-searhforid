#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"


int read_int(int *out) {
    char buf[INPUT_SIZE];
    char rest;

    if (fgets(buf, sizeof(buf), stdin) == NULL) return INPUT_EOF;
    if (sscanf(buf, "%d %c", out, &rest) != 1) {
        printf("Это не число!\n");
        return INPUT_BAD;
    }
    return INPUT_OK;
}


int read_int_range(const char *prompt, int min, int max, int *out) {
    int status;

    printf("%s", prompt);
    status = read_int(out);
    if (status != INPUT_OK) return status;
    if (*out < min || *out > max) {
        printf("Число должно быть от %d до %d!\n", min, max);
        return INPUT_BAD;
    }
    return INPUT_OK;
}


int read_line(char *out, int size) {
    size_t len;

    if (fgets(out, size, stdin) == NULL) {
        out[0] = '\0';
        return 0;
    }
    len = strlen(out);
    if (len > 0 && out[len - 1] == '\n') out[len - 1] = '\0';
    return 1;
}


int ask_yes_no(const char *question) {
    char buf[INPUT_SIZE];

    while (1) {
        printf("%s (д/н): ", question);
        if (!read_line(buf, sizeof(buf))) return 0;
        if (strcmp(buf, "д") == 0 || strcmp(buf, "Д") == 0 ||
            strcmp(buf, "y") == 0 || strcmp(buf, "Y") == 0) return 1;
        if (strcmp(buf, "н") == 0 || strcmp(buf, "Н") == 0 ||
            strcmp(buf, "n") == 0 || strcmp(buf, "N") == 0) return 0;
        printf("Ответьте «д» или «н».\n");
    }
}


int random_range(int a, int b) {
    return a + rand() % (b - a + 1);
}


void time_init(struct GameTime *t) {
    t->day = 1;
    t->hour = START_HOUR;
}


int time_clock_hour(const struct GameTime *t) {
    return t->hour % HOURS_IN_DAY;
}


void time_show(const struct GameTime *t) {
    printf("День %d, %02d:00\n", t->day, time_clock_hour(t));
}


int time_hours_left(const struct GameTime *t) {
    int left = DEADLINE_HOUR - t->hour;

    if (left < 0) return 0;
    return left;
}


int time_is_night(const struct GameTime *t) {
    return t->hour >= DEADLINE_HOUR;
}


void time_add_hours(struct GameTime *t, int hours) {
    t->hour += hours;
}


void time_next_morning(struct GameTime *t) {
    t->day++;
    t->hour = START_HOUR;
}


void time_wait(struct GameTime *t) {
    int left = time_hours_left(t);
    int hours;
    char prompt[INPUT_SIZE];

    if (left <= 0) {
        printf("Уже 02:00 ночи - пора спать, ждать нельзя.\n");
        return;
    }
    sprintf(prompt, "Сколько часов ждать (1-%d)? ", left);
    if (read_int_range(prompt, 1, left, &hours) != INPUT_OK) return;
    time_add_hours(t, hours);
    time_show(t);
}


void set_default_item_names(char names[][MAX_NAME]) {
    const char *defaults[MAX_ITEMS] = {
        "Пусто", "Дерево", "Камень", "Семена",
        "Железо", "Золото", "Зелье", "Факел",
        "Верёвка", "Хлеб"
    };
    int i;

    for (i = 0; i < MAX_ITEMS; i++) {
        strncpy(names[i], defaults[i], MAX_NAME - 1);
        names[i][MAX_NAME - 1] = '\0';
    }
}


void load_item_names(char names[][MAX_NAME]) {
    FILE *f;
    char line[INPUT_SIZE];
    char *p;
    size_t len;
    int i;

    set_default_item_names(names);

    f = fopen(ITEMS_FILE, "r");
    if (!f) {
        printf("%s не найден — используются базовые названия.\n", ITEMS_FILE);
        return;
    }
    i = 0;
    while (i < MAX_ITEMS && fgets(line, sizeof(line), f)) {
        p = strchr(line, ' ');
        if (!p) continue;
        p++;
        len = strlen(p);
        if (len > 0 && p[len - 1] == '\n') p[len - 1] = '\0';
        strncpy(names[i], p, MAX_NAME - 1);
        names[i][MAX_NAME - 1] = '\0';
        i++;
    }
    fclose(f);
    printf("Каталог загружен из %s.\n", ITEMS_FILE);
}


int load_prices(char names[][MAX_NAME], int prices[], int max) {
    FILE *f;
    char line[LINE_SIZE];
    char *sp;
    size_t len;
    int count = 0;

    f = fopen(PRICES_FILE, "r");
    if (!f) {
        printf("%s не найден.\n", PRICES_FILE);
        return -1;
    }

    while (fgets(line, sizeof(line), f) && count < max) {
        len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') line[len - 1] = '\0';
        if (strlen(line) == 0) continue;

        sp = strrchr(line, ' ');
        if (!sp) continue;
        *sp = '\0';
        strncpy(names[count], line, MAX_NAME - 1);
        names[count][MAX_NAME - 1] = '\0';
        prices[count] = atoi(sp + 1);
        count++;
    }
    fclose(f);
    return count;
}


FILE *open_file(const char *path, const char *mode) {
    FILE *f = fopen(path, mode);

    if (!f) printf("Не удалось открыть %s.\n", path);
    return f;
}
