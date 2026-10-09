#include "patches.h"
#include "camera.h"
#include "play_state.h"
#include "room.h"

DECLARE_FUNC(void, recomp_prerendered_room, s32 on, s32 cam_setting);

void Play_Destroy(GameState* thisx);

void prerender_frame_begin(GameState* gameState) {
  PlayState* play;
  Room* room;
  Camera* cam;
  s32 prerendered = 0;
  s32 camSetting = -1;

  if (gameState->destroy == Play_Destroy) {
    play = (PlayState*)gameState;
    room = &play->roomCtx.curRoom;

    if ((room->roomShape != NULL) && (room->roomShape->base.type == ROOM_SHAPE_TYPE_IMAGE)) {
      prerendered = 1;
    }

    if ((play->activeCamId >= 0) && (play->activeCamId < NUM_CAMS)) {
      cam = play->cameraPtrs[play->activeCamId];
      if (cam != NULL) {
        camSetting = cam->setting;
        if ((camSetting == CAM_SET_PREREND_FIXED) || (camSetting == CAM_SET_PREREND_PIVOT) ||
            (camSetting == CAM_SET_PREREND_SIDE_SCROLL)) {
          prerendered = 1;
        }
      }
    }
  }

  recomp_prerendered_room(prerendered, camSetting);
}
