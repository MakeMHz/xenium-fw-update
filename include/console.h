#pragma once

#if _XBOX
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

#define CONSOLE_MAX_LINES 13
#define CONSOLE_LAST_LINE CONSOLE_MAX_LINES - 1

class Console {
    public:
        Console(void (*callback)(void), uint16_t pos_x, uint16_t pos_y);
        virtual ~Console();
        void render(SDL_Renderer *renderer);

        void skip();
        void print(const char *string, bool increment);
        void printf(const char *fmt, ...);
        void print_progress(uint16_t size, uint32_t start, uint32_t end, uint32_t progress);
    protected:
    private:
        // Refrence to render callback
        void (*callback)(void);

        //
        const SDL_Color font_color = { 0xFF, 0xFF, 0xFF, 0xFF };

        //
        SDL_Texture *console_line[CONSOLE_MAX_LINES] = { NULL };
        SDL_Rect console_line_pos[CONSOLE_MAX_LINES];

        // Console screen position
        uint32_t position_x = 0;
        uint32_t position_y = 0;

        // Track current line in our line buffer array
        uint16_t current_line = 0;
};
