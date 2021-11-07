/* entry point of our kernel */
ENTRY(main)

SECTIONS
{
	. = 0x7c00;

  	/* text section */
	.text  : 
	{
		*(.text) *(.text.$)
	}

}
