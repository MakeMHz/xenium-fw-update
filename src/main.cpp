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
#include <stdio.h>
#include <string.h>

#ifdef _XBOX
#include <hal/debug.h>
#include <hal/xbox.h>
#include <hal/video.h>
#include <windows.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#endif

#include "assets.h"
#include "common.h"
#include "console.h"
#include "crc32.h"
#include "debug.h"
#include "main.h"
#include "xenium.h"
#include "versions.h"

SDL_Window *window = NULL;
SDL_Renderer *gRenderer = NULL;

TTF_Font *gFontSmall = NULL;

bool running = true;

uint8_t *bank_xeniumos      = NULL;
uint8_t *bank_xeniumos_data = NULL;
uint8_t *bank_bootloader    = NULL;

uint32_t bank_xeniumos_crc32      = 0;
uint32_t bank_xeniumos_data_crc32 = 0;
uint32_t bank_bootloader_crc32    = 0;

//
Version *matched_version = NULL;

//
UpdateState state = UpdateState::WarningScreen;

//
Console *console = NULL;

//
SDL_Texture *texture_warning = NULL;
SDL_Texture *texture_background = NULL;

void render(void) {
    // Clear screen
    SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(gRenderer);

    if (state == UpdateState::WarningScreen) {
        SDL_RenderCopy(gRenderer, texture_warning, NULL, NULL);
    } else {
        SDL_RenderCopy(gRenderer, texture_background, NULL, NULL);
    }

    console->render(gRenderer);
    SDL_RenderPresent(gRenderer);

    Sleep(500);
}

void init_sdl(void){
    #ifdef _XBOX
    XVideoSetMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, REFRESH_DEFAULT);
    #endif

    // Enable standard application logging
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL video.\n");
        printSDLErrorAndReboot();
    }

    // Load joystick
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");

    // TODO: Cleanup logic
    if (SDL_NumJoysticks() < 1) {
        debugPrint("Couldn't find any joysticks.\n");
    } else {
        SDL_GameController *controller = NULL;
        for (int i = 0; i < SDL_NumJoysticks(); ++i) {
            if (SDL_IsGameController(i)) {
                controller = SDL_GameControllerOpen(i);
                if (controller == nullptr) {
                    debugPrint("Couldn't find any joysticks.\n");
                }
            }
        }
    }

    // Load TTF library
    if (TTF_Init() != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TTF_Init failed: %s\n", TTF_GetError());
        printSDLErrorAndReboot();
    }

    // Creat SDL window
    window = SDL_CreateWindow("OpenXenium Firmware Update", SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    if (window == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Window creation failed: %s\n", SDL_GetError());
    }

    // Load SDL_image
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't intialize SDL_image.\n");
        SDL_VideoQuit();
        printIMGErrorAndReboot();
    }

    // Create SDL renderer
    gRenderer = SDL_CreateRenderer(window, -1, 0);
    if (gRenderer == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "CreateRenderer failed: %s\n", SDL_GetError());
        printSDLErrorAndReboot();
    }
}

