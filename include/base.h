#ifndef BASE_H
#define BASE_H

#include "vec.cpp"
#include "stdio.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include <SDL2/SDL.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <html5.h>
#endif

#define bool32 int32_t
#define f32 float
#define internal static
#define remembered_var static

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

typedef struct target_node {
  target_node *prev, *next;
  v2 pos;
} target_node;

bool32 hit_target(target_node *target, v2 pos) {
  f32 dist = magnitude(target->pos - pos);
  return fabs(dist) < 2.0f;
}

typedef struct chicken_info {
  SDL_Texture *tex;
  SDL_Rect rect;
  int frame;
  int x, y;
} chicken;

enum peon_type { PEON_NORMAL, PEON_CHICKEN, PEON_TYPE_COUNT };

typedef struct peon {
  v2 pos, dir;
  f32 speed;
  struct {
    int w, h;
  } size;
  int frame;
  int which;
  uint32_t ticks;
  bool32 alive;
  peon_type type;
  target_node *target; // where to walk towards
  SDL_Texture *tex;
  bool32 hmirror;
} peon;

typedef struct peon_stack {
  size_t count, capacity;
  peon *stack;
  v2 spawner;
  target_node
      *target; // some sort of default target to walk towards upon adding a peon
} peon_stack;

typedef struct drawitem {
  drawitem *next;
  SDL_Rect src, dst;
  SDL_Texture *tex;
  bool32 hmirror;
} drawitem;

typedef struct drawitem_stack {
  size_t count, capacity;
  drawitem *stack;
} drawitem_stack;

typedef struct game_state {
  struct {
    SDL_Rect rect;
    f32 zoom;
  } camera;
  uint32_t taps;
  uint32_t ticks;
  uint32_t fps;
  SDL_Renderer *sdlRenderer;
  SDL_Window *sdlWindow;
  SDL_Texture *bg, *tower, *road, *default_peon, *isometric, *grid;
  bool32 running;
  peon_stack peons;
  target_node *targets[3];
  int lastTargetIndex;
  drawitem_stack drawitems;
  drawitem_stack grounditems;
  struct {
    SDL_Rect rect;
    bool32 captured;
    bool32 on_tap_target;
    bool32 button1, button2, button3;
    uint32_t timestamp;
  } mouse;
  struct {
    bool32 down;
    v2 cur, rel;
  } touch;
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
  SDL_Cursor *handcursor;
  SDL_Cursor *arrowcursor;
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
// pass a negative width in the dst rect to flip horizontally
void push_drawitem(drawitem_stack *d, SDL_Texture *tex, SDL_Rect *src,
                   SDL_Rect *dst) {
  if (d->stack == 0) {
    d->stack = (drawitem *)malloc(10000 * sizeof(drawitem));
    d->capacity = 10000;
  }
  if (d->capacity == d->count) {
    SDL_Log(
        "too many items (10000) in drawitems stack! bailing to let you know. "
        "enjoy! :D");
    die();
    ASSERT(false);
    return;
  }
  if (d->count == 0) {
    // stored as a linked list even though it's a raw n * size stack, we need to
    // access both ways to sort and draw in the desired order easily later
    drawitem *next = d->stack + d->count;
    next->dst = *dst;
    if (src != 0) {
      next->src = *src;
    }
    next->tex = tex;
    next->hmirror = false;
    if (next->dst.w < 0) {
      next->hmirror = true;
      next->dst.w = -next->dst.w;
    }
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
      if (src != 0) {
        next->src = *src;
      }
      next->tex = tex;
      next->next = best->next;
      next->hmirror = false;
      if (next->dst.w < 0) {
        next->hmirror = true;
        next->dst.w = -next->dst.w;
      }
      best->next = next;

      d->count++;
      return;
    }
    // IMPORTANT(caf): we sort by y position + height because we left SDL's
    // top-left origin coordinate system alone
    // rather than flipping y. might want to eventually flip y if we do anything
    // much more complex than a tappy game
    // with autonomous simple peeps
    if (dst->y > cur->dst.y) {
      best = cur;
    }
    cur = cur->next;
  }
}

void render_drawitemstack(drawitem_stack *d, SDL_Renderer *renderer,
                          game_state *gs) {
  d->count = 0;
  drawitem *p = d->stack;
  while (p != 0) {
    SDL_Rect dst = p->dst;
    // for things we care about sorting in "z"...
    dst.y -= p->dst.h;       // bottom
    dst.x -= (p->dst.w / 2); // center
    dst.y += gs->camera.rect.y;
    dst.x += gs->camera.rect.x;
    // apply playfield zoom...
    // dst.h *= gs->camera.zoom;
    // dst.w *= gs->camera.zoom;
    // TODO(caf): pull out render copy into something which obeys scrolling,
    // camera zoom, etc automatically..
    if (p->hmirror) {
      SDL_RenderCopyEx(renderer, p->tex, &p->src, &dst, 0, 0,
                       SDL_FLIP_HORIZONTAL);
    } else {
      SDL_RenderCopy(renderer, p->tex, &p->src, &dst);
    }
    p = p->next;
  }
}

struct {
  int multiplier, cost;
  const char *name, *icon_path;
} upgrades[] = {{2, 100, "double!", "double.png"},
                {4, 1000, "quadruple!", "quadruple.png"}};

internal void move_camera_by(game_state *gs, f32 x, f32 y) {
  gs->camera.rect.x += x;
  gs->camera.rect.y += y;
  if (gs->camera.rect.x > MAX_CAMERA_X) {
    gs->camera.rect.x = MAX_CAMERA_X;
  }
  if (gs->camera.rect.y > MAX_CAMERA_Y) {
    gs->camera.rect.y = MAX_CAMERA_Y;
  }
  if (gs->camera.rect.x < MIN_CAMERA_X) {
    gs->camera.rect.x = MIN_CAMERA_X;
  }
  if (gs->camera.rect.y < MIN_CAMERA_Y) {
    gs->camera.rect.y = MIN_CAMERA_Y;
  }
}

internal void zoom_camera_by(game_state *gs, f32 amount) {
  gs->camera.zoom += amount;
  if (gs->camera.zoom < 0.1f) {
    gs->camera.zoom = 0.1f;
  }
  if (gs->camera.zoom > 2.0f) {
    gs->camera.zoom = 2.0f;
  }
}

// convert coordinates in screen space to game coords
inline internal v2 deletemecamera_to_playfield(game_state *gs, f32 x, f32 y) {
  v2 result;
  // result.x = (gs->camera.rect.x) * (1 / gs->camera.zoom) - x;
  // result.y = (gs->camera.rect.y) * (1 / gs->camera.zoom) - y;
  result.x = (gs->camera.rect.x) - x;
  result.y = (gs->camera.rect.y) - y;
  return result;
}

internal void reset_camera(game_state *gs) {
  gs->camera.rect.x = 0;
  gs->camera.rect.y = 0;
  gs->camera.zoom = 1.0f;
}

#endif
