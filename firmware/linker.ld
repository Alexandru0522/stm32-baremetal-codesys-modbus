/**
 * @file    stm32f103c8tx_flash.ld
 * @brief   Linker script for STM32F103C8Tx (Cortex-M3, 64KB FLASH, 20KB RAM)
 * @note    Industry-standard GNU LD script with C/C++ init arrays and RAM assertion.
 */

ENTRY(Reset_Handler)

/* Adresa de start a stivei (Top of RAM: 0x20000000 + 20KB = 0x20005000) */
_estack = ORIGIN(RAM) + LENGTH(RAM);

/* Rezervare minimă garantată pentru Heap și Stack (previne RAM overflow la link-time) */
_Min_Heap_Size  = 0x200; /* 512 Bytes */
_Min_Stack_Size = 0x400; /* 1024 Bytes */

/* Maparea blocurilor fizice de memorie conforme datasheet-ului STM32F103C8 */
MEMORY
{
  FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
  RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 20K
}

SECTIONS
{
  /* Tabelul de vectori de întrerupere - Mapped la adresa de offset 0x0 în FLASH */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } >FLASH

  /* Codul instrucțiunilor (Program Code) și constantele Read-Only (.rodata) */
  .text :
  {
    . = ALIGN(4);
    *(.text)
    *(.text.*)
    *(.rodata)
    *(.rodata.*)
    *(.glue_7)
    *(.glue_7t)
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* Simbol exportat: Sfârșitul secțiunii .text */
  } >FLASH

  /* Tabelele de Unwinding pentru ARM Exception Handling / Debugging */
  .ARM.extab :
  {
    *(.ARM.extab* .gnu.linkonce.armextab.*)
  } >FLASH

  .ARM.exidx :
  {
    __exidx_start = .;
    *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    __exidx_end = .;
  } >FLASH

  /* Secțiuni de inițializare C/C++ (Constructori globali / Newlib runtime) */
  .preinit_array :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >FLASH

  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >FLASH

  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >FLASH

  /* Adresa de încărcare din FLASH pentru secțiunea .data */
  _sidata = LOADADDR(.data);

  /* Variabile inițializate: stocate în FLASH, relocate în RAM la pornire de Reset_Handler */
  .data :
  {
    . = ALIGN(4);
    _sdata = .;        /* Simbol exportat: Inceput .data în RAM */
    *(.data)
    *(.data.*)
    . = ALIGN(4);
    _edata = .;        /* Simbol exportat: Sfârșit .data în RAM */
  } >RAM AT> FLASH

  /* Variabile neinițializate (Zero-initialized Data) - curățate în RAM la boot */
  .bss :
  {
    . = ALIGN(4);
    _sbss = .;         /* Simbol exportat: Inceput .bss în RAM */
    *(.bss)
    *(.bss.*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;         /* Simbol exportat: Sfârșit .bss în RAM */
  } >RAM

  /* Verificare de siguranță la nivel de Linker pentru spațiul Heap și Stack */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >RAM

  /* Eliminare simboluri neutilizate din librăriile standard C */
  /DISCARD/ :
  {
    libc.a (*)
    libm.a (*)
    libgcc.a (*)
  }
}
