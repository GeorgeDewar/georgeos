/* entry point of our kernel */
ENTRY(_start)

SECTIONS
{
	. = 0x7e00;

  	/* text section */
	.text  : 
	{
		*(.text) *(.text.$)
	}

}
