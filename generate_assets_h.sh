#!/usr/bin/env zsh

gfx=(assets/gfx/*.png(.))
fontchars=(assets/minifont/*.png(.))
total_asset_count=$((${#gfx}+${#fontchars}))

cat<<ASSET_HEADER>src/assets.h
#define TOTAL_ASSET_COUNT ${total_asset_count}
static SDL_Texture* textures[TOTAL_ASSET_COUNT];
static const char* assets[TOTAL_ASSET_COUNT] = {
$(print ${(F)gfx[@]} | sed -E 's@^(.+)$@    "\1",@g')
$(print ${(F)fontchars[@]} | sed -E 's@^(.+)$@    "\1",@g')
};
static const size_t char_to_asset[256] = {
$(for byte in {0..255}; do printf -v fname assets/minifont/%04X.png ${byte}; if [[ -f ${fname} ]] { print -n "$((${#gfx}+${fontchars[(i)${fname}]}-1))" } else { print -n '0' }; print ','; done)
};
static const unsigned char asset_to_char[TOTAL_ASSET_COUNT] = {
$(for graphic in $gfx; do print '0,'; done)
$(for fontchar in $fontchars; do print "$(( 0x${fontchar:16:4} )),"; done)
};
ASSET_HEADER
