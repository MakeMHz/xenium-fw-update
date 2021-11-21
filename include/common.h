#pragma once

#if _XBOX
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

SDL_Texture *loadTexture(SDL_Renderer *renderer, const void *mem, int size);
TTF_Font *loadFont(const void *mem, int size, int ptsize);
SDL_Texture *drawText(TTF_Font *font, SDL_Color font_color, const char *text);
int8_t proccess_event(SDL_Event event);
