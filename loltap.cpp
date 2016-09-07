#include "include/base.h"

#include "menu.cpp"
#include "peon.cpp"
#include "events.cpp"

#undef main // to shut up winders

extern "C" int main(int argc, char **argv) {
  srand(SDL_GetPerformanceCounter());
  game_state gs = {0};
  gs.running = 1;
  gs.lastMs = SDL_GetTicks();
  gs.lastFPSstamp = gs.lastMs;
  struct {
    SDL_Texture **tex;
    const char *filename;
  } things_to_load[] = {{&gs.bg, "bg_BG.png"},
                        {&gs.tower, "tower.png"},
                        {&gs.grid, "gridmaybe.png"},
                        {&gs.road, "bg_Road.png"},
                        {&gs.clouds.tex, "clouds.png"},
                        {&gs.isometric, "isometrics.png"},
                        {&gs.chicken.tex, "chicken_chicken.png"},
                        {&gs.default_peon, "peon.png"},
                        {&gs.menu.font, "font.png"}};
  gs.clouds.rect.x = 0;
  gs.clouds.rect.y = 24;
  gs.clouds.rect.w = 128;
  gs.clouds.rect.h = 128;
  gs.mouse.rect.w = 1;
  gs.mouse.rect.h = 1;
  gs.zoom = 1.0f;

  gs.peons.spawner.x = rand() % 500;
  gs.peons.spawner.y = rand() % 500;

  target_node debug_target_node = {0, 0, {16.0f, 100.0f}};
  target_node second_target_node = {0, 0, {100.0f, 16.0f}};
  target_node third_target_node = {0, 0, {120.0f, 50.0f}};
  debug_target_node.next = &second_target_node;
  second_target_node.next = &third_target_node;
  third_target_node.next = &debug_target_node;
  gs.targets[0] = &debug_target_node;
  gs.targets[1] = &second_target_node;
  gs.targets[2] = &third_target_node;

  if (!init_menu(&gs)) {
    return die();
  }

#ifdef __EMSCRIPTEN__
  // IMPORTANT(caf): this set and unset loop is a weird workaround I found
  // to shut emscripten up about a main loop when using SDL2
  emscripten_set_main_loop_arg(&emscripten_loop_workaround, (void *)&gs, 0, 0);
#endif

  SDL_CreateWindowAndRenderer(SCREEN_W, SCREEN_H, SDL_WINDOW_RESIZABLE,
                              &gs.sdlWindow, &gs.sdlRenderer);
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");
  SDL_RenderSetLogicalSize(gs.sdlRenderer, LOGICAL_WIDTH, LOGICAL_HEIGHT);

#ifdef __EMSCRIPTEN__
  emscripten_cancel_main_loop();
#endif

  printf("set rendering res %dx%d\n", LOGICAL_HEIGHT, LOGICAL_WIDTH);
  for (int i = 0; i < ARRAY_COUNT(things_to_load); i++) {
    char buf[1024];
    sprintf(buf, "assets/%s", things_to_load[i].filename);
    if (!load_texture_from_bitmap(gs.sdlRenderer, things_to_load[i].tex, buf)) {
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Missing file", buf, NULL);
      return die();
    }
    printf("loaded %s!\n", things_to_load[i].filename);
  }

  add_peon(&gs.peons, &gs);

  gs.handcursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
  gs.arrowcursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);

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

