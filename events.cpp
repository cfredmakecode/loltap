#include "include/base.h"

void handle_events(game_state *gs) {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    switch (e.type) {
    case SDL_WINDOWEVENT:
      switch (e.window.event) {
      case SDL_WINDOWEVENT_RESIZED:
      case SDL_WINDOWEVENT_SIZE_CHANGED:
        SDL_Log("Window %d size changed to %dx%d", e.window.windowID,
                e.window.data1, e.window.data2);
        gs->camera.rect.w = e.window.data1;
        gs->camera.rect.h = e.window.data2;
        break;
      default:
        SDL_Log("Window %d got unknown event %d", e.window.windowID,
                e.window.event);
        break;
      }
    case SDL_MOUSEMOTION:
      gs->mouse.rect.x = e.motion.x;
      gs->mouse.rect.y = e.motion.y;
      if (gs->mouse.button1) {
        gs->camera.rect.x -= e.motion.xrel;
        gs->camera.rect.y -= e.motion.yrel;
      }
      break;
    case SDL_MOUSEBUTTONDOWN:
      switch (e.button.button) {
      case SDL_BUTTON_LEFT:
        gs->mouse.button1 = true;
        break;
      case SDL_BUTTON_RIGHT:
        v2 t =
            screen_coords_to_playfield(gs, gs->mouse.rect.x, gs->mouse.rect.y);
        static int i = 0;
        pts.points[i].x = t.x;
        pts.points[i].y = t.y;
        i = (i + 1) % 10;
        break;
      }
      break;
    case SDL_MOUSEBUTTONUP:
      switch (e.button.button) {
      case SDL_BUTTON_LEFT:
        gs->mouse.button1 = false;
        break;
      }
      break;
    case SDL_QUIT:
      printf("quit requested\n");
      gs->running = false;
      die();
      break;
    case SDL_KEYDOWN:
      break;
    case SDL_KEYUP:
      if (e.key.keysym.sym == 'q') {
        printf("quit requested\n");
        gs->running = false;
        die();
        break;
      }
      if (e.key.keysym.sym == 'c') {
        printf("re-center/reset requested\n");
        gs->camera.rect.x = 0;
        gs->camera.rect.y = 0;
        gs->camera.zoom = 1.0f;
        break;
      }
      if (e.key.keysym.sym == SDLK_f) {
        static bool32 fullscreen = false;
        if (fullscreen) {
          SDL_SetWindowFullscreen(gs->sdlWindow, 0);
          SDL_SetWindowSize(gs->sdlWindow, 1280, 720);
          gs->camera.rect.w = 1280;
          gs->camera.rect.w = 720;
        } else {
          SDL_SetWindowFullscreen(gs->sdlWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
        }
        fullscreen = !fullscreen;
        break;
      }
      break;
    default:
      printf("event %u\n", e.type);
      break;
    }
  }
}
