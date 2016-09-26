#include "include/base.h"

#include "events.cpp"
#include "math.h"

SDL_Texture *huge_image;

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
  SDL_CreateWindowAndRenderer(1280, 720,
                              SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED,
                              &gs.sdlWindow, &gs.sdlRenderer);
  SDL_MaximizeWindow(gs.sdlWindow);
  SDL_DisplayMode mode;
  SDL_GetWindowDisplayMode(gs.sdlWindow, &mode);
  int w, h;
  SDL_GetWindowSize(gs.sdlWindow, &w, &h);
  gs.camera.screenw = w;
  gs.camera.screenh = h;
  gs.camera.scale = 1.0f;

  SDL_Surface *s = SDL_LoadBMP("assets/huge_image.bmp");
  if (s == 0) {
    SDL_ShowSimpleMessageBox(0, "couldn't load huge_image.bmp!",
                             "couldn't load huge_image.bmp!", gs.sdlWindow);
    die();
    return 2;
  }
  huge_image = SDL_CreateTextureFromSurface(gs.sdlRenderer, s);
  if (huge_image == 0) {
    SDL_ShowSimpleMessageBox(0, "couldn't convert huge_image.bmp!",
                             "couldn't convert huge_image.bmp!", gs.sdlWindow);
    die();
    return 2;
  }
  SDL_FreeSurface(s);

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");

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
  camera_step(&gs->camera);

  SDL_RenderSetScale(gs->sdlRenderer, 1.0, 1.0);
  SDL_SetRenderDrawColor(gs->sdlRenderer, 0x00, 0x00, 0x00, 0xff);
  SDL_RenderClear(gs->sdlRenderer);
  SDL_SetRenderDrawBlendMode(gs->sdlRenderer, SDL_BLENDMODE_BLEND);

  SDL_Rect r = {0, 0, 3761, 4833};
  // always scale everything first always.
  v2 t = w2s(&gs->camera, r.x, r.y);
  r.x = t.x;
  r.y = t.y;

  r.w *= gs->camera.scale;
  r.h *= gs->camera.scale;
  render_image(gs, huge_image, 0, 0, 3761, 4833, 0, 0, 3761, 4833, 3761 / 2.0f,
               4833 / 2.0f);
  // SDL_RenderCopy(gs->sdlRenderer, huge_image, 0, &r);

  r.w = 100;
  r.h = 100;
  t = s2w(&gs->camera, gs->mouse.rect.x, gs->mouse.rect.y);
  r.x = t.x;
  r.y = t.y;
  t = w2s(&gs->camera, r.x, r.y);
  r.x = t.x - (r.w / 2);
  r.y = t.y - (r.h / 2);
  SDL_SetRenderDrawColor(gs->sdlRenderer, 0xff, 0x00, 0x00, 0x44);
  // SDL_RenderFillRect(gs->sdlRenderer, &r);

  r.w = 10;
  r.h = 10;
  t = s2w(&gs->camera, gs->mouse.rect.x, gs->mouse.rect.y);
  r.x = t.x;
  r.y = t.y;
  t = w2s(&gs->camera, r.x, r.y);
  r.x = t.x - (r.w / 2);
  r.y = t.y - (r.h / 2);
  SDL_SetRenderDrawColor(gs->sdlRenderer, 0xff, 0x00, 0x00, 0xff);
  // SDL_RenderDrawRect(gs->sdlRenderer, &r);

  t = s2w(&gs->camera, gs->mouse.rect.x, gs->mouse.rect.y);
  render_rect(gs, t.x, t.y, 10, 10, 5, 5, 0xff, uint8_t(t.x), 0, 0xff);

  SDL_RenderPresent(gs->sdlRenderer);

  gs->frames++;
  if (SDL_GetTicks() - gs->lastFPSstamp > 1000) {
    gs->fps = gs->frames;
    // SDL_Log("%d fps", gs->fps);
    gs->frames = 0;
    gs->lastFPSstamp = SDL_GetTicks();
  }
  gs->lastMs = SDL_GetTicks();
}

void emscripten_loop_workaround(void *gs) { main_loop((game_state *)gs); }
