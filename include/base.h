#ifndef BASE_H
#define BASE_H

#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include <SDL2/SDL.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define bool32 int32_t

#define SCREEN_H 640
#define SCREEN_W 640
#define MS_PER_TICK 1000 / 60

typedef struct chicken_info {
  SDL_Texture *tex;
  SDL_Rect rect;
  int frame;
  int x, y;
} chicken;

typedef struct game_state {
  uint32_t taps;
  uint32_t ticks;
  uint32_t fps;
  SDL_Renderer *sdlRenderer;
  SDL_Window *sdlWindow;
  SDL_Texture *bg, *tower, *road;
  bool32 running;
  struct {
    SDL_Rect rect;
    bool32 captured;
    bool32 button1, button2;
  } mouse;
  int frames;
  uint32_t lastMs;
  uint32_t lastFPSstamp;
  struct {
    SDL_Texture *tex;
    SDL_Rect rect;
  } clouds;
  chicken_info chicken;
  struct {
    bool32 up, down, left, right, a, b, select, start;
  } pad;
  struct {
    SDL_Texture *font;
    SDL_Rect rect;
    bool32 open;
    struct {
      SDL_Rect rect;
    } tapbutton;
  } menu;
} game_state;

void emscripten_loop_workaround(void *gs);
void main_loop(game_state *gs);

bool32 load_texture_from_bitmap(SDL_Renderer *renderer, SDL_Texture **tex,
                                const char *name) {
  // it's just easier to get it converted to a texture this way, even though
  // there's the extra step;
  // todo i think we leak memory if sdl_freesurface doesn't call free() on
  // underlying pixel data..
  int x, y, n;
  unsigned char *data = stbi_load(name, &x, &y, &n, 4);
  if (!data) {
    printf("failed to load %s!\n", name);
    return false;
  }
  SDL_Surface *temp = SDL_CreateRGBSurfaceFrom(
      data, x, y, n * 8, 4 * x, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
  // SDL_Surface *temp = SDL_LoadBMP(name);
  if (temp == 0) {
    printf("failed to load %s!\n", name);
    printf("%s\n", SDL_GetError());
    return false;
  }
  // SDL_SetColorKey(temp, true, 0xFFFF00FF);
  *tex = SDL_CreateTextureFromSurface(renderer, temp);
  if (*tex == 0) {
    printf("failed to convert %s into texture!\n", name);
    SDL_FreeSurface(temp);
    return false;
  }
  SDL_FreeSurface(temp);
  return true;
}

bool32 die() {
#ifdef __EMSCRIPTEN__
  emscripten_cancel_main_loop();
#endif
  SDL_Quit();
  return false;
}

#endif
