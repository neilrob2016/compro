#include "globals.h"


void printLineStart(void)
{
	// Print back to comment start. Skip the start chars of comment
	for(char *ptr=line_start_ptr;ptr <= memptr-2;++ptr) putchar(*ptr);
}



void printLineNum(void)
{
	printf("%-3d: ",linenum);
}



void printTotals(void)
{
	if (option_x)
	{
		printf("\nTotal of %d lines out of %d consisting of %d C comments and %d %s comments.\n\n",
			linecnt,linenum-1,c_cnt,cpp_cnt,cpp_c99_str);
	}
	exit(0);
}



inline void modePutchar1(char c)
{
	if (mode != MODE_CODE && mode != MODE_TYPE_LINENUM) putchar(c);
}



inline void modePutchar2(char c)
{
	if (mode < MODE_COM || 
	    (print_line && (mode == MODE_COM || mode == MODE_COM_LINENUM))) 
	{ 
	     putchar(c); 
	} 
}
