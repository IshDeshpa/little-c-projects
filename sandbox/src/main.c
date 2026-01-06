#include <SDL3/SDL.h>
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

#define GRID_SIZE 512

typedef struct grain {
  SDL_FRect *pixel;
  bool update;
} grain;

const int num_grains = (GRID_SIZE * GRID_SIZE)/(GRAIN_H*GRAIN_H);
SDL_Mutex *grain_mtx;
SDL_FRect *grains;
grain *grain_md;

int *col_heights; // len = num_grains/GRAIN_H

int spawn_ptr = 1;

void init_grain(grain *grain, int ind){
  grain->pixel = &grains[ind];
  grain->pixel->x = GRID_SIZE/2;
  grain->pixel->y = 0;
  grain->pixel->w = GRAIN_H;
  grain->pixel->h = GRAIN_H;
  grain->update = true;
}

void update_grain(grain *grain){
  // Check surrounding pixels on screen state
  int rand_dir = abs(pmrand()) % 3 - 1;
  int column = grain->pixel->x / 5;

  if(grain->pixel->y < GRID_SIZE - col_heights[column] - 1) {
    int new_column = (grain->pixel->x + rand_dir) / 5;
    if (grain->pixel->y + 1 < GRID_SIZE - col_heights[new_column] - 1){
      grain->pixel->x += rand_dir; 
    }
    grain->pixel->y += 1;
  } else {
    col_heights[column]++;
    grain->update = false;
  }
}

int update_state(void *data){
  while (!done){
    for(int i=0; i<spawn_ptr; i++){
      SDL_LockMutex(grain_mtx);
      if(grain_md[i].update) update_grain(&grain_md[i]);
      SDL_UnlockMutex(grain_mtx);
    }

    if(spawn_ptr < num_grains) spawn_ptr++;

    SDL_Delay(GRAIN_SPEED);
  }

  return 0;
}

SDL_Mutex *frame_counter_mtx;
int frame_counter = 0;
float fps = 0;

int update_debug(void *data){
  while (!done){
    SDL_Delay(500);
    fps = frame_counter / 0.5;

    SDL_LockMutex(frame_counter_mtx);
    frame_counter = 0;
    SDL_UnlockMutex(frame_counter_mtx);
  }

  return 0;
}

int main(int argc, char **argv){
  // Seed random number generator
  struct timespec tp;
  clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
  pmcurr = tp.tv_nsec;

  HANDLE_SDL_CALL(SDL_Init(SDL_INIT_VIDEO));

  window = SDL_CreateWindow("SDL3 Test", GRID_SIZE, GRID_SIZE, SDL_WINDOW_BORDERLESS);
  renderer = SDL_CreateRenderer(window, NULL);

  HANDLE_SDL_CALL(SDL_SetWindowSize(window, GRID_SIZE, GRID_SIZE));

  grains = calloc(num_grains, sizeof(SDL_FRect));
  grain_md = calloc(num_grains, sizeof(grain));
  for(int i=0; i<num_grains; i++){
    init_grain(&grain_md[i], i);
  }
  grain_mtx = SDL_CreateMutex();

#ifdef DEBUG
  frame_counter_mtx = SDL_CreateMutex();
#endif

  col_heights = calloc(num_grains/GRAIN_H, sizeof(int));
  for(int i=0; i<num_grains/GRAIN_H; i++){
    col_heights[i] = 0;
  }

#ifdef DEBUG
  SDL_Thread *debug_thread = SDL_CreateThread(update_debug, "update debug", NULL);
#endif
  SDL_Thread *state_thread = SDL_CreateThread(update_state, "update state", NULL);

  SDL_Event e;

  while (!done){
    uint64_t time_a = SDL_GetTicks();
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

    HANDLE_SDL_CALL(SDL_RenderFillRects(renderer, grains, num_grains));

#ifdef DEBUG
    HANDLE_SDL_CALL(SDL_RenderDebugTextFormat(renderer, 5.0, 5.0, "%.2f", fps));
#endif

    HANDLE_SDL_CALL(SDL_RenderPresent(renderer));

#ifdef DEBUG
    SDL_LockMutex(frame_counter_mtx);
    frame_counter++;
    SDL_UnlockMutex(frame_counter_mtx);
#endif

    uint64_t time_b = SDL_GetTicks();
    SDL_Delay(17 - (time_b - time_a)); // cap to 60fps
  }

  SDL_WaitThread(state_thread, NULL);

#ifdef DEBUG
  SDL_WaitThread(debug_thread, NULL);
#endif

  SDL_DestroyMutex(grain_mtx);

  free(grains);

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  SDL_Quit();

  return 0;
}
