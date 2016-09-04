#include "include/base.h"

// where to create new ones
// #define PEON_HOTSPOT_X 30
// #define PEON_HOTSPOT_Y 107
#define PEON_HOTSPOT_X 55
#define PEON_HOTSPOT_Y 100

void add_peon(game_state *gs) {
  // if there's an empty slot, add it. otherwise oh well
  for (int i = 0; i < ARRAY_COUNT(gs->peons); i++) {
    if (gs->peons[i] == 0) {
      gs->peons[i] = (peon *)malloc(sizeof(peon));
      peon *p = gs->peons[i];
      p->type = PEON_NORMAL;
      gs->peons[i]->pos.x = (rand() % 12) - 6 + PEON_HOTSPOT_X;
      gs->peons[i]->pos.y = (rand() % 8) - 4 + PEON_HOTSPOT_Y;
      p->speed = ((rand() % 8) * 0.1f) + 0.1f;

      gs->peons[i]->size.h = 5;
      gs->peons[i]->size.w = 5;
      gs->peons[i]->frame = 0;
      gs->peons[i]->which = i;
      gs->peons[i]->alive = true;

      if (p->which % 10 == 0) {
        p->type = PEON_CHICKEN;
        p->size.h = 16;
        p->size.w = 16;
      }
      p->target = gs->targets[(rand() % ARRAY_COUNT(gs->targets))];
      p->dir = normalize(p->target->pos - p->pos);

      SDL_Log("peon at %f,%f", gs->peons[i]->pos.x, gs->peons[i]->pos.y);
      break;
    }
  }
}
void tick_peons(game_state *gs) {
  for (int i = 0; i < ARRAY_COUNT(gs->peons); i++) {
    if (gs->peons[i] != 0) {
      peon *p = gs->peons[i];
      if (!gs->peons[i]->alive) {
        SDL_Log("killed peon %d", gs->peons[i]->which);
        free(gs->peons[i]);
        gs->peons[i] = 0;
        continue;
      }

      p->dir = normalize(p->target->pos - p->pos);
      p->pos = p->pos + (p->dir * p->speed);

      if (hit_target(p->target, p->pos)) {
        if (p->target->next != 0) {
          p->target = p->target->next;
        }
      }

      if (p->ticks % 10 == 0) {
        p->frame = (p->frame + 1) % 5;
      }
      if (gs->peons[i]->ticks % 60 == 0) {
        // change p->target here instead
        // gs->peons[i]->speed.x = float((rand() % 5) - 2) * 0.1;
        // gs->peons[i]->speed.y = float((rand() % 5) - 2) * 0.1;
      }
      if (gs->peons[i]->pos.x > 128 || gs->peons[i]->pos.y < 60) {
        // gs->peons[i]->alive = false;
      }
      gs->peons[i]->ticks++;
    }
  }
}
void render_peons(game_state *gs) {
  for (int i = 0; i < ARRAY_COUNT(gs->peons); i++) {
    if (gs->peons[i] != 0) {
      peon *p = gs->peons[i];
      SDL_Rect src, dst;
      src.x = gs->peons[i]->frame * 5;
      src.y = (gs->peons[i]->which % 7) * 5;
      src.w = 5;
      src.h = 5;
      dst.x = int(p->pos.x);
      dst.y = int(p->pos.y);
      dst.w = 5;
      dst.h = 5;
      // SDL_RenderCopy(gs->sdlRenderer, gs->default_peon, &src,
      //                &gs->peons[i]->pos);
      if (p->type == PEON_CHICKEN) {
        src.x = p->frame * 16;
        src.y = 0;
        src.w = 16;
        src.h = 16;
        dst.w = 16;
        dst.h = 16;
        push_drawitem(&gs->drawitems, gs->chicken.tex, &src, &dst);
      } else {
        push_drawitem(&gs->drawitems, gs->default_peon, &src, &dst);
      }
      // todo handle some Z ordering with a tiny bit of sorting before blitting
      // directly
    }
  }
}