#define HELD_DOWN_MIN_TICKS 300
void main_loop(game_state *gs) {
  while (1) {
    uint32_t toWait = SDL_GetTicks() - gs->lastMs;
    if (toWait >= MS_PER_TICK) {
      break;
    }
    if (toWait <= 1) {
      continue;
    }
    SDL_Delay(1);
    // todo sleep a bit less silly-ly, handle shorter periods correctly
    // TODO fix tickrate to tick multiple times till caught up, currently
    // ticks
    // are based on FPS, just capped at 60
  }
  gs->ticks++;

  if (gs->mouse.on_tap_target && gs->mouse.timestamp &&
      SDL_GetTicks() - gs->mouse.timestamp > HELD_DOWN_MIN_TICKS) {
    // obviously trickery with rate of auto click stuff goes here later
    if (gs->ticks % 10 == 0) {
      gs->taps++;
      add_peon(&gs->peons, gs);
    }
  }
  /// todo handle having to step multiple ticks because fps got behind
  handle_events(gs);
  tick_menu(gs);
  tick_peons(&gs->peons);

  SDL_SetRenderDrawColor(gs->sdlRenderer, 0x49, 0x60, 0x0, 0xff);
  SDL_RenderClear(gs->sdlRenderer);

  SDL_Rect grid = {0};
  grid.w = 64 * gs->zoom;
  grid.h = 32 * gs->zoom;
  int scr_y = int(gs->scroll.y) % grid.h;
  int scr_x = int(gs->scroll.x) % grid.w;
  // to ensure we draw enough to go offscreen, give me another one
  scr_y -= grid.h;
  scr_x -= grid.w;
  // and then add another in the other direction in this loop..
  for (grid.y = scr_y; grid.y < LOGICAL_HEIGHT + grid.h; grid.y += grid.h) {
    for (grid.x = scr_x; grid.x < LOGICAL_WIDTH + grid.w; grid.x += grid.w) {
      SDL_RenderCopy(gs->sdlRenderer, gs->grid, 0, &grid);
    }
  }

  if (gs->ticks % 100 == 0) {
    gs->clouds.rect.x++;
  }
  if (gs->clouds.rect.x > 128) {
    gs->clouds.rect.x = -128;
  }
  SDL_SetRenderDrawBlendMode(gs->sdlRenderer, SDL_BLENDMODE_BLEND);
  SDL_SetTextureAlphaMod(gs->clouds.tex, 0xA0);
  SDL_RenderCopy(gs->sdlRenderer, gs->clouds.tex, 0, &gs->clouds.rect);
  SDL_Rect r = gs->clouds.rect;

  // pretend more clouds
  r.x = ((gs->ticks / 8) % 256) - 128;
  r.y = -2;
  r.w = 128;
  r.h = 256;
  SDL_SetTextureAlphaMod(gs->clouds.tex, 128);
  SDL_RenderCopyEx(gs->sdlRenderer, gs->clouds.tex, 0, &r, 0, 0,
                   SDL_FLIP_HORIZONTAL);
  render_peons(&gs->peons, &gs->drawitems);
  r.y += 6;
  r.x -= 24;
  SDL_RenderCopy(gs->sdlRenderer, gs->clouds.tex, 0, &r);

  int w, h;
  SDL_QueryTexture(gs->tower, NULL, NULL, &w, &h);
  SDL_Rect towersrc;
  SDL_Rect towerdst;
  towersrc.x = 0;
  towersrc.y = 0;
  towersrc.w = w;
  towersrc.h = h;
  towerdst.x = 200;
  towerdst.y = 500;
  towerdst.w = w / 2;
  towerdst.h = h / 2;
  push_drawitem(&gs->drawitems, gs->tower, &towersrc, &towerdst);
  remembered_var int t2x = (rand() % LOGICAL_WIDTH);
  towerdst.x = t2x;
  remembered_var int t2y = (rand() % LOGICAL_HEIGHT);
  towerdst.y = t2y;
  towerdst.w = -towerdst.w;
  push_drawitem(&gs->drawitems, gs->tower, &towersrc, &towerdst);
  remembered_var int t3x = (rand() % LOGICAL_WIDTH);
  towerdst.x = t3x;
  remembered_var int t3y = (rand() % LOGICAL_HEIGHT);
  towerdst.y = t3y;
  towerdst.w = -towerdst.w;
  push_drawitem(&gs->drawitems, gs->tower, &towersrc, &towerdst);
  remembered_var int t4x = (rand() % LOGICAL_WIDTH);
  towerdst.x = t4x;
  remembered_var int t4y = (rand() % LOGICAL_HEIGHT);
  towerdst.y = t4y;
  towerdst.w = -towerdst.w;
  push_drawitem(&gs->drawitems, gs->tower, &towersrc, &towerdst);

  render_drawitemstack(&gs->drawitems, gs->sdlRenderer, gs);

  for (int i = 0; i < ARRAY_COUNT(gs->targets); i++) {
    SDL_Rect temp = {0};
    temp.h = 4;
    temp.w = 4;
    temp.x = gs->targets[i]->pos.x - 1;
    temp.x *= gs->zoom;
    temp.x += gs->scroll.x;
    temp.y = gs->targets[i]->pos.y - 1;
    temp.y *= gs->zoom;
    temp.y += gs->scroll.y;
    SDL_SetRenderDrawColor(gs->sdlRenderer, 0xff, 0xff, 0xff, 0xff);
    SDL_RenderDrawRect(gs->sdlRenderer, &temp);
    temp.h = 2;
    temp.w = 2;
    temp.x += 1;
    temp.y += 1;
    SDL_SetRenderDrawColor(gs->sdlRenderer, 200, 20, 40, 0xff);
    SDL_RenderDrawRect(gs->sdlRenderer, &temp);
  }

  // "UI", if you will
  render_menu(gs);

  SDL_SetRenderDrawColor(gs->sdlRenderer, 20, 20, 40, 240);
  SDL_RenderFillRect(gs->sdlRenderer, &gs->mouse.rect);

  SDL_RenderPresent(gs->sdlRenderer);
  gs->frames++;
  if (SDL_GetTicks() - gs->lastFPSstamp > 1000) {
    if (gs->mouse.button1) {
      SDL_Log("Mouse button 1 down!");
    }
    if (gs->mouse.button2) {
      SDL_Log("Mouse button 2 down!");
    }
    gs->fps = gs->frames;
    gs->frames = 0;
    gs->lastFPSstamp = SDL_GetTicks();
  }
  gs->lastMs = SDL_GetTicks();
}

void emscripten_loop_workaround(void *gs) { main_loop((game_state *)gs); }
