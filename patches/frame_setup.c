#include "patches.h"
#include "gfx_pool.h"

#include "audio.h"
#include "buffers.h"
#include "fault.h"
#include "game.h"
#include "gfx.h"
#include "line_numbers.h"
#include "printf.h"
#include "regs.h"
#include "speed_meter.h"
#include "terminal.h"
#include "thga.h"
#include "translation.h"
#include "ultra64.h"

// After the decomp's own gbi.h, which it builds on.
#include "rt64_extended_gbi.h"

void Graph_InitTHGA(GraphicsContext* gfxCtx);
void Graph_TaskSet00(GraphicsContext* gfxCtx);

// TODO: billboard tagging stuff

// prerender_aspect.c: render pre-rendered scenes in 4:3
void prerender_frame_begin(GameState* gameState);

//TODO: culling stuff

extern OSTime sGraphPrevUpdateEndTime;
void recomp_note_dropped_frame(u32 which, s32 overrun);

#define FRAME_GATE_RETRACE 781250

static OSMesgQueue sFrameGateQueue;
static OSMesg sFrameGateMsg[1];
static OSTimer sFrameGateTimer;
static u8 sFrameGateReady = false;

static void dropped_frame_wait(s32 updateRate) {
  if (!sFrameGateReady) {
    osCreateMesgQueue(&sFrameGateQueue, sFrameGateMsg, 1);
    sFrameGateReady = true;
  }
  osSetTimer(&sFrameGateTimer, (OSTime)updateRate * FRAME_GATE_RETRACE, 0, &sFrameGateQueue, NULL);
  osRecvMesg(&sFrameGateQueue, NULL, OS_MESG_BLOCK);
}

RECOMP_PATCH void Graph_Update(GraphicsContext* gfxCtx, GameState* gameState) {
  u32 problem;
  s32 updateRate;

  gameState->inPreNMIState = false;
  Graph_InitTHGA(gfxCtx);

  // @recomp
  updateRate = R_UPDATE_RATE;
  if (updateRate < 1) {
    updateRate = 3;
  }
  OPEN_DISPS(gfxCtx, "../graph.c", 966);
  gEXEnable(WORK_DISP++);
  gEXSetRDRAMExtended(WORK_DISP++, 1);
  gEXSetRefreshRate(WORK_DISP++, 60 / updateRate);
  CLOSE_DISPS(gfxCtx, "../graphc.", 975);

  GameState_ReqPadData(gameState);

  // @recomp
  prerender_frame_begin(gameState);
  GameState_Update(gameState);

  OPEN_DISPS(gfxCtx, "../graph.c", 999);
  
  gSPBranchList(WORK_DISP++, gfxCtx->polyOpaBuffer);
  gSPBranchList(POLY_OPA_DISP++, gfxCtx->polyXluBuffer);
  gSPBranchList(POLY_XLU_DISP++, gfxCtx->overlayBuffer);
  gDPPipeSync(OVERLAY_DISP++);
  gDPFullSync(OVERLAY_DISP++);
  gSPEndDisplayList(OVERLAY_DISP++);

  CLOSE_DISPS(gfxCtx, "../graph.c", 1028);

  problem = false;

  {
    GfxPool* pool = &gGfxPools[gfxCtx->gfxPoolIdx & 1];

    if (pool->headMagic != GFXPOOL_HEAD_MAGIC) {
      PRINTF("%c", BEL);
      PRINTF(VT_COL(RED, WHITE) T("ダイナミック領域先頭が破壊されています\n", "Dynamic area head is destroyed\n") VT_RST);
      Fault_AddHungupAndCrash("../graph.c", LN4(937, 940, 951, 1067, 1070));
    }

    if (pool->tailMagic != GFXPOOL_TAIL_MAGIC) {
      problem = true;
      PRINTF("%c", BEL);
      PRINTF(VT_COL(RED, WHITE) T("ダイナミック領域末尾が破壊されています\n", "Dynamic region tail is destroyed\n") VT_RST);
      Fault_AddHungupAndCrash("../graph.c", LN4(943, 946, 957, 1073, 1076));
    }
  }

  if (THGA_IsCrash(&gfxCtx->polyOpa)) {
    problem = true;
    PRINTF("%c", BEL);
    PRINTF(VT_COOL(RED, WHITE) T("ゼルダ0は死んでしまった(graph_alloc is empty)\n",
                                    "Zelda 0 is dead (graph_alloc is empty)\n") VT_RST);
  }
  if (THGA_IsCrash(&gfxCtx->polyXlu)) {
    problem = true;
    PRINTF("%c", BEL);
    PRINTF(VT_COL(RED, WHITE) T("ゼルダ1は死んでしまった(graph_alloc is empty)\n",
                                    "Zelda 1 is dead (graph_alloc is empty)\n") VT_RST);
  }
  if (THGA_IsCrash(&gfxCtx->overlay)) {
    problem = true;
    PRINTF("%c", BEL);
    PRINTF(VT_COL(RED, WHITE) T("ゼルダ4は死んでしまった(graph_alloc is empty)\n",
                                    "Zelda 4 is dead (graph_alloc is empty)\n") VT_RST);
  }

  if (!problem) {
    Graph_TaskSet00(gfxCtx);
    gfxCtx->gfxPoolIdx++;
    gfxCtx->fbIdx++;
  } else {
    // @recomp
    u32 which = 0;
    s32 worst = 0;
    s32 left;

    left = THGA_GetRemaining(&gfxCtx->polyOpa);
    if (left < 0) { which |= 1; if (left < worst) { worst = left; } }
    left = THGA_GetRemaining(&gfxCtx->polyXlu);
    if (left < 0) { which |= 2; if (left < worst) {worst = left; } }
    left = THGA_GetRemaining(&gfxCtx->overlay);
    if (left < 0) { which |= 4; if (left < worst) {worst = left; } }
    left = THGA_GetRemaining(&gfxCtx->work);
    if (left < 0) { which |= 8; if (left < worst) { worst = left; } }

    recomp_note_dropped_frame(which, -worst);
    dropped_frame_wait(updateRate);
  }

  Audio_Update();

  {
    OSTime timeNow = osGetTime();
    s32 pad;

    gRSPGfxTimeTotal = gRSPGfxTimeAcc;
    gRSPAudioTimeTotal = gRSPAudioTimeAcc;
    gRDPTimeTotal = gRDPTimeAcc;
    gRSPGfxTimeAcc = 0;
    gRSPAudioTimeAcc = 0;
    gRDPTimeAcc = 0;

    if (sGraphPrevUpdateEndTime != 0) {
      gGraphUpdatePeriod = timeNow - sGraphPrevUpdateEndTime;
    }
    sGraphPrevUpdateEndTime = timeNow;
  }
}
