#include "include/base.h"

// where to create new ones
#define PEON_HOTSPOT_X 30
#define PEON_HOTSPOT_Y 107

void add_peon(game_state *gs) {
  // if there's an empty slot, add it. otherwise oh well
  for (int i = 0; i < ARRAY_COUNT(gs->peons); i++) {
    if (gs->peons[i] == 0) {
      gs->peons[i] = (peon *)malloc(sizeof(peon));
      gs->peons[i]->pos.x = (rand() % 12) - 6 + PEON_HOTSPOT_X;
      gs->peons[i]->pos.y = (rand() % 8) - 4 + PEON_HOTSPOT_Y;
      gs->peons[i]->pos.h = 6;
      gs->peons[i]->pos.w = 6;
      gs->peons[i]->frame = 0;
      gs->peons[i]->which = i;
      gs->peons[i]->alive = true;
      gs->peons[i]->speed = (rand() % 60) + 10;

      SDL_Log("peon at %d,%d", gs->peons[i]->pos.x, gs->peons[i]->pos.y);
      break;
    }
  }
}
void tick_peons(game_state *gs) {
  for (int i = 0; i < ARRAY_COUNT(gs->peons); i++) {
    if (gs->peons[i] != 0) {
      if (!gs->peons[i]->alive) {
        SDL_Log("killed peon %d", gs->peons[i]->which);
        free(gs->peons[i]);
        gs->peons[i] = 0;
        continue;
      }
      if (gs->peons[i]->ticks % 10 == 0) {
        gs->peons[i]->frame = (gs->peons[i]->frame + 1) % 3;
      }
      if (gs->peons[i]->ticks % gs->peons[i]->speed ==
          0) { // obvioulsy speed of movement..
        gs->peons[i]->pos.x += 1;
        ;
      }
      if (rand() % 300 == 0) {
        gs->peons[i]->pos.y += (rand() % 3) - 1;
      }
      if (gs->peons[i]->pos.x > 128) {
        gs->peons[i]->alive = false;
      }
      gs->peons[i]->ticks++;
    }
  }
}
void render_peons(game_state *gs) {
  for (int i = 0; i < ARRAY_COUNT(gs->peons); i++) {
    if (gs->peons[i] != 0) {
      SDL_Rect src;
      src.x = gs->peons[i]->frame * 3;
      src.y = 0;
      src.w = 3;
      src.h = 3;
      // SDL_RenderCopy(gs->sdlRenderer, gs->default_peon, &src,
      //                &gs->peons[i]->pos);
      push_drawitem(&gs->drawitems, gs->default_peon, &src, &gs->peons[i]->pos);
      // todo handle some Z ordering with a tiny bit of sorting before blitting
      // directly
    }
  }
}
