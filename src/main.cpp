/*
 * Shoot It (c) Talon1024 2026-present
 *
 * GPLv2 License
 */

#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <cstdint>

#define VIEW_WIDTH 240
#define VIEW_HEIGHT 160

enum class GameState {
    Loading,
    LoadFail,
    Play, // or LoadSuccess
    YouWin, // needed?
    GameOver,
};

/* We will use this renderer to draw into this window every frame. */
static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;
static SDL_AsyncIOQueue* queue = nullptr;
static GameState gameState = GameState::Loading;

struct PendingExternalAsset {
    const char* fname;
    SDL_AsyncIOOutcome outcome;
};

#include "assets.h"

uint32_t getPrimaryDisplay();
SDL_Texture* loadAsset(SDL_Renderer* renderer, const char* fname);
bool loadAssetAsync(const char* asset);

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    SDL_SetAppMetadata("Shoot It!", "0.0", "io.github.Talon1024.ShootIt");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    uint32_t primaryDisplayId = getPrimaryDisplay();
    SDL_Rect screenRect;

    if (primaryDisplayId == 0) {
        SDL_Log("No display found!");
        return SDL_APP_FAILURE;
    }

    if (!SDL_GetDisplayBounds(primaryDisplayId, &screenRect)) {
        SDL_Log("Could not get primary display size.");
    }

    if (!SDL_CreateWindowAndRenderer("Shoot It!", screenRect.w, screenRect.h, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_SetRenderLogicalPresentation(renderer, VIEW_WIDTH, VIEW_HEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX);
    SDL_SetDefaultTextureScaleMode(renderer, SDL_SCALEMODE_PIXELART);
    SDL_HideCursor();

    queue = SDL_CreateAsyncIOQueue();
    if (!queue) {
        SDL_Log("Couldn't create async i/o queue: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    for (const char* asset : assets) {
        if (!loadAssetAsync(asset)) {
            return SDL_APP_FAILURE;
        }
    }

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

uint32_t getPrimaryDisplay() {
    int numDisplays;
    uint32_t primaryDisplayId = 0;
    uint32_t* displays = SDL_GetDisplays(&numDisplays);
    if (numDisplays > 0) {
        primaryDisplayId = *displays; // Deref (copy) the first element.
    }
    SDL_free(displays);
    return primaryDisplayId;
}

SDL_Texture* loadAsset(SDL_Renderer* renderer, const char* fname) {
    char* fullpath;
    SDL_asprintf(&fullpath, "%s/%s", SDL_GetBasePath(), fname);
    SDL_Surface* surf = SDL_LoadPNG(fullpath);
    SDL_free(fullpath);
    if (!surf) { return nullptr; }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_DestroySurface(surf);
    if (!tex) { return nullptr; }
    return tex;
}

bool loadAssetAsync(const char* asset) {
    char* fullpath;
    SDL_asprintf(&fullpath, "%s/%s", SDL_GetBasePath(), asset);
    bool result = SDL_LoadFileAsync(fullpath, queue, nullptr);
    if (!result) {
        SDL_Log("Unable to load asset %s: %s", asset, SDL_GetError());
    }
    SDL_free(fullpath);
    return result;
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS; // end the program, reporting success to the OS.
    }
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once per frame, and is the heart of the program.
 SAFETY: The renderer is most likely initialized.
 */
SDL_AppResult SDL_AppIterate(void* appstate)
{
    if (!SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255)) {
        SDL_Log("Could not set clear colour. %s", SDL_GetError());
    }
    SDL_MouseButtonFlags mouseButtons;
    SDL_AsyncIOOutcome outcome;

    if (SDL_GetAsyncIOResult(queue, &outcome)) {
        //  
    }

    float cursorX, cursorY;
    mouseButtons = SDL_GetMouseState(&cursorX, &cursorY);
    if (!SDL_RenderCoordinatesFromWindow(renderer, cursorX, cursorY, &cursorX, &cursorY)) {
        SDL_Log("ERROR: %s", SDL_GetError());
    }
    // For the rectangle that flashes during the loading sequence
    // ==========================================================
    // Size: 20x20
    //
    // X position:
    // 240 / 2 = 120
    // 120 - 10 = 110
    //
    // Y position:
    // 160 * (3/4) = 120
    // 120 - 10 = 110
    SDL_FRect loadRect {110., 110., 20., 20.};
    // SDL_FRect loadRect {cursorX - 10.f, cursorY - 10.f, 20., 20.};
    if (gameState == GameState::Loading) {
        SDL_RenderClear(renderer);
        // SDL_GetTicks() returns ms since program start
        // 500 ms is half a second
        // Maximum of any int % 500 is 499.
        // 499 >> 1 = 249
        // 255 - 249 = 6
        uint8_t shade = (uint8_t)((SDL_GetTicks() % 500) >> 1) + 6;
        SDL_SetRenderDrawColor(renderer, shade, shade, shade, 255);
        SDL_RenderFillRect(renderer, &loadRect);
    }
    // SDL_RenderTexture(renderer, backgroundTexture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us. */
}
