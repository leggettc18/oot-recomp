#ifndef __PLAY_PATCHES_H__
#define __PLAY_PATCHES_H__

#include "patches.h"
#include "play_state.h"

void camera_pre_play_update(PlayState* play);
void camera_post_play_update(PlayState* play);

#endif // __PLAY_PATCHES_H__
