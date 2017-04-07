/*
 * $FILE$
 * 
 * ld script to build x86
 * 
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 * 
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

/*OUTPUT_FORMAT("binary")*/
OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH("i386")
ENTRY(start)

OFFSET = 8*1024*1024;

SECTIONS
{
  _sguest = .;

  .boot ALIGN(4): {
  	*(.text.init)
  }
  . = ALIGN(0x2000);
  _slinuxloader = .;
  . = . + OFFSET;
  _elinuxloader = .;
  .text ALIGN (4) : {
  	*(.text)
  }

  .rodata ALIGN (4) : {
	*(.rodata)
  	*(.rodata.*)
	*(.rodata.*.*)
  }
  
  .data ALIGN (4) : {
	_sdata = .;
	*(.data)
	_edata = .;
  }

 .linux ALIGN (4) : {
  	 _slinuxImg = .;
  	 *(.linux)
	 _elinuxImg = .;
  }
  .bss ALIGN(4) : {
	_sbss = .;
	*(COMMON)
	*(.bss)
	_ebss = .;
  }
  
  _eguest = .;

  /DISCARD/ :
  {
	*(.note)
	*(.comment*)
  }
}

