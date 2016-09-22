#ifndef BASE_H
#define BASE_H

#include "vec.cpp"
#include "stdio.h"

#include <SDL2/SDL.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <html5.h>
#endif

#define bool32 int32_t
#define f32 float
#define internal static
#define remembered_var static

#define MS_PER_TICK 1000 / 60

#define HELD_DOWN_MIN_TICKS 300 // delay before awarding auto taps/clicks

#define ARRAY_COUNT(thing) sizeof(thing) / sizeof(thing[0])
#define ASSERT(thing)                                                          \
  \
if(!(thing)) {                                                                 \
    SDL_ShowSimpleMessageBox(0, "assertion failed!", 0, 0);                    \
    char *blowup = 0;                                                          \
    *blowup = 'Y';                                                             \
  \
}

typedef struct camera_info {
  v2 pos;
  f32 screenw, screenh;
  f32 scale;
} camera_info;

typedef struct game_state {
  camera_info camera;
  uint32_t ticks;
  uint32_t fps;
  SDL_Renderer *sdlRenderer;
  SDL_Window *sdlWindow;
  bool32 running;
  struct {
    SDL_Rect rect;
    bool32 button1;
  } mouse;
  int frames;
  uint32_t lastMs;
  uint32_t lastFPSstamp;
} game_state;

void emscripten_loop_workaround(void *gs);
void main_loop(game_state *gs);

bool32 die() {
#ifdef __EMSCRIPTEN__
  emscripten_cancel_main_loop();
#endif
  SDL_Quit();
  return false;
}

internal v2 s2w(camera_info *c, f32 x, f32 y) {
  v2 pos;
  pos.x = x * 1 / c->scale;
  pos.y = y * 1 / c->scale;
  pos.x -= c->pos.x * 1 / c->scale;
  pos.y -= c->pos.y * 1 / c->scale;
  return pos;
}
internal v2 w2s(camera_info *c, f32 x, f32 y) {
  v2 pos;
  pos.x = x * c->scale;
  pos.y = y * c->scale;
  pos.x += c->pos.x;
  pos.y += c->pos.y;
  return pos;
}

internal v2 camera_center2w(camera_info *c) {
  return s2w(c, c->screenw / 2, c->screenh / 2);
}

// internal v2 s2w(camera_info *c, f32 x, f32 y) {
//   v2 result;
//   v2 pos = {x, y};
//   v2 view;
//   v2 viewcenter;
//   v2 screencenter;
//
//   screencenter.x = (c->screenw / 2.0f);
//   screencenter.y = (c->screenh / 2.0f);
//   v2 sray = screencenter - pos;
//
//   viewcenter.x = (c->rect.w / 2.0f);
//   viewcenter.y = (c->rect.h / 2.0f);
//   v2 vrelativepos;
//   vrelativepos = pos / screencenter;
//   view = viewcenter + (sray * vrelativepos);
//
//   result.x = view.x + c->rect.x;
//   result.y = view.y + c->rect.y;
//   return result;
// }
//
// internal v2 w2s(camera_info *c, f32 x, f32 y) {
//   v2 result;
//   v2 pos = {x, y};
//   v2 view;
//   v2 viewcenter;
//   v2 screencenter;
//
//   screencenter.x = (c->screenw / 2.0f);
//   screencenter.y = (c->screenh / 2.0f);
//
//   viewcenter.x = (c->rect.w / 2.0f);
//   viewcenter.y = (c->rect.h / 2.0f);
//
//   view.x = pos.x - c->rect.x; // - viewcenter.x;
//   view.y = pos.y - c->rect.y; // - viewcenter.y;
//   v2 vray = viewcenter - view;
//
//   f32 srelativepos;
//   srelativepos = view.x / viewcenter.x;
//
//   result.x = c->rect.x + screencenter.x * srelativepos;
//   result.y = c->rect.y + screencenter.y * srelativepos;
//   return result;
// }
#endif
