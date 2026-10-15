#include "patches.h"
#include "transform_ids.h"
#include "cutscene.h"
#include "overlays/misc/ovl_kaleido_scope/z_kaleido_scope.h"
#include "overlays/gamestates/ovl_file_choose/file_select.h"
#include "play_state.h"
#include "gfx.h"
#include "camera.h"
#include "file_select_state.h"
#include "olib.h"
#include "z_lib.h"
#include "camera_patches.h"

#include "rt64_extended_gbi.h"

static bool camera_interpolation_forced = false;
static bool camera_skip_interpolation_forced = false;
static bool camera_ignore_tracking = false;
static bool in_kaleido = false;
static bool prev_in_kaleido = false;
static bool camera_skipped = false;

void set_camera_skipped(bool skipped) {
  camera_skipped = skipped;
}

void clear_camera_skipped() {
  camera_skipped = false;
}

bool camera_was_skipped() {
  return camera_skipped;
}

void camera_pre_play_update(PlayState* play) {}

void camera_post_play_update(PlayState* play) {
  // Track whether kaleido menu is up.
  prev_in_kaleido = in_kaleido;

  if ((play->pauseCtx.state != 0) || (play->pauseCtx.debugState != PAUSE_DEBUG_STATE_CLOSED)) {
    in_kaleido = true;
  } else {
    in_kaleido = false;

    if (play->activeCamId >= 0 && play->activeCamId < NUM_CAMS) {
      Camera *active_cam = play->cameraPtrs[play->activeCamId];
      // Workarounds for undetected large camera movements.
      bool force_interpolation = false;
      if (active_cam->setting == CAM_SET_BOSS_GANON) {
        force_interpolation = true;
      } else if (active_cam->setting == CAM_SET_NORMAL0 || active_cam->setting == CAM_SET_DUNGEON0) {
        force_interpolation =
          active_cam->mode == CAM_MODE_Z_TARGET_FRIENDLY ||
          active_cam->mode == CAM_MODE_Z_PARALLEL ||
          active_cam->mode == CAM_MODE_Z_TARGET_UNFRIENDLY;
      } // TODO: Find other scenarios where we need to force camera interpolation.

      if (force_interpolation) {
        force_camera_interpolation();
      }
      bool force_no_interpolation = false;

      // TODO: Workarounds for undetected small camera teleports
      
      if (force_no_interpolation) {
        force_camera_skip_interpolation();
      }
    }
  }
}

s32 View_ApplyPerspective(View* view);
s32 View_ApplyOrtho(View* view);

void force_camera_interpolation() {
  camera_interpolation_forced = true;
}

void force_camera_skip_interpolation() {
  camera_skip_interpolation_forced = true;
}

void force_camera_ignore_tracking() {
  camera_ignore_tracking = true;
}

RECOMP_PATCH void KaleidoScope_SetView(PauseContext* pauseCtx, f32 x, f32 y, f32 z) {
    Vec3f eye;
    Vec3f lookAt;
    Vec3f up;

    eye.x = x;
    eye.y = y;
    eye.z = z;
    lookAt.x = lookAt.y = lookAt.z = 0.0f;
    up.x = up.z = 0.0f;
    up.y = 1.0f;

    // @recomp Force interpolation for this view and skip tracking positions.
    force_camera_interpolation();
    force_camera_ignore_tracking();

    View_LookAt(&pauseCtx->view, &eye, &lookAt, &up);
    View_Apply(&pauseCtx->view,
               VIEW_ALL | VIEW_FORCE_VIEWING | VIEW_FORCE_VIEWPORT | VIEW_FORCE_PROJECTION_PERSPECTIVE);
}

