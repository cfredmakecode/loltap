#include "include/base.h"

bool32 init_chicken(game_state *gs) {
  if (!load_texture_from_bitmap(gs->sdlRenderer, &gs->chicken.tex,
                                "chicken_chicken.png")) {
    SDL_Quit();
    return false;
  }
  printf("loaded chicken!\n");
  gs->chicken.frame = 0;
  gs->chicken.rect.x = 0;
  gs->chicken.rect.y = 0;
  gs->chicken.rect.w = 16;
  gs->chicken.rect.h = 16;
  gs->chicken.x = 50;
  gs->chicken.y = 50;
  return true;
}

void tick_chicken(game_state *gs) {
  if (gs->ticks % 10 == 0) {
    gs->chicken.frame = (gs->chicken.frame + 1) % 6;
  }
  if (gs->mouse.captured) {
    if (gs->pad.up) {
      gs->chicken.y--;
    }
    if (gs->pad.down) {
      gs->chicken.y++;
    }
    if (gs->pad.left) {
      gs->chicken.x--;
    }
    if (gs->pad.right) {
      gs->chicken.x++;
    }
  }
}

void render_chicken(game_state *gs) {
  SDL_Rect r;
  r.x = gs->chicken.x;
  r.y = gs->chicken.y;
  r.w = 16;
  r.h = 16;

  gs->chicken.rect.x = 16 * gs->chicken.frame;
  gs->chicken.rect.y = 0;

  SDL_RenderCopy(gs->sdlRenderer, gs->chicken.tex, &gs->chicken.rect, &r);
}
