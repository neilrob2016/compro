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



inline void commPutchar(char c)
{
	if (mode != MODE_CODE && mode != MODE_TYPE_LINENUM) putchar(c);
}



inline bool isNonNLSpace(char c)
{
	return (c != '\n' && isspace(c));
}
