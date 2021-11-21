#ifdef _XBOX
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <hal/debug.h>
#include <hal/video.h>
#include <hal/xbox.h>
#include <windows.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#endif

#include "assets.h"
#include "debug.h"

extern SDL_Renderer *gRenderer;

SDL_Texture *loadTexture(SDL_Renderer *renderer, const void *mem, int size) {
    SDL_Texture *newTexture = NULL;

    SDL_RWops   *mem_rw  = SDL_RWFromConstMem(mem, size);
    SDL_Surface *surface = IMG_Load_RW(mem_rw, 0);

    if (surface == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to load image! SDL_image Error: %s\n", IMG_GetError());
        printSDLErrorAndReboot();
    } else {
        // Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface(renderer, surface);
        if (newTexture == NULL) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unable to create texture! SDL Error: %s\n", SDL_GetError());
            printSDLErrorAndReboot();
        }

        SDL_FreeSurface(surface);
    }

    return newTexture;
}

TTF_Font *loadFont(const void *mem, int size, int ptsize) {
    SDL_RWops *mem_rw = SDL_RWFromConstMem(mem, size);
    TTF_Font  *font   = TTF_OpenFontRW(mem_rw, 1, ptsize);

    if (font == NULL) {
        SDL_VideoQuit();
        printSDLErrorAndReboot();
    }

    return font;
}

SDL_Texture *drawText(TTF_Font *font, SDL_Color font_color, const char *text) {
	SDL_Texture* texture = NULL;
	SDL_Surface* surface = TTF_RenderText_Blended(font, text, font_color);

	if(surface == NULL) {
		debugPrint("TTF_RenderText failed: %s", TTF_GetError());
		Sleep(2000);
	}

	texture = SDL_CreateTextureFromSurface(gRenderer, surface);
	SDL_FreeSurface(surface);
	if(texture == NULL) {
		debugPrint("Couldn't create texture: %s\n", SDL_GetError());
		Sleep(2000);
	}

	return texture;
}

int8_t proccess_event(SDL_Event event) {
    int8_t button_press = SDL_CONTROLLER_BUTTON_INVALID;

    switch (event.type)
    {
    case SDL_CONTROLLERBUTTONDOWN:
        button_press = event.cbutton.button;
        break;
    default:
        break;
    }

    return button_press;
}
