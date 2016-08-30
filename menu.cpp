#include "include/base.h"
void tick_menu(game_state *gs);

// todo return the rect the text was rendered into (with padding?)
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
    } else if (*l == '.') {
      offset = 26;
    } else if (*l == '?') {
      offset = 27;
    } else if (*l == '!') {
      offset = 28;
    } else if (*l == ':') {
      offset = 29;
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
  gs->menu.tapbutton.rect.y = 100;
  gs->menu.tapbutton.rect.x = 0;
  gs->menu.tapbutton.rect.w = 128;
  gs->menu.tapbutton.rect.h = 28;
  tick_menu(gs);
  return true;
}

void tick_menu(game_state *gs) {
  SDL_Rect tower, chicken;
  tower.h = 64;
  tower.w = 16;
  tower.x = 16;
  tower.y = 16;
  chicken.h = 16;
  chicken.w = 16;
  chicken.x = gs->chicken.x;
  chicken.y = gs->chicken.y;
  if (gs->mouse.button1) {
    if (SDL_HasIntersection(&tower, &gs->mouse.rect) ||
        SDL_HasIntersection(&chicken, &gs->mouse.rect)) {
      gs->menu.open = true;
      gs->mouse.button1 = false;
      gs->menu.rect.x = gs->mouse.rect.x;
      gs->menu.rect.y = gs->mouse.rect.y;
    } else {
      gs->menu.open = false;
      gs->mouse.button1 = false;
    }
  }
  gs->menu.rect.w = 96;
  gs->menu.rect.h = 96;
}

#define LINE_HEIGHT_IN_PX 18

void render_menu(game_state *gs) {
  SDL_SetRenderDrawBlendMode(gs->sdlRenderer, SDL_BLENDMODE_BLEND);
  if (SDL_HasIntersection(&gs->menu.tapbutton.rect, &gs->mouse.rect)) {
    SDL_SetRenderDrawColor(gs->sdlRenderer, 0xff, 0xff, 0xff, 180);
    SDL_RenderDrawRect(gs->sdlRenderer, &gs->menu.tapbutton.rect);
  }
  SDL_SetRenderDrawColor(gs->sdlRenderer, 0x00, 0x00, 0xff, 64);
  SDL_RenderFillRect(gs->sdlRenderer, &gs->menu.tapbutton.rect);
  if (gs->menu.open) {
    SDL_SetRenderDrawBlendMode(gs->sdlRenderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(gs->sdlRenderer, 0xff, 0xff, 0xff, 180);
    SDL_RenderFillRect(gs->sdlRenderer, &gs->menu.rect);
    SDL_SetRenderDrawColor(gs->sdlRenderer, 0x00, 0x00, 0xff, 64);
    SDL_RenderDrawRect(gs->sdlRenderer, &gs->menu.rect);

    int yoffset = 2;
    char buf[16];
    sprintf(buf, "%u fps", gs->fps);
    render_text(gs, buf, gs->menu.rect.x + 2, gs->menu.rect.y + yoffset);
    yoffset += LINE_HEIGHT_IN_PX;

    SDL_SetRenderDrawColor(gs->sdlRenderer, 0x00, 0x00, 0x00, 64);
    SDL_Rect zebra = gs->menu.rect;
    zebra.h = LINE_HEIGHT_IN_PX + 2;
    zebra.y = gs->menu.rect.y + yoffset - 2;
    SDL_RenderFillRect(gs->sdlRenderer, &zebra);
    render_text(gs, "item 2", gs->menu.rect.x + 2, gs->menu.rect.y + yoffset);

    yoffset += LINE_HEIGHT_IN_PX;
    render_text(gs, "menu !", gs->menu.rect.x + 2, gs->menu.rect.y + yoffset);

    yoffset += LINE_HEIGHT_IN_PX;
    zebra.y = gs->menu.rect.y + yoffset - 2;
    SDL_RenderFillRect(gs->sdlRenderer, &zebra);
    render_text(gs, "ya!?:.", gs->menu.rect.x + 2, gs->menu.rect.y + yoffset);

    yoffset += LINE_HEIGHT_IN_PX;
    render_text(gs, "baka", gs->menu.rect.x + 2, gs->menu.rect.y + yoffset);

    // todo(caf): get back rect from rendering text each time so we can resize
    // things we need to
  }
  char buf[16];
  sprintf(buf, "%u taps", gs->taps);
  render_text(gs, buf, 2, 2);
}
