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
  v2 pos, target;
  f32 screenw, screenh;
  f32 scale, targetscale;
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

internal void camera_zoom(camera_info *c, f32 x, f32 y, f32 amount) {
  camera_zoom_to(c, x, y, amount - c->scale);
}

internal void camera_zoom_to(camera_info *c, f32 x, f32 y, f32 zoom) {
  c->targetscale = zoom;
  if (c->targetscale < 0.1f) {
    c->targetscale = 0.1f;
  }
  if (c->targetscale > 8.0f) {
    c->targetscale = 8.0f;
  }
  c->target.x = x;
  c->target.y = y;
}

internal void camera_step(camera_info *c) {
  v2 diff = c->target - c->pos;
  c->pos.x += diff.x / 10.0f;
  c->pos.y += diff.y / 10.0f;
}

internal void render_rect(game_state *gs, f32 x, f32 y, int w, int h, f32 refx,
                          f32 refy, uint8_t r, uint8_t g, uint8_t b,
                          uint8_t a) {
  SDL_Rect rect;
  v2 t = w2s(&gs->camera, x, y);
  rect.x = t.x;
  rect.y = t.y;
  rect.w = w * gs->camera.scale;
  rect.h = h * gs->camera.scale;
  rect.x -= (refx * gs->camera.scale);
  rect.y -= (refy * gs->camera.scale);
  // SDL_Log("%d, %d  %d, %d", rect.x, rect.y, rect.w, rect.h);
  SDL_SetRenderDrawColor(gs->sdlRenderer, r, g, b, a);
  SDL_RenderFillRect(gs->sdlRenderer, &rect);
}

internal void render_image(game_state *gs, SDL_Texture *tex, int srcx, int srcy,
                           int srcw, int srch, f32 dstx, f32 dsty, int dstw,
                           int dsth, f32 refx, f32 refy) {
  SDL_Rect srcrect, dstrect;
  srcrect.x = srcx;
  srcrect.y = srcy;
  srcrect.w = srcw;
  srcrect.h = srch;

  v2 t = w2s(&gs->camera, dstx, dsty);
  dstrect.x = t.x;
  dstrect.y = t.y;
  dstrect.w = dstw * gs->camera.scale;
  dstrect.h = dsth * gs->camera.scale;
  dstrect.x -= (refx * gs->camera.scale);
  dstrect.y -= (refy * gs->camera.scale);
  SDL_RenderCopy(gs->sdlRenderer, tex, &srcrect, &dstrect);
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
