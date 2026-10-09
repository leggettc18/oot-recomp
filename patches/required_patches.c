#include "patches.h"
#include "misc_funcs.h"

#include "libu64/overlay.h"

extern u8 _codeSegmentStart[];
extern u8 _codeSegmentRomStart[];
extern u8 _codeSegmentRomEnd[];
extern u8 _codeSegmentBssStart[];
extern u8 _codeSegmentBssEnd[];

void DmaMgr_Init(void);
void DmaMgr_RequestSync(void* ram, uintptr_t vrom, size_t size);
void Main(void* arg);

RECOMP_PATCH void Main_ThreadEntry(void* arg) {
  size_t code_size = (size_t)(_codeSegmentRomEnd - _codeSegmentRomStart);
  DmaMgr_Init();
  // @recomp Load the code segment in the recomp runtime.
  recomp_load_overlays((u32)_codeSegmentRomStart, _codeSegmentStart, code_size);
  DmaMgr_RequestSync(_codeSegmentStart, (uintptr_t)_codeSegmentRomStart, code_size);
  bzero(_codeSegmentBssStart, (size_t)(_codeSegmentBssEnd - _codeSegmentBssStart));
  Main(arg);
}

RECOMP_PATCH size_t Overlay_Load(uintptr_t vromStart, uintptr_t vromEnd, void* vramStart, void* vramEnd, void* allocatedRamAddr) {
  s32 size = vromEnd - vromStart;
  uintptr_t end;
  OverlayRelocationSection* ovlRelocs;

  // @recomp Load the code segment in the recomp runtime.
  recomp_load_overlays((u32)vromStart, allocatedRamAddr, (u32)size);
  end = (uintptr_t)allocatedRamAddr + size;
  DmaMgr_RequestSync(allocatedRamAddr, vromStart, size);
  ovlRelocs = (OverlayRelocationSection*)(end - ((s32*)end)[-1]);
  Overlay_Relocate(allocatedRamAddr, ovlRelocs, vramStart);
  if ((s32)ovlRelocs->bssSize != 0) {
    bzero((void*)end, (s32)ovlRelocs->bssSize);
  }
  size = (uintptr_t)vramEnd - (uintptr_t)vramStart;
  osWritebackDCache(allocatedRamAddr, size);
  osInvalICache(allocatedRamAddr, size);

  return size;
}

extern u8 D_80121210;
extern u8 D_80121211;

RECOMP_PATCH void func_800AD410(void) {
  D_80121210 = true; // the check has been performed
  D_80121211 = false; // and there is no drive
}
