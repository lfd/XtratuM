/*
 * $FILE$
 * 
 * ld script to build ia32
 * 
 * $VERSION$
 *
 * Author: Miguel Masmano <mmasmano@ai2.upv.es>
 * 
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

/* <track id="PARTITION_LINKER_SCRIPT"> */

/*OUTPUT_FORMAT("binary")*/
OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH("i386")
ENTRY(start)

SECTIONS
{
  .text ALIGN (4): {
	. = ALIGN(4K);
	_sguest = .;
  	*(.text.init)
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

  .bss ALIGN (4) : {
  	*(.xm_ctrl)
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

/* </track id="PARTITION_LINKER_SCRIPT"> */
