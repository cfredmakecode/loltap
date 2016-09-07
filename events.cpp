#include "include/base.h"

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
        gs->mouse.captured = false;
        gs->mouse.button1 = false;
        gs->mouse.button2 = false;
        gs->mouse.button3 = false;
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
      if (gs->mouse.button3) {
        SDL_SetCursor(gs->handcursor);
        move_camera_by(gs, e.motion.xrel, e.motion.yrel);
        break;
      }
      gs->mouse.rect.x = e.motion.x;
      gs->mouse.rect.y = e.motion.y;
      break;
    case SDL_MOUSEWHEEL:
      zoom_camera_by(gs, e.wheel.y);
      break;
    case SDL_FINGERDOWN:
      gs->touch.down = true;
      gs->touch.cur.x = e.tfinger.x * gs->camera.rect.w;
      gs->touch.cur.y = e.tfinger.y * gs->camera.rect.h;
      break;
    case SDL_FINGERUP:
      gs->touch.down = false;
      gs->touch.cur.x = e.tfinger.x * gs->camera.rect.w;
      gs->touch.cur.y = e.tfinger.y * gs->camera.rect.h;
      break;
    case SDL_FINGERMOTION:
      v2 pos;
      pos.x = e.tfinger.x * gs->camera.rect.w;
      pos.y = e.tfinger.y * gs->camera.rect.h;
      gs->touch.rel.x = gs->touch.cur.x - pos.x;
      gs->touch.rel.y = gs->touch.cur.y - pos.y;
      gs->touch.cur.x = pos.x;
      gs->touch.cur.y = pos.y;
      if (gs->touch.down) {
        move_camera_by(gs, -gs->touch.rel.x, -gs->touch.rel.y);
        gs->touch.rel.x = 0;
        gs->touch.rel.y = 0;
      }
      break;
    case SDL_MOUSEBUTTONDOWN:
      switch (e.button.button) {
      case SDL_BUTTON_LEFT:
        gs->mouse.timestamp = SDL_GetTicks();
        // catch all tap events always because the user notices if we don't
        if ((SDL_HasIntersection(&gs->menu.tapbutton.rect, &gs->mouse.rect))) {
          gs->taps++;
          add_peon(&gs->peons, gs);
          gs->mouse.on_tap_target = true;
          gs->menu.open = false;
        }
        gs->mouse.button1 = true;
        break;
      case SDL_BUTTON_MIDDLE:
        gs->mouse.button3 = true;
        break;
      case SDL_BUTTON_RIGHT:
        gs->mouse.button2 = true;
        target_node *target = gs->targets[gs->lastTargetIndex];
        target->pos.x = gs->mouse.rect.x - gs->camera.rect.x;
        target->pos.y = gs->mouse.rect.y - gs->camera.rect.y;
        int t = gs->lastTargetIndex;
        int c = ARRAY_COUNT(gs->targets);
        gs->lastTargetIndex = (t + 1) % c;
        break;
      }
      break;
    case SDL_MOUSEBUTTONUP:
      switch (e.button.button) {
      case SDL_BUTTON_LEFT:
        gs->mouse.timestamp = 0;
        gs->mouse.button1 = false;
        gs->mouse.on_tap_target = false;
        break;
      case SDL_BUTTON_RIGHT:
        gs->mouse.button2 = false;
        break;
      case SDL_BUTTON_MIDDLE:
        gs->mouse.button3 = false;
        SDL_SetCursor(gs->arrowcursor);
        break;
      }
      break;
    case SDL_QUIT:
      printf("quit requested\n");
      gs->running = false;
      die();
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
        die();
        break;
      }
      if (e.key.keysym.sym == '[') {
        zoom_camera_by(gs, 0.1f);
        break;
      }
      if (e.key.keysym.sym == ']') {
        zoom_camera_by(gs, -0.1f);
        break;
      }
      if (e.key.keysym.sym == 'c') {
        printf("re-center/reset requested\n");
        reset_camera(gs);
        break;
      }
      if (e.key.keysym.sym == 'a') {
        printf("add 50 peons requested\n");
        for (int i = 0; i < 50; i++) {
          add_peon(&gs->peons, gs);
        }
        break;
      }
      if (e.key.keysym.sym == SDLK_SPACE) {
        gs->taps++;
        add_peon(&gs->peons, gs);
        break;
      }
      if (e.key.keysym.sym == SDLK_f) {
        SDL_SetWindowFullscreen(gs->sdlWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
        break;
      }
      printf("key: %c\n", e.key.keysym.sym);
      printf("key: %s\n", SDL_GetKeyName(e.key.keysym.sym));
      break;
    default:
      printf("event %u\n", e.type);
      break;
    }
  }
}
