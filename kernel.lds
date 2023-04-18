OUTPUT_ARCH(riscv)
ENTRY(_start)

HIDDEN(KERNEL_ENTRY_ADDR = 0x80200000);
HIDDEN(KERNEL_END_ADDR = 0x80600000);

SECTIONS {
    . = KERNEL_ENTRY_ADDR;
    .text : {
        *(.boot)
        *(.text .text.*)
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