RECOMP_PATCH void FileSelect_SetView(FileSelectState* this, f32 eyeX, f32 eyeY, f32 eyeZ) {
    Vec3f eye;
    Vec3f lookAt;
    Vec3f up;

    eye.x = eyeX;
    eye.y = eyeY;
    eye.z = eyeZ;

    lookAt.x = lookAt.y = lookAt.z = 0.0f;

    up.x = up.z = 0.0f;
    up.y = 1.0f;

    // @recomp Force interpolation for this view and skip tracking positions.
    force_camera_interpolation();
    force_camera_ignore_tracking();

    View_LookAt(&this->view, &eye, &lookAt, &up);
    View_Apply(&this->view, VIEW_ALL | VIEW_FORCE_VIEWING | VIEW_FORCE_VIEWPORT | VIEW_FORCE_PROJECTION_PERSPECTIVE);
}

bool should_interpolate_perspective(Vec3f* eye, Vec3f* at) {
  static Vec3f prev_eye = {0,0,0};
  static Vec3f prev_at = {0,0,0};
  static Vec3f eye_velocity = {0,0,0};
  static Vec3f at_velocity = {0,0,0};

  Vec3f predicted_eye;
  Vec3f predicted_at;

  Math_Vec3f_Sum(&prev_eye, &eye_velocity, &predicted_eye);
  Math_Vec3f_Sum(&prev_at, &at_velocity, &predicted_at);

  Math_Vec3f_Diff(eye, &prev_eye, &eye_velocity);
  Math_Vec3f_Diff(at, &prev_at, &at_velocity);

  float eye_dist = Math_Vec3f_DistXYZ(&predicted_eye, eye);
  float at_dist = Math_Vec3f_DistXYZ(&predicted_at, at);

  float velocity_diff = Math_Vec3f_DistXYZ(&eye_velocity, &at_velocity);

  prev_eye = *eye;
  prev_at = *at;

  // If the velocity of both positions was the same, then they're moving together and should interpolate.
  if (velocity_diff <= 3.0f && eye_dist <= 100.0f && at_dist <= 100.0f) {
    return true;
  }

  // If the focus or position are basically the same across frames and the eye didn't move too far then it should probably be interpolated.
  if (at_dist <= 10.0f && eye_dist <= 300.0f) {
    return true;
  }
  if (eye_dist <= 10.0f && at_dist <= 300.0f) {
    return true;
  }

  if (velocity_diff > 50.0f || at_dist > 50.0f || eye_dist > 300.0f) {
    eye_velocity.x = 0.0f;
    eye_velocity.y = 0.0f;
    eye_velocity.z = 0.0f;
    at_velocity.x = 0.0f;
    at_velocity.y = 0.0f;
    at_velocity.z = 0.0f;
    return false;
  }

  return true;
}

