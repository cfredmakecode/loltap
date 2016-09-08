#include "include/base.h"

#include "events.cpp"
#include "math.h"

#undef main // to shut up winders

extern "C" int main(int argc, char **argv) {
  game_state gs = {0};
  gs.running = 1;
  gs.lastMs = SDL_GetTicks();
  gs.lastFPSstamp = gs.lastMs;

#ifdef __EMSCRIPTEN__
  // IMPORTANT(caf): this set and unset loop is a weird workaround I found
  // to shut emscripten up about a main loop when using SDL2
  emscripten_set_element_css_size(0, 1280, 720);
  emscripten_set_main_loop_arg(&emscripten_loop_workaround, (void *)&gs, 0, 0);
#endif
  SDL_CreateWindowAndRenderer(1280, 720, SDL_WINDOW_RESIZABLE, &gs.sdlWindow,
                              &gs.sdlRenderer);
  SDL_MaximizeWindow(gs.sdlWindow);
  SDL_DisplayMode mode;
  SDL_GetWindowDisplayMode(gs.sdlWindow, &mode);
  gs.camera.rect.w = mode.w;
  gs.camera.rect.h = mode.h;
  // NOTE(caf): center the camera way away from negative coords and we avoid
  // handling some weirdness
  // we won't need to scroll this far ever
  gs.camera.rect.x = 0;
  gs.camera.rect.y = 0;
  gs.camera.zoom = 1.0f;

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");

  for (int i = 0; i < ARRAY_COUNT(pts.points); i++) {
    pts.points[i].x = i * 100;
    pts.points[i].y = i * 100;
  }

#ifdef __EMSCRIPTEN__
  emscripten_cancel_main_loop();
#endif

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop_arg(&emscripten_loop_workaround, (void *)&gs, 0, 1);
#else
  while (gs.running) {
    main_loop(&gs);
  }
#endif
  SDL_Quit();

  return 0;
}

void main_loop(game_state *gs) {
  while (1) {
    uint32_t toWait = SDL_GetTicks() - gs->lastMs;
    if (toWait >= MS_PER_TICK) {
      break;
    }
    if (toWait <= 1) {
      continue;
    }
    SDL_Delay(1);
    // todo sleep a bit less silly-ly, handle shorter periods correctly
    // TODO fix tickrate to tick multiple times till caught up, currently
    // ticks
    // are based on FPS, just capped at 60
  }
  gs->ticks++;

  /// todo handle having to step multiple ticks because fps got behind
  handle_events(gs);

  SDL_SetRenderDrawColor(gs->sdlRenderer, 0x49, 0x60, 0x0, 0xff);
  SDL_RenderClear(gs->sdlRenderer);
  SDL_SetRenderDrawColor(gs->sdlRenderer, 20, 20, 40, 240);
  SDL_RenderFillRect(gs->sdlRenderer, &gs->mouse.rect);

  SDL_RenderSetScale(gs->sdlRenderer, gs->camera.zoom, gs->camera.zoom);
  for (int i = 0; i < ARRAY_COUNT(pts.points); i++) {
    SDL_Rect rect;
    rect.w = 10;
    rect.h = 10;
    rect.x = pts.points[i].x;
    rect.y = pts.points[i].y;
    SDL_SetRenderDrawColor(gs->sdlRenderer, rect.x + gs->ticks % 255,
                           rect.y + gs->ticks % 255, rect.x % 255, 255);
    render_rect(gs, rect.x, rect.y, rect.w, rect.h);
    continue;
    rect.x = pts.points[i].x - gs->camera.rect.x;
    rect.y = gs->camera.rect.h - pts.points[i].y + (rect.h) - gs->camera.rect.y;
    SDL_RenderFillRect(gs->sdlRenderer, &rect);
  }
  SDL_RenderSetScale(gs->sdlRenderer, 1.0f, 1.0f);

  //
  // grid
  SDL_SetRenderDrawColor(gs->sdlRenderer, 0, 0, 0, 100);
  for (int y = 0; y < gs->camera.rect.h; y += 10) {
    SDL_RenderDrawLine(gs->sdlRenderer, 0, y, gs->camera.rect.w, y);
  }
  for (int x = 0; x < gs->camera.rect.w; x += 10) {
    SDL_RenderDrawLine(gs->sdlRenderer, x, 0, x, gs->camera.rect.h);
  }
  SDL_SetRenderDrawColor(gs->sdlRenderer, 0, 70, 35, 180);
  for (int y = 0; y < gs->camera.rect.h; y += 100) {
    SDL_RenderDrawLine(gs->sdlRenderer, 0, y, gs->camera.rect.w, y);
  }
  for (int x = 0; x < gs->camera.rect.w; x += 100) {
    SDL_RenderDrawLine(gs->sdlRenderer, x, 0, x, gs->camera.rect.h);
  }
  //
  //

  //
  // points
  SDL_SetRenderDrawColor(gs->sdlRenderer, 0xff, 0, 0, 0xff);
  SDL_Rect r;
  r.w = 3;
  r.h = 3;
  r.x = -1;
  r.y = -1;
  SDL_RenderFillRect(gs->sdlRenderer, &r);
  r.x = -1;
  r.y = gs->camera.rect.h - 2;
  SDL_RenderFillRect(gs->sdlRenderer, &r);
  r.x = gs->camera.rect.w - 2;
  r.y = -1;
  SDL_RenderFillRect(gs->sdlRenderer, &r);
  r.x = gs->camera.rect.w - 2;
  r.y = gs->camera.rect.h - 2;
  SDL_RenderFillRect(gs->sdlRenderer, &r);
  r.x = (gs->camera.rect.w / 2) - 1;
  r.y = (gs->camera.rect.h / 2) - 1;
  SDL_RenderFillRect(gs->sdlRenderer, &r);
  SDL_SetRenderDrawColor(gs->sdlRenderer, 0xff, 0xff, 0xff, 0xff);
  r.w = 1;
  r.h = 1;
  r.x = 0;
  r.y = 0;
  SDL_RenderDrawPoint(gs->sdlRenderer, r.x, r.y);
  r.x = 0;
  r.y = gs->camera.rect.h - 1;
  SDL_RenderDrawPoint(gs->sdlRenderer, r.x, r.y);
  r.x = gs->camera.rect.w - 1;
  r.y = 0;
  SDL_RenderDrawPoint(gs->sdlRenderer, r.x, r.y);
  r.x = gs->camera.rect.w - 1;
  r.y = gs->camera.rect.h - 1;
  SDL_RenderDrawPoint(gs->sdlRenderer, r.x, r.y);
  r.x = (gs->camera.rect.w / 2);
  r.y = (gs->camera.rect.h / 2);
  SDL_RenderDrawPoint(gs->sdlRenderer, r.x, r.y);
  //
  //

  SDL_RenderSetScale(gs->sdlRenderer, gs->camera.zoom, gs->camera.zoom);
  render_rect(gs, 100, 100, 20, 20);
  render_rect(gs, 200, 200, 100, 100);
  SDL_RenderSetScale(gs->sdlRenderer, 1.0f, 1.0f);

  SDL_RenderPresent(gs->sdlRenderer);

  gs->frames++;
  if (SDL_GetTicks() - gs->lastFPSstamp > 1000) {
    gs->fps = gs->frames;
    SDL_Log("%d fps", gs->fps);
    gs->frames = 0;
    gs->lastFPSstamp = SDL_GetTicks();
  }
  gs->lastMs = SDL_GetTicks();
}

void emscripten_loop_workaround(void *gs) { main_loop((game_state *)gs); }
