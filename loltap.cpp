#include "include/base.h"

#include "chicken.cpp"

void handle_events(game_state *gs) {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    switch (e.type) {
    case SDL_WINDOWEVENT: {
      SDL_Event *event = &e;
      switch (event->window.event) {
      case SDL_WINDOWEVENT_SHOWN:
        SDL_Log("Window %d shown", event->window.windowID);
        break;
      case SDL_WINDOWEVENT_HIDDEN:
        SDL_Log("Window %d hidden", event->window.windowID);
        break;
      case SDL_WINDOWEVENT_EXPOSED:
        SDL_Log("Window %d exposed", event->window.windowID);
        break;
      case SDL_WINDOWEVENT_MOVED:
        SDL_Log("Window %d moved to %d,%d", event->window.windowID,
                event->window.data1, event->window.data2);
        break;
      case SDL_WINDOWEVENT_RESIZED:
        SDL_Log("Window %d resized to %dx%d", event->window.windowID,
                event->window.data1, event->window.data2);
        break;
      case SDL_WINDOWEVENT_SIZE_CHANGED:
        SDL_Log("Window %d size changed to %dx%d", event->window.windowID,
                event->window.data1, event->window.data2);
        break;
      case SDL_WINDOWEVENT_MINIMIZED:
        SDL_Log("Window %d minimized", event->window.windowID);
        break;
      case SDL_WINDOWEVENT_MAXIMIZED:
        SDL_Log("Window %d maximized", event->window.windowID);
        break;
      case SDL_WINDOWEVENT_RESTORED:
        SDL_Log("Window %d restored", event->window.windowID);
        break;
      case SDL_WINDOWEVENT_ENTER:
        SDL_Log("Mouse entered window %d", event->window.windowID);
        gs->mouse.captured = true;
        break;
      case SDL_WINDOWEVENT_LEAVE:
        SDL_Log("Mouse left window %d", event->window.windowID);
        gs->mouse.captured = true;
        break;
      case SDL_WINDOWEVENT_FOCUS_GAINED:
        SDL_Log("Window %d gained keyboard focus", event->window.windowID);
        break;
      case SDL_WINDOWEVENT_FOCUS_LOST:
        SDL_Log("Window %d lost keyboard focus", event->window.windowID);
        break;
      case SDL_WINDOWEVENT_CLOSE:
        SDL_Log("Window %d closed", event->window.windowID);
        break;
      default:
        SDL_Log("Window %d got unknown event %d", event->window.windowID,
                event->window.event);
        break;
      }
    }
    case SDL_MOUSEMOTION:
      gs->mouse.x = e.motion.x;
      gs->mouse.y = e.motion.y;
      break;
    case SDL_QUIT:
      printf("quit requested\n");
      gs->running = false;
#ifdef __EMSCRIPTEN__
      emscripten_cancel_main_loop();
#endif
      break;
    case SDL_KEYDOWN:
      switch (e.key.keysym.sym) {
      case SDLK_UP:
        gs->pad.up = true;
        break;
      case SDLK_DOWN:
        gs->pad.down = true;
        break;
      case SDLK_LEFT:
        gs->pad.left = true;
        break;
      case SDLK_RIGHT:
        gs->pad.right = true;
        break;
      }
      break;
    case SDL_KEYUP:
      switch (e.key.keysym.sym) {
      case SDLK_UP:
        gs->pad.up = false;
        break;
      case SDLK_DOWN:
        gs->pad.down = false;
        break;
      case SDLK_LEFT:
        gs->pad.left = false;
        break;
      case SDLK_RIGHT:
        gs->pad.right = false;
        break;
      }
      if (e.key.keysym.sym == 'q') {
        printf("quit requested\n");
        gs->running = false;
#ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop();
#endif
      }
      printf("key: %c\n", e.key.keysym.sym);
      printf("key: %s\n", SDL_GetKeyName(e.key.keysym.sym));
      break;
    default:
      printf("event %u\n", e.type);
      break;
    }
  }
  SDL_ShowCursor(!gs->mouse.captured);
}

#undef main // to shut up winders

typedef struct loadable {
  SDL_Texture **tex;
  const char *filename;
} loadable;

