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
    case SDL_MOUSEWHEEL:
      // IMPORTANT(caf): the wheel values aren't based in reality, or pixels or
      // steps or anything
      // I get +1 or -1 on windows, but +-166 in a browser (I assume it's a
      // "line height" in px..)
      // there doesn't appear to be a way to query the environment for a way to
      // normalize the wheel :(

      if (e.wheel.which != 0) {
        // note(caf): ... it's not clear what other `which`es are. i get extra
        // events in browser and in windows that have a different ID and wildly
        // different distances, so just ignore anything other than the mouse, i
        // guess
        break;
      }
      gs->camera.zoom -= e.wheel.y < 0 ? 0.1f : -0.1f;
      printf("mouse wheel y:%d\n", e.wheel.y);
      break;
    case SDL_MOUSEMOTION:
      gs->mouse.rect.x = e.motion.x;
      gs->mouse.rect.y = e.motion.y;
      if (gs->mouse.button1) {
        // TODO(caf): zoom centered around mouse x/y or pinch x/y
        gs->camera.rect.x -= (e.motion.xrel * (1 / gs->camera.zoom));
        gs->camera.rect.y -= (e.motion.yrel * (1 / gs->camera.zoom));
        // this is close but still "loses" steps when moving while zoomed in.
        // might be OK if we don't care precisely how zoomed in camera moves
        // specifically works
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
      if (e.key.keysym.sym == '[') {
        gs->camera.zoom -= 0.1f;
        break;
      }
      if (e.key.keysym.sym == ']') {
        gs->camera.zoom += 0.1f;
        break;
      }
      if (e.key.keysym.sym == 'z') {
        gs->camera.zoom = 1.0f;
        break;
      }
      if (e.key.keysym.sym == 'd') {
        printf("camera: %d,%d %dx%d zoom: %f\n", gs->camera.rect.x,
               gs->camera.rect.y, gs->camera.rect.h, gs->camera.rect.w,
               gs->camera.zoom);
        break;
      }
      if (e.key.keysym.sym == 'c') {
        printf("re-center/reset requested\n");
        gs->camera.rect.x = 0;
        gs->camera.rect.y = 0;
        gs->camera.zoom = 1.0f;
        SDL_WarpMouseInWindow(gs->sdlWindow, gs->camera.rect.w / 2,
                              gs->camera.rect.h / 2);
        break;
      }
      if (e.key.keysym.sym == SDLK_f) {
        static bool32 fullscreen = false;
        if (fullscreen) {
          SDL_SetWindowFullscreen(gs->sdlWindow, 0);
          SDL_SetWindowSize(gs->sdlWindow, 1280, 720);
          gs->camera.rect.w = 1280;
          gs->camera.rect.w = 720;
          gs->camera.zoom = 1.0f;
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
