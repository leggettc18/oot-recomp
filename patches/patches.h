#ifndef PATCHES_H
#define PATCHES_H

#define bzero bzero_recomp
#define osWritebackDCache osWritebackDCache_recomp
#define osInvalICache osInvalICache_recomp
#define osGetTime osGetTime_recomp
#define osSendMesg osSendMesg_recomp

#include "patch_helpers.h"

#define RECOMP_PATCH __attribute__((section(".recomp_patch")))
#define RECOMP_FORCE_PATCH __attribute__((section(".recomp_force_patch")))
#define RECOMP_EXPORT __attribute__((section(".recomp_export")))

DECLARE_FUNC(void, recomp_puts, const char* str, u32 size);
DECLARE_FUNC(void, recomp_exit, void);

#endif // PATCHES_H
