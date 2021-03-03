global sse_init
sse_init:
    mov rax, cr0
    and rax, ~(1<<2)
    or rax, (1<<1)
    mov cr0, rax
    mov rax, cr4
    or rax, 3 << 9
    mov cr4, rax
    ret

global enable_xsave
enable_xsave:
    mov rax, cr4
    or rax, (1<<18)
    mov cr4, rax

    push rax
    push rcx
    push rdx
    xor rax,rax
    xor rcx, rcx
    mov rdx, 0
    or rax, 3
    xsetbv

    pop rdx
    pop rcx
    pop rax
    ret

global avx_init
avx_init:
    push rax
    push rcx
    push rdx
    xor rax,rax
    xor rcx, rcx
    mov rdx, 0
    or rax, 7
    xsetbv

    pop rdx
    pop rcx
    pop rax
    ret