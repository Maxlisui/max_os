DIRECTORY_GUARD=mkdir -p $(@D)

END_PATH := ./kernel ./libs/libc ./libs/max_os_libs

BUILD_OUT := ./build

HFILES    := $(shell find $(END_PATH) -type f -name '*.h')

CFILES    := $(shell find $(END_PATH) -type f -name '*.c')
COBJFILES := $(patsubst %.c,$(BUILD_OUT)/%.o,$(CFILES))

CXXFILES    := $(shell find $(END_PATH) -type f -name '*.cpp')
CXXOBJFILES := $(patsubst %.cpp,$(BUILD_OUT)/%.o,$(CXXFILES))


LINK_PATH := ./kernel/linker.ld


CC         = ./toolchain/bin/x86_64-elf-gcc
CXX        = ./toolchain/bin/x86_64-elf-c++
LD         = ./toolchain/bin/x86_64-elf-ld

OBJ := $(shell find $(BUILD_OUT) -type f -name '*.o')

KERNEL_HDD = ./build/disk.hdd
KERNEL_ELF = ./build/max_os.elf

.DEFAULT_GOAL =$(KERNEL_HDD)
CHARDFLAGS := $(CFLAGS)                        \
	-DBUILD_TIME='"$(BUILD_TIME)"'         \
	-std=c11                               \
	-g                                     \
	-masm=intel                            \
	-fno-pic                               \
	-no-pie                                \
	-m64                                   \
	-Wall                                  \
	-MD                                    \
	-MMD                                   \
	-Werror                                \
	-mcmodel=kernel                        \
	-mno-80387                             \
	-mno-red-zone                          \
	-fno-rtti                              \
	-fno-exceptions                        \
	-ffreestanding                         \
	-fno-stack-protector                   \
	-fno-omit-frame-pointer                \
	-fno-isolate-erroneous-paths-attribute \
	-fno-delete-null-pointer-checks        \
	-I./kernel                             \
	-I./libs/libc                          \
	-I./libs/max_os_libs
CXXHARDFLAGS := $(CFLAGS)                      \
	-DBUILD_TIME='"$(BUILD_TIME)"'         \
	-std=c++20                             \
	-g                                     \
	-masm=intel                            \
	-fno-pic                               \
	-no-pie                                \
	-m64                                   \
	-mavx                                  \
	-mavx2                                 \
	-Wall                                  \
	-MD                                    \
	-msse                                  \
	-MMD                                   \
	-Werror                                \
	-mcmodel=kernel                        \
	-mno-80387                             \
	-mno-red-zone                          \
	-fno-rtti                              \
	-fno-exceptions                        \
	-ffreestanding                         \
	-fno-stack-protector                   \
	-fno-omit-frame-pointer                \
	-fno-isolate-erroneous-paths-attribute \
	-fno-delete-null-pointer-checks        \
	-I./kernel                             \
	-I./libs/libc                          \
	-I./libs/max_os_libs

LDHARDFLAGS := $(LDFLAGS)        \
	-z max-page-size=0x1000   \
	-T $(LINK_PATH)

.PHONY:$(KERNEL_HDD)
$(KERNEL_HDD): $(KERNEL_ELF)
	rm -f $(KERNEL_HDD)
	dd if=/dev/zero bs=1M count=0 seek=64 of=$(KERNEL_HDD)
	parted -s $(KERNEL_HDD) mklabel gpt
	parted -s $(KERNEL_HDD) mkpart primary 2048s 100%
	echfs-utils -g -p0 $(KERNEL_HDD) quick-format 512
	echfs-utils -g -p0 $(KERNEL_HDD) import limine.cfg limine.cfg
	echfs-utils -g -p0 $(KERNEL_HDD) import $(KERNEL_ELF) max_os.elf
	limine-install $(KERNEL_HDD)

.PHONY:run
run: $(KERNEL_HDD)
	qemu-system-x86_64 -m 4G -s -device pvpanic -smp 6 -serial stdio -enable-kvm -d cpu_reset -d guest_errors -hda $(KERNEL_HDD) \
		-nic user,model=e1000 -M q35 -cpu host 

.PHONY:super
super:
	@make app -j12
	@make format
	@make -j12

	@objdump kernel.elf -f -s -d --source > kernel.map
	@make run

$(BUILD_OUT)/%.o: %.c 
	@$(DIRECTORY_GUARD)
	@echo "[KERNEL] (c) $<"
	@$(CC) $(CHARDFLAGS) -c $< -o $@
$(BUILD_OUT)/%.o: %.cpp 
	@$(DIRECTORY_GUARD)
	@echo "[KERNEL] (cpp) $<"
	@$(CXX) $(CXXHARDFLAGS) -c $< -o $@
%.h : %.h 
	@echo "[KERNEL] (h) $<"


.PHONY:$(KERNEL_ELF)
$(KERNEL_ELF): $(COBJFILES) $(CXXOBJFILES) $(LINK_PATH)
	@ld $(LDHARDFLAGS) $(COBJFILES) $(CXXOBJFILES) -o $@

.PHONY:clean
clean:
	-rm -f $(KERNEL_HDD) $(KERNEL_ELF) $(OBJ)