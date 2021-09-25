/* entry point of our kernel */
ENTRY(_start)

SECTIONS
{
	. = 0x80000;

  	/* text section */
	.text  : 
	{
		*(.text) *(.text.$)
	}

}
