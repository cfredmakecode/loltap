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
        // SDL_ShowCursor(!gs->mouse.captured);
        break;
      case SDL_WINDOWEVENT_LEAVE:
        SDL_Log("Mouse left window %d", event->window.windowID);
        gs->mouse.captured = false;
        // SDL_ShowCursor(!gs->mouse.captured);
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
      gs->mouse.rect.x = e.motion.x;
      gs->mouse.rect.y = e.motion.y;
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
      case SDL_BUTTON_RIGHT:
        gs->mouse.button2 = true;
        gs->targets[gs->lastTargetIndex]->pos.x = gs->mouse.rect.x;
        gs->targets[gs->lastTargetIndex]->pos.y = gs->mouse.rect.y;
        int t = gs->lastTargetIndex;
        int c = ARRAY_COUNT(gs->targets);
        gs->lastTargetIndex = (t + 1) % c;
        // gs->lastTargetIndex =
        //     (gs->lastTargetIndex + 1) % ARRAY_COUNT(gs->targets);
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
