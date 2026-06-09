#!/usr/bin/env zsh

great_great_assets=(assets/**/*.png(.))  # P.S. I don't play Lethal Company ;P

# struct PendingExternalAsset {
#     const char* fname;
#     SDL_AsyncIOOutcome outcome;
# };

cat<<ASSET_HEADER>src/assets.h
#define ASSET_COUNT ${#great_great_assets}
static SDL_Texture* textures[ASSET_COUNT];
static const char* assets[ASSET_COUNT] = {
$(print ${(F)great_great_assets[@]} | sed -E 's@^(.+)$@    "\1",@g')
};
ASSET_HEADER
# $(print ${(F)great_great_assets[@]} | sed -E 's@^(.+)$@    {"\1", {}},@g')
