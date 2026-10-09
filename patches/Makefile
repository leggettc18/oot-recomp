TARGET = patches.elf
BINARY = patches.bin

CC ?= clang
LD ?= ld.lld

CFLAGS := -target mips -mips2 -mabi=32 -O2 -G0 -mno-abicalls -mno-odd-spreg -mno-check-zero-division \
					-fomit-frame-pointer -ffast-math -fno-unsafe-math-optimizations -fno-builtin-memset \
					-Wall -Wextra -Wno-incompatible-library-redeclaration -Wno-unused-parameter \
					-Wno-unknown-pragmas -Wno-unused-variable -Wno-missing-braces -Wno-unsupported-floating-point-opt
CPPFLAGS := -nostdinc -D_LANGUAGE_C -DMIPS -DOOT_VERSION=NTSC_1_0 -DOOT_REVISION=0 -DOOT_REGION=REGION_US -DPLATFORM_N64=1 -DPLATFORM_GC=0 -DPLATFORM_IQUE=0 -DLIBULTRA_VERSION=LIBULTRA_VERSION_I -DLIBULTRA_PATCH=1 -DDEBUG_FEATURES=0 -DNDEBUG -DF3DEX_GBI_2 -I dummy_headers -I ../lib/oot/include -I ../lib/oot/include/libc -I ../lib/oot/src -I ../lib/oot/assets -I ../lib/oot/include/audio -I ../lib/oot/include/tables -I ../lib/oot -I ../lib/rt64/include
LDFLAGS := -nostdlib -T patches.ld -T syms.ld -Map patches.map --unresolved-symbols=ignore-all --emit-relocs

C_SRCS := $(wildcard *.c)
C_OBJS := $(C_SRCS:.c=.o)
C_DEPS := $(C_SRCS:.c=.d)

all: $(TARGET)

$(TARGET): $(C_OBJS) patches.ld syms.ld
	$(LD) $(C_OBJS) $(LDFLAGS) -o $@

$(C_OBJS): %.o : %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $< -MMD -MF $(@:.o=.d) -c -o $@

clean:
	rm -rf $(C_OBJS) $(TARGET) $(C_DEPS) $(BINARY) patches.map

-include $(C_DEPS)

.PHONY: all clean
