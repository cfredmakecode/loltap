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

#define SCREEN_H 512
#define SCREEN_W 512
#define LOGICAL_HEIGHT 128
#define LOGICAL_WIDTH 128
#define MS_PER_TICK 1000 / 60
#define ARRAY_COUNT(thing) sizeof(thing) / sizeof(thing[0])

typedef struct chicken_info {
  SDL_Texture *tex;
  SDL_Rect rect;
  int frame;
  int x, y;
} chicken;

typedef struct peon {
  SDL_Rect pos;
  int frame;
  int which;
  int speed;
  uint32_t ticks;
  bool32 alive;
} peon;

typedef struct drawitem {
  drawitem *next;
  SDL_Rect src, dst;
  SDL_Texture *tex;
} drawitem;

typedef struct drawitemstack {
  size_t count, capacity;
  drawitem *stack;
} drawitemstack;

typedef struct game_state {
  uint32_t taps;
  uint32_t ticks;
  uint32_t fps;
  SDL_Renderer *sdlRenderer;
  SDL_Window *sdlWindow;
  SDL_Texture *bg, *tower, *road, *default_peon;
  bool32 running;
  peon *peons[100];
  drawitemstack drawitems;
  struct {
    SDL_Rect rect;
    bool32 captured;
    bool32 on_tap_target;
    bool32 button1, button2;
    uint32_t timestamp;
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

// prepare to draw an item later. will be sorted by z before actually blitting
// to screen
// naive and slow
void push_drawitem(drawitemstack *d, SDL_Texture *tex, SDL_Rect *src,
                   SDL_Rect *dst) {
  if (d->stack == 0) {
    d->stack = (drawitem *)malloc(1000 * sizeof(drawitem));
    d->capacity = 1000;
  }
  if (d->capacity == d->count) {
    SDL_Log("too many items in drawitems stack! bailing to let you know. "
            "enjoy! :D");
    die();
  }
  // stored as a linked list even though it's a raw n * size stack, we need to
  // access both ways to sort and draw in the desired order easily later
  drawitem *prev = d->stack + d->count;
  d->count++;
  drawitem *next = d->stack + d->count;
  prev->next = next;
  next->dst = *dst;
  next->src = *src;
  next->tex = tex;
  next->next = 0;
}

void sort_drawitems(drawitem *d) {
  // rewrite the next pointers in place based on y position
}

void render_drawitemstack(drawitemstack *d, SDL_Renderer *renderer) {
  // no-op
  d->count = 0;
  drawitem *p = d->stack;
  sort_drawitemstack(d);
  while (p != 0) {
    SDL_RenderCopy(renderer, p->tex, &p->src, &p->dst);
    p = p->next;
  }
}

#endif