#define ARRAY_COUNT(thing) sizeof(thing) / sizeof(thing[0])
extern "C" int main(int argc, char **argv) {
  game_state gs = {0};
  gs.running = 1;
  gs.lastMs = SDL_GetTicks();
  gs.lastFPSstamp = gs.lastMs;
  printf("hi\n");
  loadable things_to_load[] = {{&gs.bg, "bg_bg.png"},
                               {&gs.tower, "bg_tower.png"},
                               {&gs.road, "bg_road.png"},
                               {&gs.font, "font.png"}};
  gs.clouds.rect.x = 0;
  gs.clouds.rect.y = 24;
  gs.clouds.rect.w = 128;
  gs.clouds.rect.h = 128;

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
  SDL_RenderSetLogicalSize(gs.sdlRenderer, 128, 128);
#ifdef __EMSCRIPTEN__
  emscripten_cancel_main_loop();
#endif
  printf("set 128x128 rendering res\n");
  for (int i = 0; i < ARRAY_COUNT(things_to_load); i++) {
    if (!load_texture_from_bitmap(gs.sdlRenderer, things_to_load[i].tex,
                                  things_to_load[i].filename)) {
      SDL_Quit();
      return false;
    }
    printf("loaded %s!\n", things_to_load[i].filename);
  }
  if (!init_chicken(&gs)) {
    printf("bailing!\n");
    return 1;
  };

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
    SDL_Delay(
        1); // todo sleep a bit less silly-ly, handle shorter periods correctly
  }
  gs->ticks++;
  /// todo handle having to step multiple ticks because fps got behind
  handle_events(gs);
  tick_chicken(gs);
  // todo sky color here
  SDL_SetRenderDrawColor(gs->sdlRenderer, 69, 150, 240, 255);
  //
  SDL_RenderClear(gs->sdlRenderer);

  SDL_RenderCopy(gs->sdlRenderer, gs->bg, 0, 0);
  SDL_RenderCopy(gs->sdlRenderer, gs->road, 0, 0);

  if (gs->ticks % 100 == 0) {
    gs->clouds.rect.x++;
  }
  if (gs->clouds.rect.x > 128) {
    gs->clouds.rect.x = -128;
  }
  SDL_SetTextureAlphaMod(gs->clouds.tex, 255);
  SDL_RenderCopy(gs->sdlRenderer, gs->clouds.tex, 0, &gs->clouds.rect);
  SDL_Rect r = gs->clouds.rect;
  r.x += 2;
  r.y += 2;
  // SDL_RenderCopy(gs->sdlRenderer, gs->clouds.tex, 0, &r);

  SDL_RenderCopy(gs->sdlRenderer, gs->tower, 0, 0);

  render_chicken(gs);

  // pretend more clouds
  r.x = ((gs->ticks / 8) % 256) - 128;
  r.y = -2;
  r.w = 128;
  r.h = 256;
  SDL_SetTextureAlphaMod(gs->clouds.tex, 128);
  // SDL_RenderCopyEx(gs->sdlRenderer, gs->clouds.tex, 0, &r, 0, 0,
  //                  SDL_FLIP_HORIZONTAL);
  render_chicken(gs);
  r.y += 6;
  r.x -= 24;
  // SDL_RenderCopy(gs->sdlRenderer, gs->clouds.tex, 0, &r);

  SDL_Rect letter = {(gs->ticks / 100) % 10 * 5, 0, 5, 8};
  SDL_Rect pos = {16, 16, 10, 16};
  SDL_RenderCopy(gs->sdlRenderer, gs->font, &letter, &pos);
  letter.x = (gs->ticks / 100) % 10 * 5;
  letter.y = 8;
  pos.x = 30;
  SDL_RenderCopy(gs->sdlRenderer, gs->font, &letter, &pos);
  letter.y = 16;
  pos.x = 44;
  SDL_RenderCopy(gs->sdlRenderer, gs->font, &letter, &pos);
  letter.y = 24;
  pos.x = 58;
  SDL_RenderCopy(gs->sdlRenderer, gs->font, &letter, &pos);

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
