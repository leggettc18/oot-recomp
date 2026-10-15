#include "play_patches.h"
#include "patch_helpers.h"

#include "regs.h"
#include "debug_display.h"

#define PLAY_LOG(line) (void)0

RECOMP_DECLARE_EVENT(recomp_on_play_main(PlayState* play));
RECOMP_DECLARE_EVENT(recomp_on_play_update(PlayState* play));
RECOMP_DECLARE_EVENT(recomp_after_play_update(PlayState* play));

void Play_Update(PlayState* play);
void Play_Draw(PlayState* play);

extern Input* D_8012D1F8;

RECOMP_PATCH void Play_Main(GameState* thisx) {
    PlayState* this = (PlayState*)thisx;

    // @recomp_event recomp_on_play_main(PlayState* play): Allow mods to execute code every frame.
    recomp_on_play_main(this);

    // @recomp


    D_8012D1F8 = &this->state.input[0];

    DebugDisplay_Init();

    PLAY_LOG(4556);

    if (DEBUG_FEATURES && (R_HREG_MODE == HREG_MODE_PLAY) && (R_PLAY_INIT != HREG_MODE_PLAY)) {
        R_PLAY_RUN_UPDATE = true;
        R_PLAY_RUN_DRAW = true;
        R_PLAY_DRAW_SKYBOX = true;
        R_PLAY_DRAW_ROOM_FLAGS = (ROOM_DRAW_OPA | ROOM_DRAW_XLU);
        R_PLAY_DRAW_ACTORS = true;
        R_PLAY_DRAW_LENS_FLARES = true;
        R_PLAY_DRAW_SCREEN_FILLS = true;
        R_PLAY_DRAW_SANDSTORM = true;
        R_PLAY_DRAW_OVERLAY_ELEMENTS = true;
        R_PLAY_DRAW_ENV_FLAGS = (PLAY_ENV_DRAW_SKYBOX_FILTERS | PLAY_ENV_DRAW_SUN_AND_MOON | PLAY_ENV_DRAW_LIGHTNING |
                                 PLAY_ENV_DRAW_LIGHTS);
        HREG(91) = 1; // reg is not used in this mode
        R_PLAY_DRAW_COVER_ELEMENTS = true;
        R_PLAY_DRAW_DEBUG_OBJECTS = true;
        R_PLAY_INIT = HREG_MODE_PLAY;
    }

    if (!DEBUG_FEATURES || (R_HREG_MODE != HREG_MODE_PLAY) || R_PLAY_RUN_UPDATE) {
        camera_pre_play_update(this);

        // @recomp_event recomp_on_play_update(PlayState* play): Play_Update is about to be called.
        recomp_on_play_update(this);

        Play_Update(this);

        // @recomp_event recomp_after_play_update(PlayState* play): Play_Update was called.
        recomp_after_play_update(this);

        camera_post_play_update(this);
    }

    PLAY_LOG(4583);

    Play_Draw(this);

    PLAY_LOG(4587);
}
