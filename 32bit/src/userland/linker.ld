/* entry point of our kernel */
ENTRY(_start)

SECTIONS
{
	. = 0x100000;

  	/* text section */
	.text  : 
	{
		*(.text) *(.text.$)
	}

}
