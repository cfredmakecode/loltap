#ifndef BASE_H
#define BASE_H

#include "vec.cpp"
#include "stdio.h"

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

typedef struct target_node {
  target_node *prev, *next;
  v2 pos;
} target_node;

bool32 hit_target(target_node *target, v2 pos) {
  float dist = magnitude(target->pos - pos);
  return dist < 5.0f;
}

typedef struct chicken_info {
  SDL_Texture *tex;
  SDL_Rect rect;
  int frame;
  int x, y;
} chicken;

enum peon_type {
  PEON_NORMAL,
  PEON_CHICKEN,
  PEON_TYPE_COUNT
};

typedef struct peon {
  v2 pos, dir;
  float speed;
  struct {
    int w,h;
  } size;
  int frame;
  int which;
  uint32_t ticks;
  bool32 alive;
  peon_type type;
  target_node *target; // where to walk towards
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
  SDL_Texture *bg, *tower, *road, *default_peon, *isometric;
  bool32 running;
  peon *peons[1000];
  target_node *targets[3];
  int lastTargetIndex;
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
  uint32_t upgrades_unlocked;
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
// TODO obviously do the samn sorting at insertion time, complete DERP
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
  if (d->count == 0) {
    // stored as a linked list even though it's a raw n * size stack, we need to
    // access both ways to sort and draw in the desired order easily later
    drawitem *next = d->stack + d->count;
    next->dst = *dst;
    next->src = *src;
    next->tex = tex;
    next->next = 0;
    d->count++;
    return;
  }

  drawitem *cur = d->stack;
  drawitem *best = d->stack;
  while (true) {
    // follow links, compare y to each node
    // if cury < newitemy && cury > highesty_less_than_cury:
    // highesty_less_than_cury = cury
    // when we've gotten next = 0:
    // newitem.next = cur.next
    // cur.next = newitem
    // that's it? seems too easy
    if (cur == 0) {
      // end of the line for us
      drawitem *next = d->stack + d->count;
      next->dst = *dst;
      next->src = *src;
      next->tex = tex;
      next->next = best->next;
      best->next = next;
      d->count++;
      return;
    }
    // IMPORTANT(caf): we sort by y position + height because we left SDL's top-left origin coordinate system alone
    // rather than flipping y. might want to eventually flip y if we do anything much more complex than a tappy game
    // with autonomous simple peeps
    if (dst->y+dst->h > cur->dst.y+cur->dst.h) {
      best = cur;
    }
    cur = cur->next;
  }
}

void render_drawitemstack(drawitemstack *d, SDL_Renderer *renderer) {
  d->count = 0;
  drawitem *p = d->stack;
  while (p != 0) {
    SDL_Rect dst = p->dst;
    dst.y -= p->src.h;
    dst.x -= (p->src.w / 2);
    SDL_RenderCopy(renderer, p->tex, &p->src, &dst);
    p = p->next;
  }
}

struct {
  int multiplier, cost;
  const char *name, *icon_path;
} upgrades[] = {{2, 100, "double!", "double.png"},
                {4, 1000, "quadruple!", "quadruple.png"}};

#endif