int main(void)
{
    init_sdl();

    // Load in font
    gFontSmall = loadFont((const void *)&asset_font, sizeof(asset_font), 16);

    // Load in warning screen image
    texture_warning = loadTexture(gRenderer, (const void *)&asset_warning, sizeof(asset_warning));

    // Load in background image
    texture_background = loadTexture(gRenderer, (const void *)&asset_background, sizeof(asset_background));

    // Initialize the text console
    // NOTE: Make sure to initialize the console after the background textures
    //       since the render callbacks assumes they're already loaded.
    console = new Console(&render, 86, 146);

    // Allocate memory for banks
    bank_xeniumos      = (uint8_t *)malloc(XENIUM_BANK_XENIUMOS_SIZE);
    bank_xeniumos_data = (uint8_t *)malloc(sizeof(patch_xeniumos_data));
    bank_bootloader    = (uint8_t *)malloc(XENIUM_BANK_BOOTLOADER_SIZE);

    while (running) {
        // Check for events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            int8_t button_press = proccess_event(event);

            if (state == UpdateState::WarningScreen && button_press == SDL_CONTROLLER_BUTTON_X) {
                state = UpdateState::WelcomeScreen;
            }
        }

        switch (state) {
        case UpdateState::WarningScreen:
            break;
        case UpdateState::WelcomeScreen:
            console->printf("Xenium Firmware Update");
            console->printf("Update Version: %d.%d.%d", 2, 3, 4);
            console->printf("------------------------------");

            state = UpdateState::CheckXeniumDetected;
            break;
        case UpdateState::CheckXeniumDetected:
            if (xenium_is_detected()) {
                console->printf("Xenium Detected!");
                state = UpdateState::ReadXeniumOS;
            } else {
                console->printf("ERROR: Xenium not detected!");
                state = UpdateState::DoneError;
            }
            break;
        // Read in XeniumOS
        case UpdateState::ReadXeniumOS:
            console->printf("Reading Flash - XeniumOS");

            xenium_set_bank(XENIUM_BANK_XENIUMOS);
            xenium_flash_read_stream(0, bank_xeniumos, XENIUM_BANK_XENIUMOS_SIZE);

            state = UpdateState::ReadBootloader;
            break;
        // Read in bootloader
        case UpdateState::ReadBootloader:
            console->printf("Reading Flash - Bootloader");

            xenium_set_bank(XENIUM_BANK_BOOTLOADER);
            xenium_flash_read_stream(0, bank_bootloader, XENIUM_BANK_BOOTLOADER_SIZE);

            state = UpdateState::CalculateCRCs;
            break;
        // Calculate CRC32s
        case UpdateState::CalculateCRCs:
            console->printf("Calculating CRC32s");

            bank_xeniumos_crc32   = CRC_result(CRC_add(CRC_init(), (uint8_t const *)bank_xeniumos, XENIUM_BANK_XENIUMOS_SIZE));
            bank_bootloader_crc32 = CRC_result(CRC_add(CRC_init(), (uint8_t const *)bank_bootloader, XENIUM_BANK_BOOTLOADER_SIZE));

            state = UpdateState::MatchVersion;
            break;
        // Match version
        case UpdateState::MatchVersion:
            // Check to see this matches any known values
            for (uint32_t i = 0; i < (sizeof(XeniumVersions) / sizeof(Version)); i++) {
                if (XeniumVersions[i].crc32_bootloader == bank_bootloader_crc32 &&
                    XeniumVersions[i].crc32_bootloader == bank_bootloader_crc32) {
                    // Store reference
                    matched_version = (Version *)&XeniumVersions[i];
                    break;
                }
            }

            // Check if have match
            if(matched_version) {
                console->printf("Found XeniumOS");
                console->printf("  %s", matched_version->name);
                state = UpdateState::EraseBootloader;
            } else if(bank_xeniumos_crc32 == 0x0A3D160A && bank_bootloader_crc32 == 0xD8A29B51) {
                console->printf("XeniumOS already updated");
                state = UpdateState::Done;
            } else {
                console->printf("Unknown XeniumOS version detected");
                console->printf("  X: 0x%08x B: 0x%08x", bank_xeniumos_crc32, bank_bootloader_crc32);
                state = UpdateState::DoneError;
            }
            break;
        // Erase bootloader
        case UpdateState::EraseBootloader:
            console->printf("Erasing  Bootloader");

            xenium_set_bank(XENIUM_BANK_BOOTLOADER);
            xenium_flash_reset();

            for (uint32_t sector = XENIUM_BANK_BOOTLOADER_OFFSET;
                sector < XENIUM_BANK_BOOTLOADER_OFFSET + XENIUM_BANK_BOOTLOADER_SIZE; sector += XENIUM_FLASH_SECTOR_SIZE)
            {
                xenium_start_sector_erase(sector);
                do { Sleep(1000); } while(xenium_flash_busy());
            }

            state = UpdateState::ProgramBootloader;
            break;
        // Program Bootloader
        case UpdateState::ProgramBootloader:
            console->printf("Flashing Bootloader");

            xenium_set_bank(XENIUM_BANK_BOOTLOADER);
            xenium_flash_reset();

            for (uint32_t i = 0; i < sizeof(patch_bootloader); i++) {
                xenium_start_flash_program_byte(XENIUM_BANK_BOOTLOADER_OFFSET + i, patch_bootloader[i]);
                while(xenium_flash_busy()) { ;; }
            }

            state = UpdateState::EraseXOS;
            break;
        // Erase XOS
        case UpdateState::EraseXOS:
            console->printf("Erasing  XOS Data");

            xenium_set_bank(XENIUM_BANK_RECOVERY);
            xenium_flash_reset();

            // NOTE: We're not touching the user config.
            for (uint32_t sector = XENIUM_BANK_XENIUM_DATA_OFFSET;
                sector < XENIUM_BANK_XENIUM_DATA_OFFSET + sizeof(patch_xeniumos_data); sector += XENIUM_FLASH_SECTOR_SIZE)
            {
                xenium_start_sector_erase(sector);
                do { Sleep(1000); } while(xenium_flash_busy());
            }

            state = UpdateState::ProgramXOS;
            break;
        // Program XOS
        case UpdateState::ProgramXOS:
            console->printf("Flashing XOS Data");

            xenium_set_bank(XENIUM_BANK_RECOVERY);
            xenium_flash_reset();

            for (uint32_t i = 0; i < sizeof(patch_xeniumos_data); i++) {
                xenium_start_flash_program_byte(XENIUM_BANK_XENIUM_DATA_OFFSET + i, patch_xeniumos_data[i]);
                while(xenium_flash_busy()) { ;; }
            }

            state = UpdateState::ReadVerificationBootloader;
            break;
        // Read in Bootloader for Verification
        case UpdateState::ReadVerificationBootloader:
            console->printf("Reading Flash - Bootloader");

            xenium_set_bank(XENIUM_BANK_BOOTLOADER);
            xenium_flash_read_stream(0, bank_bootloader, XENIUM_BANK_BOOTLOADER_SIZE);

            state = UpdateState::ReadVerificationXeniumData;
            break;
        // Read in XOS Data for Verification
        case UpdateState::ReadVerificationXeniumData:
            console->printf("Reading Flash - XOS Data");

            xenium_set_bank(XENIUM_BANK_RECOVERY);
            xenium_flash_read_stream(0, bank_xeniumos_data, sizeof(patch_xeniumos_data));

            state = UpdateState::CalculateVerificationCRCs;
            break;
        // Calculate CRC32s for Verification
        case UpdateState::CalculateVerificationCRCs:
            console->printf("Calculating CRC32s");

            bank_bootloader_crc32    = CRC_result(CRC_add(CRC_init(), (uint8_t const *)bank_bootloader, XENIUM_BANK_BOOTLOADER_SIZE));
            bank_xeniumos_data_crc32 = CRC_result(CRC_add(CRC_init(), (uint8_t const *)bank_xeniumos_data, sizeof(patch_xeniumos_data)));

            if(bank_bootloader_crc32 == 0xD8A29B51 && bank_xeniumos_data_crc32 == 0xB39AAC57) {
                state = UpdateState::Done;
            } else {
                console->printf("Flash verification failed");
                state = UpdateState::EraseBootloader;
            }
            break;
        //
        case UpdateState::Done:
            console->printf("Completed!");
            console->printf("It is now safe to restart");
            console->printf("your Xbox system.");
            state = UpdateState::Exit;
            break;
        case UpdateState::DoneError:
            console->printf("An error has occured! Please");
            console->printf("refer to URL below for more");
            console->printf("information.");
            console->printf("makemhz.com/xos-upgrade-help");
            state = UpdateState::Exit;
            break;
        case UpdateState::Exit:
            break;
        }

        // Redraw the screen again just in case.
        // (Most screen updates are handled via a callback when then console is updated.)
        render();
    }

    SDL_VideoQuit();

    return 0;
}
