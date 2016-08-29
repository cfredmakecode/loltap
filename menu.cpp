#include "include/base.h"
void tick_menu(game_state *gs);

void render_text(game_state *gs, const char *text, int x, int y) {
  const char *l = text;
  int offset = 0;
  while (l != 0 && *l != 0) {
    if (*l >= 'A' && *l <= 'Z') {
      // which tile has the letter we need?
      offset = (int)*l - 'A';
    } else if (*l >= 'a' && *l <= 'z') {
      offset = (int)*l - 'a';
    } else if (*l >= '0' && *l <= '9') {
      offset = (int)*l - '0';
      offset += 30;
    } else {
      x += 12;
      l++;
      continue;
    }
    SDL_Rect letter = {(offset % 10) * 5, (offset / 10) * 8, 5, 8};
    SDL_Rect pos = {x, y, 10, 16};
    SDL_RenderCopy(gs->sdlRenderer, gs->menu.font, &letter, &pos);
    x += 12;
    l++;
  }
}

bool32 init_menu(game_state *gs) {
  // text!
  gs->menu.item = "text";
  tick_menu(gs);
  return true;
}

void tick_menu(game_state *gs) {
  if (gs->mouse.button2) {
    gs->menu.open = !gs->menu.open;
  }
  gs->menu.rect.x = gs->mouse.x;
  gs->menu.rect.y = gs->mouse.y;
  gs->menu.rect.w = 32;
  gs->menu.rect.h = 64;
}

void render_menu(game_state *gs) {
  if (gs->menu.open) {
    SDL_SetRenderDrawColor(gs->sdlRenderer, 0xff, 0xff, 0xff, 180);
    SDL_RenderDrawRect(gs->sdlRenderer, &gs->menu.rect);
    SDL_RenderFillRect(gs->sdlRenderer, &gs->menu.rect);
  }
  render_text(gs, gs->menu.item, 4, 4);
}
