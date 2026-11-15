/*
 * @file    stm32f103.ld
 * @brief   Production GNU Linker Script for STM32F103 (64KB Flash, 20KB RAM)
 * @author  Turcu Alexandru-Costin
 * @date    2026
 */

ENTRY(Reset_Handler)

/* Top of RAM (0x20000000 + 20K = 0x20005000) */
_estack = ORIGIN(RAM) + LENGTH(RAM);

/* Minimum required sizes for linker memory check */
_Min_Heap_Size  = 0x200; /* 512 Bytes */
_Min_Stack_Size = 0x400; /* 1024 Bytes */

MEMORY
{
  FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
  RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 20K
}

SECTIONS
{
  /* Vector table placed at start of Flash (Naturally 256-byte aligned for VTOR) */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } >FLASH

  /* Application code and read-only data */
  .text :
  {
    . = ALIGN(4);
    *(.text)
    *(.text.*)
    *(.glue_7)         /* Glue arm to thumb code */
    *(.glue_7t)        /* Glue thumb to arm code */
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* End of text section */
  } >FLASH

  /* Read-only constant data */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)
    *(.rodata.*)
    . = ALIGN(4);
  } >FLASH

  /* C/C++ Static Constructor Initialization Tables */
  .preinit_array :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
    . = ALIGN(4);
  } >FLASH

  .init_array :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
    . = ALIGN(4);
  } >FLASH

  .fini_array :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
    . = ALIGN(4);
  } >FLASH

  /* Address in Flash where initialized data values are stored */
  _sidata = LOADADDR(.data);

  /* Initialized data section loaded into RAM at boot */
  .data :
  {
    . = ALIGN(4);
    _sdata = .;        /* Start of .data in RAM */
    *(.data)
    *(.data.*)
    . = ALIGN(4);
    _edata = .;        /* End of .data in RAM */
  } >RAM AT> FLASH

  /* Uninitialized data section cleared to zero at boot */
  .bss :
  {
    . = ALIGN(4);
    _sbss = .;         /* Start of .bss in RAM */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss.*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;         /* End of .bss in RAM */
    __bss_end__ = _ebss;
  } >RAM

  /* Reserve space for stack and heap to prevent RAM overflow */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >RAM

  /* Linker-time assertion: Throws build error if static RAM allocations overflow boundary */
  ASSERT(ORIGIN(RAM) + LENGTH(RAM) >= (_ebss + _Min_Heap_Size + _Min_Stack_Size),
         "ERROR: SRAM overflowed! Static data (.data + .bss) plus minimum Heap & Stack exceeds 20KB RAM limit.")

  /* Discard unused compiler exception unwinding sections */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
    *(.ARM.extab* .gnu.linkonce.armextab.*)
    *(.ARM.exidx*)
  }
}
