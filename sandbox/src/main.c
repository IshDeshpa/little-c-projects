#include <SDL3/SDL_hints.h>
#include <stdio.h>
#include <SDL3/SDL.h>

#define HANDLE_SDL_CALL(x) do {if(!x) {SDL_Log("Failed " #x " on line %d", __LINE__); return -1;}} while(0)

int main(int argc, char **argv){
  HANDLE_SDL_CALL(SDL_Init(SDL_INIT_VIDEO));

  SDL_Window *window = SDL_CreateWindow("SDL3 Test", 512, 512, SDL_WINDOW_RESIZABLE | SDL_WINDOW_BORDERLESS);
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
    }

    HANDLE_SDL_CALL(SDL_SetWindowSize(window, 512, 512));

    HANDLE_SDL_CALL(SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE));
    HANDLE_SDL_CALL(SDL_RenderClear(renderer));

    // Create a rectangle to render
    SDL_FRect rect = {.x = 0, .y = 0, .w = 256, .h = 256};
    HANDLE_SDL_CALL(SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE));
    HANDLE_SDL_CALL(SDL_RenderFillRect(renderer, &rect));

    // Another one!
    SDL_FRect rect2 = {.x = 0, .y = 0, .w = 128, .h = 128};
    HANDLE_SDL_CALL(SDL_SetRenderDrawColor(renderer, 0, 0, 255, SDL_ALPHA_OPAQUE));
    HANDLE_SDL_CALL(SDL_RenderFillRect(renderer, &rect2));
    HANDLE_SDL_CALL(SDL_RenderPresent(renderer));
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  SDL_Quit();

  return 0;
}
