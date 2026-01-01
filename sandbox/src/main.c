#include <SDL3/SDL.h>
#include <SDL3/SDL_mutex.h>
#include <SDL3/SDL_thread.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#define HANDLE_SDL_CALL(x) do {if(!x) {SDL_Log("Failed " #x " on line %d", __LINE__); done = true; return -1;}} while(0)
SDL_Window *window;
SDL_Renderer *renderer;
bool done = false;

// PSEUDORANDOM
#define P 2147483647
#define A 16807
int pmcurr = 0;

// Based on https://statmath.wu.ac.at/software/src/prng-3.0.2/doc/prng.html/Table_LCG.html
// Recommended parameters by Park and Miller (pm)
int pmrand(){
  pmcurr = (pmcurr*A) % P;
  return pmcurr;
}
// PSEUDORANDOM

#define GRAIN_SPEED 20 //ms
#define GRAIN_H 5 //pixels

typedef struct grain {
  SDL_FRect pixel;
  SDL_Mutex *mtx;
} grain;

void init_grain(grain *grain){
  grain->pixel.x = 256;
  grain->pixel.y = 0;
  grain->pixel.w = GRAIN_H;
  grain->pixel.h = GRAIN_H;
  grain->mtx = SDL_CreateMutex();
}

void update_grain(grain *grain){
  grain->pixel.y += 1;
  grain->pixel.x += abs(pmrand()) % 3 - 1;
}

const int num_grains = 512*512/(GRAIN_H*GRAIN_H);
grain *grains;
int spawn_ptr = 1;

int update_state(void *data){
  while (!done){
    for(int i=0; i<spawn_ptr; i++){
      SDL_LockMutex(grains[i].mtx);
      update_grain(&grains[i]);
      SDL_UnlockMutex(grains[i].mtx);
    }

    if(spawn_ptr < num_grains) spawn_ptr++;

    SDL_Delay(GRAIN_SPEED);
  }

  return 0;
}

int main(int argc, char **argv){
  // Seed random number generator
  struct timespec tp;
  clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
  pmcurr = tp.tv_nsec;

  HANDLE_SDL_CALL(SDL_Init(SDL_INIT_VIDEO));

  window = SDL_CreateWindow("SDL3 Test", 512, 512, SDL_WINDOW_BORDERLESS);
  renderer = SDL_CreateRenderer(window, NULL);

  HANDLE_SDL_CALL(SDL_SetWindowSize(window, 512, 512));

  grains = calloc(num_grains, sizeof(grain));
  for(int i=0; i<num_grains; i++){
    init_grain(&grains[i]);
  }

  SDL_Thread *state_thread = SDL_CreateThread(update_state, "update state", NULL);
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

    HANDLE_SDL_CALL(SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE));
    HANDLE_SDL_CALL(SDL_RenderClear(renderer));

    HANDLE_SDL_CALL(SDL_SetRenderDrawColor(renderer, 247, 209, 163, SDL_ALPHA_OPAQUE));

    for(int i=0; i<spawn_ptr; i++){
      SDL_LockMutex(grains[i].mtx);
      HANDLE_SDL_CALL(SDL_RenderFillRect(renderer, &grains[i].pixel));
      SDL_UnlockMutex(grains[i].mtx);
    }

    HANDLE_SDL_CALL(SDL_RenderPresent(renderer));
  }

  SDL_WaitThread(state_thread, NULL);

  for(int i=0; i<num_grains; i++){
    SDL_DestroyMutex(grains[i].mtx);
  }
  free(grains);

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  SDL_Quit();

  return 0;
}
