/*
 * Shoot It (c) Talon1024 2026-present
 *
 * GPLv3 License
 */

#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <cstdint>
#include <cstring>
#include <emscripten.h>

#define VIEW_WIDTH 240
#define VIEW_HEIGHT 160

enum class LoadState {
    Loading,
    Failure,
    Success,
    PostSuccess, // So that the 'ready' message is only sent once.
};

enum class GameState {
    WaitingToStart,
    Play,
    Paused, // Needed?
    Win,
    Loss,
};

/* We will use this renderer to draw into this window every frame. */
static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;
static SDL_AsyncIOQueue* queue = nullptr;

// Hopefully prevent cache misses when drawing text?
static struct FontCharInfo {
    size_t asset;
    float width;
    float height;
    float yoffset;
} charInfo[256] = {};

#include "assets.h"

uint32_t getPrimaryDisplay();
SDL_Texture* loadAsset(SDL_Renderer* renderer, const char* fname);
bool loadAssetAsync(const char* asset, uint32_t index);

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

    {
        uint32_t assetIndex = 0;
        for (const char* asset : assets) {
            if (!loadAssetAsync(asset, assetIndex++)) {
                return SDL_APP_FAILURE;
            }
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

bool loadAssetAsync(const char* asset, uint32_t index) {
    char* fullpath;
    SDL_asprintf(&fullpath, "%s/%s", SDL_GetBasePath(), asset);
    // It's much faster to store the asset index, since, that way, one can refer
    // directly to its slot instead of searching for it.
    bool result = SDL_LoadFileAsync(fullpath, queue, new uint32_t {index});
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

int32_t getPNGYOffset(void* bufdata, size_t bufsize) {
    // Search for the grAb chunk - the sprite offsets for ZDoom PNGs.
    char* idatStart = (char*) memmem(
        bufdata, bufsize,
        "IDAT", 4
    );
    if (!idatStart) {
        SDL_Log("Not a valid PNG!");
        return 0;
    }
    size_t grabHaystackSize = idatStart - (char*) bufdata;
    // grAb comes before IDAT
    char* grabStart = (char*) memmem(
        bufdata, grabHaystackSize,
        "grAb", 4
    );
    if (!grabStart) {
        return 0;
    }
    int32_t* offsets = (int32_t*) grabStart + 1;
    int32_t offsetX = SDL_Swap32BE(*offsets);
    int32_t offsetY = SDL_Swap32BE(*(offsets+1));
    return -offsetY; // negative Y offset = down
}

#define SPACE_WIDTH 5.0

void drawText(const char* text, float x, float y) {
    size_t textLength = std::strlen(text);
    SDL_FRect charRect {x, y, 0.0, 0.0};
    for (size_t pos = 0; pos < textLength; pos++) {
        char curChar = text[pos];
        // Space to next character - 5 if space, width + 1 for kerning.
        size_t assetIndex = charInfo[curChar].asset;
        if (curChar == ' ') {
            charRect.x += SPACE_WIDTH;
            continue;
        } else if (assetIndex == 0) { continue; }
        charRect.w = charInfo[curChar].width;
        charRect.h = charInfo[curChar].height;
        charRect.y = y + charInfo[curChar].yoffset;
        SDL_RenderTexture(renderer, textures[assetIndex], nullptr, &charRect);
        charRect.x += charInfo[curChar].width + 1.0; // 1 pixel for kerning
    }
}

EM_JS(void, signalReady, (float& difficulty), {
    window.parent.postMessage({op: "ready"});
    window.addEventListener("message", ev => {
        console.log(ev.data);
        if (difficulty in ev) {
            difficulty = ev.difficulty;
        }
    });
});

EM_JS(void, signalDone, (bool won), {
    window.parent.postMessage({op: "done", win: won});
})

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
    // The "static" keyword makes these variables global, but only accessible
    // within the scope of this function.
    static uint32_t assetsLoaded = 0;
    static LoadState loadState = LoadState::Loading;
    static GameState gameState = GameState::WaitingToStart;
    static float gameDifficulty = -1.0; // Negative = uninitialized.

    if (SDL_GetAsyncIOResult(queue, &outcome)) {
        uint32_t assetIndex = *(uint32_t*)outcome.userdata;
        delete (uint32_t*) outcome.userdata;
        if (outcome.result == SDL_ASYNCIO_COMPLETE) {
            // Set up font character
            uint32_t width = SDL_Swap32BE(*((uint32_t*)outcome.buffer + 4));
            uint32_t height = SDL_Swap32BE(*((uint32_t*)outcome.buffer + 5));
            int32_t yoffset = getPNGYOffset(outcome.buffer, (size_t) outcome.bytes_transferred);
            charInfo[asset_to_char[assetIndex]].asset = assetIndex;
            charInfo[asset_to_char[assetIndex]].width = (float) width;
            charInfo[asset_to_char[assetIndex]].height = (float) height;
            charInfo[asset_to_char[assetIndex]].yoffset = (float) yoffset;
            SDL_Surface* surf = SDL_LoadPNG_IO(SDL_IOFromConstMem(
                outcome.buffer, (size_t) outcome.bytes_transferred
            ), true);
            if (surf) {
                textures[assetIndex] = SDL_CreateTextureFromSurface(renderer, surf);
                if (!textures[assetIndex]) {
                    SDL_Log("Couldn't create texture! %s", SDL_GetError());
                    loadState = LoadState::Failure;
                }
                SDL_DestroySurface(surf);
            } else {
                SDL_Log("Could not read PNG %s: %s", assets[assetIndex], SDL_GetError());
                loadState = LoadState::Failure;
            }
            SDL_free(outcome.buffer);
            assetsLoaded++;
        } else if (outcome.result == SDL_ASYNCIO_FAILURE) {
            SDL_Log("Could not load asset %s: %s", assets[assetIndex], SDL_GetError());
            loadState = LoadState::Failure;
        }
        if (assetsLoaded == TOTAL_ASSET_COUNT) {
            // finished loading!
            loadState = LoadState::Success;
        }
    }

    float cursorX, cursorY;
    mouseButtons = SDL_GetMouseState(&cursorX, &cursorY);
    if (!SDL_RenderCoordinatesFromWindow(
        renderer, cursorX, cursorY, &cursorX, &cursorY)) {
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
    uint8_t shade;
    switch (loadState) {
    case LoadState::Loading:
        SDL_RenderClear(renderer);
        // SDL_GetTicks() returns ms since program start
        // 500 ms is half a second
        // Maximum of any int % 500 is 499.
        // 499 >> 1 = 249
        // 255 - 249 = 6
        shade = (uint8_t)((SDL_GetTicks() % 500) >> 1) + 6;
        // Assets (like font characters) are not loaded yet, so just show a
        // square as a placeholder.
        SDL_SetRenderDrawColor(renderer, shade, shade, shade, 255);
        SDL_RenderFillRect(renderer, &loadRect);
        break;
    case LoadState::Failure:
        SDL_RenderClear(renderer);
        // Red square
        SDL_SetRenderDrawColor(renderer, 180, 0, 0, 255);
        SDL_RenderFillRect(renderer, &loadRect);
        break;
    case LoadState::Success:
        signalReady(gameDifficulty);
        SDL_SetRenderDrawColor(renderer, 0, 180, 0, 255);
        SDL_RenderFillRect(renderer, &loadRect);
        loadState = LoadState::PostSuccess;
        break;
    case LoadState::PostSuccess:
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        switch (gameState) {
        case GameState::WaitingToStart:
            SDL_RenderTexture(renderer, textures[0], nullptr, nullptr);
            drawText("Get Ready...", 90, 77);
            break;
        case GameState::Play:
            SDL_RenderTexture(renderer, textures[0], nullptr, nullptr);
            drawText("Shoot!", 90, 77);
            break;
        case GameState::Win:
            signalDone(true);
            break;
        case GameState::Loss:
            signalDone(false);
            break;
        case GameState::Paused:
            // Draw everything drawn in the 'play' state, and then "Paused" on
            // top.
            break;
        }
        break;
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
