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

f32 tempZoom = 1.0f;

#define MAX_CAMERA_X 100
#define MAX_CAMERA_Y 100
#define MIN_CAMERA_X -100
#define MIN_CAMERA_Y -100
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

typedef struct game_state {
  struct {
    SDL_Rect rect;
    f32 zoom;
  } camera;
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

void render_rect(game_state *gs, int x, int y, int w, int h) {
  SDL_Rect r;
  r.w = w * (gs->camera.zoom);
  r.h = h * (gs->camera.zoom);
  r.x = (x - gs->camera.rect.x);
  r.y = ((gs->camera.rect.h - 1) - r.h - gs->camera.rect.y - y);
  SDL_RenderFillRect(gs->sdlRenderer, &r);
}

static inline v2 screen_coords_to_playfield(game_state *gs, f32 x, f32 y) {
  v2 result;
  result.x = gs->camera.rect.x + x * (1 / gs->camera.zoom);
  result.y = (gs->camera.rect.h - 1) - (y * (1 / gs->camera.zoom)) -
             (gs->camera.rect.y);
  return result;
}

typedef struct blah { v2 points[10]; } blah;

blah pts = {0};
#endif
