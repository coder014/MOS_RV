OUTPUT_ARCH(riscv)
ENTRY(_start)

KERNEL_ENTRY_ADDR = 0x80200000;
KERNEL_END_ADDR = 0x80400000;

SECTIONS {
    . = KERNEL_ENTRY_ADDR;
    .text : {
        *(.text._start)
        *(.text)
    }
    PROVIDE(etext = .);

    .data : {
        *(.data .data.* .sdata .sdata.*)
    }
    PROVIDE(edata = .);

    .bss  : {
        *(.bss .bss.* .sbss .sbss.*)
    }
    PROVIDE(bss_end = .);
    . = KERNEL_END_ADDR;
    PROVIDE(end = .);
}