RECOMP_PATCH s32 View_Apply(View* view, s32 mask) {
    mask = (view->flags & mask) | (mask >> 4);

    // @recomp Determine if the camera should be interpolated this frame.
    bool interpolate_camera = false;

    if (mask & VIEW_PROJECTION_ORTHO) {
        View_ApplyOrtho(view);
    } else {
        View_ApplyPerspective(view);

        // @recomp Determine if interpolation should occur based on the new eye and at positions.
        if (!camera_ignore_tracking) {
          interpolate_camera = should_interpolate_perspective(&view->eye, &view->at);
        }
    }

    camera_ignore_tracking = false;

    // Force skipping interpolation if the previous view was kaleido and this one isn't.
    if (prev_in_kaleido && !in_kaleido) {
      camera_skip_interpolation_forced = true;
    }

    // @recomp Apply camera interpolation overrides.
    if (camera_skip_interpolation_forced) {
      interpolate_camera = false;
    } else if (camera_interpolation_forced) {
      interpolate_camera = true;
    }

    // @recomp Tag the camera matrices
    GraphicsContext* gfxCtx = view->gfxCtx;
    OPEN_DISPS(gfxCtx, "../z_view.c", 0);

    if (interpolate_camera) {
      // Simple interpolation works much better for cameras because they orbit around a focus.
      gEXMatrixGroupSimple(POLY_OPA_DISP++, CAMERA_TRANSFORM_ID, G_EX_NOPUSH, G_MTX_PROJECTION,
          G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE,
          G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_ORDER_LINEAR, G_EX_EDIT_NONE,
          G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE);
      gEXMatrixGroupSimple(POLY_XLU_DISP++, CAMERA_TRANSFORM_ID, G_EX_NOPUSH, G_MTX_PROJECTION,
          G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE,
          G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE, G_EX_ORDER_LINEAR, G_EX_EDIT_NONE,
          G_EX_COMPONENT_INTERPOLATE, G_EX_COMPONENT_INTERPOLATE);
    } else {
      // Skip interpolation for this frame.
      gEXMatrixGroupSimple(POLY_OPA_DISP++, CAMERA_TRANSFORM_ID, G_EX_NOPUSH, G_MTX_PROJECTION,
          G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP,
          G_EX_COMPONENT_INTERPOLATE, G_EX_ORDER_LINEAR, G_EX_EDIT_NONE, G_EX_COMPONENT_SKIP,
          G_EX_COMPONENT_SKIP);
      gEXMatrixGroupSimple(POLY_XLU_DISP++, CAMERA_TRANSFORM_ID, G_EX_NOPUSH, G_MTX_PROJECTION,
          G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP, G_EX_COMPONENT_SKIP,
          G_EX_COMPONENT_INTERPOLATE, G_EX_ORDER_LINEAR, G_EX_EDIT_NONE, G_EX_COMPONENT_SKIP,
          G_EX_COMPONENT_SKIP);
    }

    // Record whether the camera was skipped for later use.
    set_camera_skipped(!interpolate_camera);

    camera_interpolation_forced = false;
    camera_skip_interpolation_forced = false;

    CLOSE_DISPS(gfxCtx, "../z_view.c", 0);
}

typedef s32 (*CameraUpdateFunc)(Camera*);

typedef struct CameraModeValue {
    s16 val;
    s16 dataType;
} CameraModeValue;

typedef struct CameraMode {
    s16 funcIdx;
    s16 valueCnt;
    CameraModeValue* values;
} CameraMode;

typedef struct CameraSetting {
    union {
        u32 unk_00;
        struct {
            u32 unk_bit0 : 1;
            u32 unk_bit1 : 1;
            u32 validModes : 30;
        };
    };
    CameraMode* cameraModes;
} CameraSetting;

extern CameraSetting sCameraSettings[];
extern s32 sCameraInterfaceField;

s32 Camera_BgCheck(Camera* camera, Vec3f* from, Vec3f* to);
Vec3s* Camera_GetBgCamFuncData(Camera* camera);
Vec3s* Camera_GetBgCamOrActorCsCamFuncData(Camera* camera, u32 camDataId);
f32 Camera_GetFocalActorHeight(Camera* camera);
f32 Camera_ScaledStepToCeilF(f32 target, f32 cur, f32 stepScale, f32 minDiff);
s16 Camera_ScaledStepToCielS(s16 target, s16 cur, f32 stepScale, s16 minDiff);
void Camera_ScaledStepToCeilVec3f(Vec3f* target, Vec3f* cur, f32 xzStepScale, f32 yStepScale, f32 minDiff);
void Camera_SetFocalActorAtOffset(Camera* camera, Vec3f* focalActorPos);
void Camera_SetUpdateRatesSlow(Camera* camera);
Vec3f Camera_Vec3sToVec3f(Vec3s* src);
void func_80043B60(Camera* camera);
f32 Camera_LERPCeilF(f32 target, f32 cur, f32 stepScale, f32 minDiff);
void Camera_LERPCeilVec3f(Vec3f* target, Vec3f* cur, f32 yStepScale, f32 xzStepScale, f32 minDiff);
s16 Camera_LERPCeilS(s16 target, s16 cur, f32 stepScale, s16 minDiff);
Vec3f Camera_AddVecGeoToVec3f(Vec3f* a, VecGeo* geo);

