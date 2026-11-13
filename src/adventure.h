#ifndef ADVENTURE_H
#define ADVENTURE_H

#include "player.h"
#include "map.h"

#define STEP_COST          1
#define FLOOR_COST         5
#define FLOOR_HOURS        1
#define GOLD_PER_FLOOR     10
#define FAINT_GOLD_DIVIDER 2

void adventure_run(struct Player *player, struct GameTime *time);

#endif
