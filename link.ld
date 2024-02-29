ENTRY (_loader)

SECTIONS
{
   . = 0x1200;

   .text :
   {
      *(.text)
      *(.text.*)
   }

   .rodata ALIGN (0x1000) :
   {
      *(.rodata)
   }

   .data ALIGN (0x1000) :
   {
      start_ctors = .;
      *(.ctor*)
      end_ctors = .;
      start_dtors = .;
      *(.dtor*)
      end_dtors = .;
      *(.data)
   }

   .bss :
   {
      _sbss = .;
      *(COMMON)
      *(.bss)
      _ebss = .;
   }
}