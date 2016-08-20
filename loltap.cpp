#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include <SDL2/SDL.h>
#include <stdio.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define bool32 int32_t

typedef struct game_state {
  uint32_t ticks;
  SDL_Renderer *sdlRenderer;
  SDL_Window *sdlWindow;
  SDL_Texture *bg;
  bool32 running;
  struct {
    int x, y;
  } mouse;
  int frames;
  uint32_t lastMs;
  uint32_t lastFPSstamp;
  struct {
    SDL_Texture *tex;
    SDL_Rect *rect;
  } clouds;
} game_state;

void emscripten_loop_workaround(void *gs);
void main_loop(game_state *gs);

#define SCREEN_H 640
#define SCREEN_W 640
#define MS_PER_TICK 1000 / 60

void handle_events(game_state *gs) {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    switch (e.type) {

    case SDL_MOUSEMOTION:
      gs->mouse.x = e.motion.x;
      gs->mouse.y = e.motion.y;
      break;
    case SDL_QUIT:
      printf("quit requested\n");
      printf("quit requested\n");
      gs->running = false;
#ifdef __EMSCRIPTEN__
      emscripten_cancel_main_loop();
#endif
      break;
    case SDL_KEYDOWN:
    case SDL_KEYUP:
      if (e.key.keysym.sym == 'q') {
        printf("quit requested\n");
        gs->running = false;
#ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop();
#endif
      }
      printf("key: %c\n", e.key.keysym.sym);
      break;
    default:
      printf("event %u\n", e.type);
      break;
    }
  }
}

#undef main // to shut up winders

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

bool32 load_bitmaps(game_state *gs) {
  if (!load_texture_from_bitmap(gs->sdlRenderer, &gs->bg, "bg.bmp")) {
    SDL_Quit();
    return false;
  }
  printf("loaded bg!\n");
  if (!load_texture_from_bitmap(gs->sdlRenderer, &gs->clouds.tex,
                                "clouds.png")) {
    SDL_Quit();
    return false;
  }
  printf("loaded clouds!\n");
  return true;
}

extern "C" int main(int argc, char **argv) {
  game_state gs = {0};
  gs.running = 1;
  gs.lastMs = SDL_GetTicks();
  gs.lastFPSstamp = gs.lastMs;
  printf("hi\n");

#ifdef __EMSCRIPTEN__
  // EM_ASM(document.getElementById('canvas').style.width = '512px');
  // EM_ASM(document.getElementById('canvas').style.height = '512px');
  // IMPORTANT(caf): this set and unset loop is a weird workaround I found to
  // shut emscripten up
  // about a main loop when using SDL2
  emscripten_set_main_loop_arg(&emscripten_loop_workaround, (void *)&gs, 0, 0);
#endif

  SDL_CreateWindowAndRenderer(SCREEN_W, SCREEN_H, 0, &gs.sdlWindow,
                              &gs.sdlRenderer);
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
  SDL_RenderSetLogicalSize(gs.sdlRenderer, 32, 32);
#ifdef __EMSCRIPTEN__
  emscripten_cancel_main_loop();
#endif
  printf("set 32x32 rendering res\n");
  if (!load_bitmaps(&gs)) {
    printf("bailing!\n");
    return 1;
  }

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
    SDL_Delay(1); // todo sleep a bit less silly-ly
  }
  gs->ticks++;
  /// todo handle having to step multiple ticks because fps got behind
  handle_events(gs);
  SDL_SetRenderDrawColor(gs->sdlRenderer, 0, 0, 0, 255);
  SDL_RenderClear(gs->sdlRenderer);
  SDL_RenderCopy(gs->sdlRenderer, gs->bg, 0, 0);
  SDL_RenderCopy(gs->sdlRenderer, gs->clouds.tex, 0, 0);

  SDL_RenderPresent(gs->sdlRenderer);
  gs->frames++;
  if (SDL_GetTicks() - gs->lastFPSstamp > 1000) {
    printf("%d fps\n", gs->frames);
    gs->frames = 0;
    gs->lastFPSstamp = SDL_GetTicks();
  }
  gs->lastMs = SDL_GetTicks();
}

void emscripten_loop_workaround(void *gs) { main_loop((game_state *)gs); }
