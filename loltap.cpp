#include <SDL/SDL.h>
#include <stdio.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define bool32 int32_t

typedef struct game_state {
  uint32_t ticks;
  SDL_Surface *screen;
  bool32 running;
  struct {
    int x, y;
  } mouse;
} game_state;

void emscripten_loop_workaround(void *gs);
void main_loop(game_state *gs);

#define SCREEN_SIZE 256

void handle_events(game_state *gs) {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    switch (e.type) {
    case SDL_MOUSEMOTION:
      gs->mouse.x = e.motion.x;
      gs->mouse.y = e.motion.y;
      break;
    default:
      printf("unknown even %u", e.type);
      break;
    }
  }
}

extern "C" int main(int argc, char **argv) {
  game_state gs = {0};
  gs.running = 1;

  SDL_Init(SDL_INIT_EVERYTHING);
  gs.screen = SDL_SetVideoMode(SCREEN_SIZE, SCREEN_SIZE, 32, SDL_SWSURFACE);
#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop_arg(&emscripten_loop_workaround, (void *)&gs, 0, 1);
#else
  main_loop(&gs);
#endif
  SDL_Quit();

  return 0;
}

void main_loop(game_state *gs) {
  handle_events(gs);
  if (gs->ticks % 2 == 0) {
    if (SDL_MUSTLOCK(gs->screen))
      SDL_LockSurface(gs->screen);
    for (int i = 0; i < SCREEN_SIZE; i++) {
      for (int j = 0; j < SCREEN_SIZE; j++) {
        int32_t color =
            SDL_MapRGBA(gs->screen->format, i + gs->ticks + gs->mouse.y,
                        j + gs->ticks + gs->mouse.x, 255 - gs->ticks, 255);
        *((Uint32 *)gs->screen->pixels +
          ((SCREEN_SIZE * SCREEN_SIZE) - i * SCREEN_SIZE) + j) = color;
      }
    }
    if (SDL_MUSTLOCK(gs->screen))
      SDL_UnlockSurface(gs->screen);
    SDL_Flip(gs->screen);
  }
  gs->ticks++;
}

void emscripten_loop_workaround(void *gs) { main_loop((game_state *)gs); }
