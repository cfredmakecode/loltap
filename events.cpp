#include "include/base.h"

void handle_events(game_state *gs) {
  f32 amount;
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    switch (e.type) {
    case SDL_WINDOWEVENT:
      switch (e.window.event) {
      case SDL_WINDOWEVENT_RESIZED:
      case SDL_WINDOWEVENT_SIZE_CHANGED:
        SDL_Log("Window %d size changed to %dx%d", e.window.windowID,
                e.window.data1, e.window.data2);
        gs->camera.screenw = f32(e.window.data1);
        gs->camera.screenh = f32(e.window.data2);
        break;
      default:
        SDL_Log("Window %d got unknown event %d", e.window.windowID,
                e.window.event);
        break;
      }
    case SDL_MULTIGESTURE: {
      SDL_Log("gesture: touchid %llu dTheta %2.2f dDist %2.2f pos %2.2f,%2.2f "
              "fingers %u",
              e.mgesture.touchId, e.mgesture.dTheta, e.mgesture.dDist,
              e.mgesture.x, e.mgesture.y, e.mgesture.numFingers);
    } break;
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
      {

        // v2 target;
        // target.x = gs->mouse.rect.x;
        // target.y = gs->mouse.rect.y;
        f32 s;
        if (e.wheel.y < 0) {
          s = gs->camera.scale / 2.0f;
        } else {
          s = gs->camera.scale * 2.0f;
        }
        // camera_zoom_to(&gs->camera, target.x, target.y, s);
        gs->camera.scalecenter.x = gs->mouse.rect.x;
        gs->camera.scalecenter.y = gs->mouse.rect.y;

        gs->camera.targetscale = s;
        // v2 before = s2w(&gs->camera, gs->mouse.rect.x, gs->mouse.rect.y);
        // v2 after = s2w(&gs->camera, gs->mouse.rect.x, gs->mouse.rect.y);
        // v2 diff = after - before;
        // diff.x *= gs->camera.scale;
        // diff.y *= gs->camera.scale;
        // gs->camera.pos = gs->camera.pos + diff;
        // gs->camera.target = gs->camera.target + diff;
        // SDL_Log("before %2.2f,%2.2f after %2.2f, %2.2f diff %2.2f,%2.2f",
        //         before.x, before.y, after.x, after.y, diff.x, diff.y);
      }
      printf("mouse wheel y:%d\n", e.wheel.y);
      break;
    case SDL_MOUSEMOTION:
      gs->mouse.rect.x = e.motion.x;
      gs->mouse.rect.y = e.motion.y;
      if (gs->mouse.button1) {
        gs->camera.target.x += e.motion.xrel;
        gs->camera.target.y += e.motion.yrel;
      }
      break;
    case SDL_MOUSEBUTTONDOWN:
      switch (e.button.button) {
      case SDL_BUTTON_LEFT:
        gs->mouse.button1 = true;
        break;
      case SDL_BUTTON_RIGHT: {
        v2 t = s2w(&gs->camera, gs->mouse.rect.x, gs->mouse.rect.y);
        SDL_Log("rightclick: screen %d,%d world %2.2f,%2.2f", gs->mouse.rect.x,
                gs->mouse.rect.y, t.x, t.y);
        v2 s = w2s(&gs->camera, t.x, t.y);
        SDL_Log("reversed  : world %2.2f,%2.2f screen %2.2f,%2.2f", t.x, t.y,
                s.x, s.y);
        t = camera_center2w(&gs->camera);
        SDL_Log("camera centered on world %2.2f,%2.2f", t.x, t.y);
      } break;
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
        gs->camera.targetscale /= 2.0f;
        break;
      }
      if (e.key.keysym.sym == ']') {
        gs->camera.targetscale *= 2.0f;
        break;
      }
      if (e.key.keysym.sym == 'z') {
        gs->camera.shake += 32.0f;
        break;
      }
      if (e.key.keysym.sym == 'd') {
        break;
      }
      if (e.key.keysym.sym == 'c') {
        printf("re-center/reset requested\n");
        camera_reset(&gs->camera);
        SDL_WarpMouseInWindow(gs->sdlWindow, gs->camera.screenw / 2,
                              gs->camera.screenh / 2);
        break;
      }
      if (e.key.keysym.sym == SDLK_f) {
        static bool32 fullscreen = false;
        if (fullscreen) {
          SDL_SetWindowFullscreen(gs->sdlWindow, 0);
          SDL_SetWindowSize(gs->sdlWindow, 1280, 720);
          gs->camera.screenw = 1280;
          gs->camera.screenh = 720;
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
