/* entry point of our kernel */
ENTRY(_start)

SECTIONS
{
	. = 0x20000;

  	/* text section */
	.text  : 
	{
		*(.text) *(.text.$)
	}

}

