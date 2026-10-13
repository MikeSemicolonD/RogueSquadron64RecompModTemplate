/* Code mod layout, as in the N64Recomp mod templates: one region, so every section's load address equals its run address. */
/* ld passes each input section (.recomp_patch, .recomp_hook.<func>, .recomp_callback.<mod>:<event>, ...) through as its own output section, which is what RecompModTool reads. */
RAMBASE = 0x81000000;

MEMORY {
    extram(ARWX) : ORIGIN = RAMBASE, LENGTH = 64M
}

SECTIONS {
    /DISCARD/ : { *(.got) *(.MIPS.*) *(.reginfo) *(.pdr) *(.mdebug*) *(.comment) *(.gnu.attributes) *(.note*) }
}
