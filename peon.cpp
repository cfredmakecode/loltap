#include "include/base.h"

// TODO? we could return the peon added so one can do things with it immediately
// like target / move / speed / etc
internal void add_peon(peon_stack *peons, game_state *gs) {
  if (peons->stack == 0) {
    peons->capacity = 1000;
    peons->stack = (peon *)calloc(peons->capacity, sizeof(peon));
    peons->count = 0;
    ASSERT(gs->targets[0]);
    peons->target = gs->targets[0];
  }
  ASSERT(peons->count + 1 < 1 || peons->capacity);
  for (int i = 0; i < peons->capacity; i++) {
    peon *p = (peon *)peons->stack + i;
    if (!p->alive) {
      peons->count++;
      p->pos.x = (rand() % 12) - 6 + peons->spawner.x;
      p->pos.y = (rand() % 8) - 4 + peons->spawner.y;
      p->speed = ((rand() % 8) * 0.1f) + 0.1f;
      p->size.h = 5;
      p->size.w = 5;
      p->frame = 0;
      p->which = i;
      p->alive = true;
      p->tex = gs->default_peon;
      if (p->which % 22 == 0) {
        p->type = PEON_CHICKEN;
        p->size.h = 16;
        p->size.w = 16;
        p->tex = gs->chicken.tex;
      }
      p->target = peons->target;
      p->dir = normalize(p->target->pos - p->pos);
      if (p->dir.x < 0) {
        p->hmirror = true;
      }
      SDL_Log("new peon at %f,%f", p->pos.x, p->pos.y);
      break;
    }
    // if there's an empty slot, add it. otherwise oh well
  }
}

internal void tick_peons(peon_stack *peons) {
  for (int i = 0; i < peons->capacity; i++) {
    peon *p = (peon *)peons->stack + i;
    if (p->alive) {
      p->pos = p->pos + (p->dir * p->speed);
      if (p->dir.x < 0) {
        p->hmirror = true;
      }
      if (hit_target(p->target, p->pos)) {
        if (p->target->next != 0) {
          p->target = p->target->next;
        }
      }
      if (p->ticks % 10 == 0) {
        p->frame = (p->frame + 1) % 5;
      }
      if (p->ticks % 60 == 0) { // don't change our direction too quickly
        ASSERT(p->target != 0);
        p->dir = normalize(p->target->pos - p->pos);
      }
      // TODO kill peons..
      p->ticks++;
    }
  }
}

void render_peons(peon_stack *peons, drawitem_stack *drawitems) {
  for (int i = 0; i < peons->capacity; i++) {
    peon *p = (peon *)peons->stack + i;
    if (p->alive) {
      SDL_Rect src, dst;
      src.x = p->frame * 5;
      src.y = (p->which % 7) * 5;
      src.w = 5;
      src.h = 5;
      dst.x = int(p->pos.x);
      dst.y = int(p->pos.y);
      dst.w = 5;
      dst.h = 5;
      // TODO keep magnification per peon..
      if (p->type == PEON_CHICKEN) {
        src.x = p->frame * 16;
        src.y = 0;
        src.w = 16;
        src.h = 16;
        dst.w = 16;
        dst.h = 16;
      }
      ASSERT(p->tex != 0);
      if (p->dir.x < 0) {
        dst.w = -dst.w;
      }
      push_drawitem(drawitems, p->tex, &src, &dst);
    }
  }
}