// @recomp Modified to not force interpolation while panning.
RECOMP_PATCH s32 Camera_Fixed1(Camera* camera) {
    Fixed1ReadOnlyData* roData = &camera->paramData.fixd1.roData;
    Fixed1ReadWriteData* rwData = &camera->paramData.fixd1.rwData;
    s32 pad;
    VecGeo eyeOffset;
    VecGeo eyeAtOffset;
    s32 pad2;
    Vec3f adjustedPos;
    BgCamFuncData* bgCamFuncData;
    Vec3f* eye = &camera->eye;
    Vec3f* at = &camera->at;
    PosRot* playerPosRot = &camera->playerPosRot;
    f32 playerHeight;

    playerHeight = Player_GetHeight(camera->player);
    if (RELOAD_PARAMS(camera) || CAM_DEBUG_RELOAD_PARAMS) {
        CameraModeValue* values = sCameraSettings[camera->setting].cameraModes[camera->mode].values;

        bgCamFuncData = (BgCamFuncData*)Camera_GetBgCamFuncData(camera);
        rwData->eyePosRotTarget.pos = Camera_Vec3sToVec3f(&bgCamFuncData->pos);
        rwData->eyePosRotTarget.rot = bgCamFuncData->rot;
        rwData->fov = bgCamFuncData->fov;

        roData->unk_00 = GET_NEXT_SCALED_RO_DATA(values) * playerHeight;
        roData->lerpStep = GET_NEXT_SCALED_RO_DATA(values);
        roData->fov = GET_NEXT_RO_DATA(values);
        roData->interfaceField = GET_NEXT_RO_DATA(values);
    }

    CAM_DEBUG_RELOAD_PREG(camera);
    if (rwData->fov == -1) {
        rwData->fov = roData->fov * 100.0f;
    } else if (rwData->fov <= 360) {
        rwData->fov *= 100;
    }

    sCameraInterfaceField = roData->interfaceField;

    if (camera->animState == 0) {
        camera->animState++;
        func_80043B60(camera);
        if (rwData->fov != -1) {
            roData->fov = CAM_DATA_SCALED(rwData->fov);
        }
    }

    // @recomp Camera Interpolation should always apply on this mode unless something else modified it externally.
    // We check for the approach percentage as well to detect when it wants to force an interpolation to the next position.
    static Vec3f lastEye = {};
    static Vec3f lastAt = {};
    if (OLib_Vec3fDist(eye, &lastEye) < 1e-6f && OLib_Vec3fDist(at, &lastAt) < 1e-6f && (roData->lerpStep < 0.999f)) {
      force_camera_interpolation();
    }

    eyeAtOffset = OLib_Vec3fDiffToVecGeo(eye, at);

    Camera_LERPCeilVec3f(&rwData->eyePosRotTarget.pos, eye, 0.1f, 0.1f, 0.2f);
    adjustedPos = playerPosRot->pos;
    adjustedPos.y += playerHeight;
    camera->dist = OLib_Vec3fDist(&adjustedPos, eye);

    eyeOffset.r = camera->dist;
    eyeOffset.pitch = Camera_LERPCeilS(-rwData->eyePosRotTarget.rot.x, eyeAtOffset.pitch, roData->lerpStep, 5);
    eyeOffset.yaw = Camera_LERPCeilS(rwData->eyePosRotTarget.rot.y, eyeAtOffset.yaw, roData->lerpStep, 5);

    *at = Camera_AddVecGeoToVec3f(eye, &eyeOffset);

    camera->eyeNext = *eye;

    camera->fov = Camera_LERPCeilF(roData->fov, camera->fov, roData->lerpStep, 0.01f);
    camera->roll = 0;
    camera->atLERPStepScale = 0.0f;

    camera->playerToAtOffset.x = camera->at.x - playerPosRot->pos.x;
    camera->playerToAtOffset.y = camera->at.y - playerPosRot->pos.y;
    camera->playerToAtOffset.z = camera->at.z - playerPosRot->pos.z;

    // @recomp
    lastEye = *eye;
    lastAt = *at;

    return true;
}
