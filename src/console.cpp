/*
 * xenium-fw-update
 *
 * Copyright (c) 2021 MakeMHz LLC
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */
#ifdef _XBOX
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdarg.h>
#include <stdio.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#endif

#include "common.h"
#include "console.h"

extern SDL_Window *window;
extern SDL_Renderer *gRenderer;
extern TTF_Font *gFontSmall;

Console::Console(void (*callback)(void), uint16_t pos_x, uint16_t pos_y) {
    // Store refrence to render callback
    this->callback = callback;

    // Store console screen position
    position_x = pos_x;
    position_y = pos_y;

    for (uint8_t index = 0; index < CONSOLE_MAX_LINES; index++) {
        console_line_pos[index].x = pos_x;
    }
}

Console::~Console(void) {
    for (uint8_t index = 0; index < CONSOLE_MAX_LINES; index++) {
        if (console_line[index] != NULL) {
            SDL_DestroyTexture(console_line[index]);
        }
    }
}

void Console::skip() {
    current_line = (current_line + 1) % CONSOLE_MAX_LINES;
}

void Console::print(const char *string, bool increment) {
    uint16_t process_line = increment ? current_line : (current_line == 0 ? CONSOLE_MAX_LINES : current_line) - 1;

    // Clear previous line texture if it exists
    if (console_line[process_line] != NULL) {
        SDL_DestroyTexture(console_line[process_line]);
        console_line[process_line] = NULL;
    }

    console_line[process_line] = drawText(gFontSmall, font_color, string);

    // Store texture size
    SDL_QueryTexture(console_line[process_line], NULL, NULL,
        &console_line_pos[process_line].w, &console_line_pos[process_line].h);

    if(increment)
        current_line = (current_line + 1) % CONSOLE_MAX_LINES;

    // Call render callback
    this->callback();
}

void Console::printf(const char *fmt, ...) {
    char buffer[300] = { 0 };

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    this->print((const char *)&buffer, 1);
}

// TODO: Make safe or just use a progress bar :P
void Console::print_progress(uint16_t size, uint32_t start, uint32_t end, uint32_t progress) {
    uint8_t  percent_n = (progress - start) * 100 / (end - start);
    uint32_t percent_b = percent_n * size / 100;


    char buffer[300] = { 0 };
    snprintf(buffer, sizeof(buffer), " %3d%", percent_n);

    //
    auto input_length = strlen(buffer);

    buffer[input_length + 0] = ' ';
    buffer[input_length + 1] = '[';
    memset(buffer + input_length + 2, ' ', size);
    memset(buffer + input_length + 2, '=', percent_b);
    buffer[input_length + 2 + size] = ']';
    buffer[input_length + 3 + size] = 0;

    this->print((const char *)&buffer, 0);
}

void Console::render(SDL_Renderer *renderer) {
    uint32_t draw_line = current_line;
    uint32_t current_y = 146;

    for (uint8_t index = 0; index < CONSOLE_MAX_LINES; index++) {
        if (console_line[draw_line] != NULL) {
            // Update SDL_Rect position
            console_line_pos[draw_line].y = current_y;
            current_y += 20;

            SDL_RenderCopy(renderer, console_line[draw_line], NULL, &console_line_pos[draw_line]);
        }

        draw_line = (draw_line + 1) % CONSOLE_MAX_LINES;
    }
}
