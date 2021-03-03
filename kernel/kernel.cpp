#include "stivale2.h"
#include <cpu/serial.h>
#include <cpu/sse.h>
#include <types.h>
// #include <stddef.h>
// #include <stdint.h>

// We need to tell the stivale bootloader where we want our stack to be.
// We are going to allocate our stack as an uninitialised array in .bss.
static u8 stack[4096];

// stivale2 uses a linked list of tags for both communicating TO the
// bootloader, or receiving info FROM it. More information about these tags
// is found in the stivale2 specification.

// As an example header tag, we're gonna define a framebuffer header tag.
// This tag tells the bootloader that we want a graphical framebuffer instead
// of a CGA-compatible text mode. Omitting this tag will make the bootloader
// default to text mode.
struct stivale2_header_tag_framebuffer framebuffer_hdr_tag = {
    // All tags need to begin with an identifier and a pointer to the next tag.
    .tag = {
        // Identification constant defined in stivale2.h and the specification.
        .identifier = STIVALE2_HEADER_TAG_FRAMEBUFFER_ID,
        // If next is 0, then this marks the end of the linked list of tags.
        .next = 0 },
    // We set all the framebuffer specifics to 0 as we want the bootloader
    // to pick the best it can.
    .framebuffer_width = 0,
    .framebuffer_height = 0,
    .framebuffer_bpp = 0
};

// The stivale2 specification says we need to define a "header structure".
// This structure needs to reside in the .stivale2hdr ELF section in order
// for the bootloader to find it. We use this __attribute__ directive to
// tell the compiler to put the following structure in said section.
__attribute__((section(".stivale2hdr"), used)) struct stivale2_header stivale_hdr = {
    // The entry_point member is used to specify an alternative entry
    // point that the bootloader should jump to instead of the executable's
    // ELF entry point. We do not care about that so we leave it zeroed.
    .entry_point = 0,
    // Let's tell the bootloader where our stack is.
    // We need to add the sizeof(stack) since in x86(_64) the stack grows
    // downwards.
    .stack = (u64)stack + sizeof(stack),
    // No flags are currently defined as per spec and should be left to 0.
    .flags = 0,
    // This header structure is the root of the linked list of header tags and
    // points to the first one (and in our case, only).
    .tags = (u64)&framebuffer_hdr_tag
};

// We will now write a helper function which will allow us to scan for tags
// that we want FROM the bootloader (structure tags).
void* stivale2_get_tag(struct stivale2_struct* stivale2_struct, uint64_t id)
{
    struct stivale2_tag* current_tag = (struct stivale2_tag*)stivale2_struct->tags;
    for (;;) {
        // If the tag pointer is NULL (end of linked list), we did not find
        // the tag. Return NULL to signal this.
        if (current_tag == NULL) {
            return NULL;
        }

        // Check whether the identifier matches. If it does, return a pointer
        // to the matching tag.
        if (current_tag->identifier == id) {
            return current_tag;
        }

        // Get a pointer to the next tag in the linked list and repeat.
        current_tag = (struct stivale2_tag*)current_tag->next;
    }
}

// The following will be our kernel's entry point.
extern "C" [[noreturn]] void _start(struct stivale2_struct* stivale2_struct)
{
    asm volatile("and rsp, -16");
    asm volatile("cli");
    // fs is used for getting cpu n°
    asm volatile("mov ax, 0");
    asm volatile("mov fs, ax");

    Serial::init_serial_out(0x3F8);
    Serial::serial_printf("Max OS!\n");

    SSE::init_sse();
    Serial::serial_printf("Result:");
    Serial::serial_printf("has SSE   .... yes (probably)");
    Serial::serial_printf("has XSAVE .... %s", SSE::has_xsave() ? "yes" : "no");
    Serial::serial_printf("has AVX   .... %s", SSE::has_avx() ? "yes" : "no");

    struct stivale2_struct_tag_framebuffer* fb_hdr_tag;
    fb_hdr_tag = (struct stivale2_struct_tag_framebuffer*)stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_FRAMEBUFFER_ID);

    if (fb_hdr_tag == NULL) {
        // It wasn't found, just hang...
        for (;;) {
            asm("hlt");
        }
    }

    u8* fb_addr = (uint8_t*)fb_hdr_tag->framebuffer_addr;

    for (usize i = 0; i < 128; i++) {
        fb_addr[i] = 0xff;
    }

    // We're done, just hang...
    for (;;) {
        asm("hlt");
    }
}