__start = 0x80000000;

/* Dummy addresses that get recompiled into function calls */
recomp_puts = 0x8F000000;
recomp_exit = 0x8F000004;

recomp_load_overlays = 0x8F000090;

bzero_recomp = 0x8F00005C;
osWritebackDCache_recomp = 0x8F000068;
osInvalICache_recomp = 0x8F000094;
osGetTime_recomp = 0x8F00009C;
osCreateMesgQueue_recomp = 0x8F0000CC;
osRecvMesg_recomp = 0x8F0000D0;
osSetTimer_recomp = 0x8F0000D4;
osSendMesg_recomp = 0x8F00011C;

recomp_note_dropped_frame = 0x8F0000D8;
recomp_prerendered_room = 0x8F0000BC;
