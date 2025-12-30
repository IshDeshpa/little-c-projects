#include <SDL3/SDL_hints.h>
#include <stdio.h>
#include <SDL3/SDL.h>

int main(int argc, char **argv){
  if(!SDL_Init(SDL_INIT_VIDEO)){
    SDL_Log("Init Failure");
    return -1;
  }

  SDL_Window *window = SDL_CreateWindow("SDL3 Test", 512, 512, SDL_WINDOW_RESIZABLE | SDL_WINDOW_BORDERLESS);
  for (int i = 0; i < SDL_GetNumVideoDrivers(); i++) {
      if (strcmp(SDL_GetVideoDriver(i), "wayland") == 0) {
          SDL_SetHintWithPriority(SDL_HINT_VIDEO_DRIVER, "wayland", SDL_HINT_OVERRIDE);
          break;
      }
  }

  SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

  bool done = false;

  SDL_Event e;
  while (!done){
    while(SDL_PollEvent(&e)){
      if (e.type == SDL_EVENT_KEY_DOWN && 
          e.key.key == SDLK_ESCAPE) {
        done = true;
      } else if (e.type == SDL_EVENT_QUIT) {
        done = true;
      }
      printf("%d\n\r", e.type);
    }

    SDL_SetWindowSize(window, 512, 512);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    // Create a rectangle to render
    SDL_FRect rect = {.x = 0, .y = 0, .w = 256, .h = 256};
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
    if(!SDL_RenderFillRect(renderer, &rect)){
      SDL_Log("Rect Render Failure");
      return -1;
    }

    // Another one!
    SDL_FRect rect2 = {.x = 0, .y = 0, .w = 128, .h = 128};
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
    if(!SDL_RenderFillRect(renderer, &rect2)){
      SDL_Log("Rect2 Render Failure");
      return -1;
    }

    SDL_RenderPresent(renderer);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
