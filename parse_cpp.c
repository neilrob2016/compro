#include "globals.h"

bool contiguousCommentFollows(void);
void printToNextComment(void);

void startCPPComment(void)
{
	in_cpp_comment = true;
	// Reset as we'll print newline at end of this comment
	mode_45_print_nl = false;

	switch(mode)
	{
	case MODE_CPP_TO_C:
		printf("/*");
		break;
	case MODE_CODE:
		break;
	case MODE_COM_LINENUM:
		// Check linenum because we don't want to print the line number
		// if we're printing the 2nd, 3rd etc comment on a line.
		if (c_end_linenum != linenum) printLineNum();
		// Fall through
	case MODE_COM:
		if (option_l) printLineStart();
		// Fall through
	case MODE_C_TO_CPP:
		printf("//");
		break;
	case MODE_TYPE_LINENUM:
		++cpp_cnt;
		++linecnt;
		printf("%-4d  C++   %-4d\n",++comcnt,linenum);
		break;
	default:
		assert(0);
	}
}




void inCPPComment(char c)
{
	if (c != '\n')
	{
		commPutchar(c);
		return;
	}
	switch(mode)
	{
	case MODE_CPP_TO_C:
		// See if we can continue a C++ comment so we have just 1 new
		// C comment spanning a number of old C++ comments.
		if (option_l && contiguousCommentFollows()) 
		{
			// Can't just return to the mainloop because as soon as
			// it hits the next C++ comment it'll print it out.
			printToNextComment();
			return;
		}
		puts(" */");
		break;
	case MODE_CODE:
		if (mode_3_print_nl) putchar('\n');
		break;
	case MODE_TYPE_LINENUM:
		break;
	default:
		putchar(c);
	}
	in_cpp_comment = false;
}



/*** See if there's nothing but whitespace (excluding newlines) until the next
     C++ comment ***/
bool contiguousCommentFollows(void)
{
	char *ptr = memptr + 1;
	for(;ptr <= memend && isNonNLSpace(*ptr);++ptr);
	return (ptr < memend && *ptr == '/' && *++ptr == '/');
}



/*** Print out the whitespace found by contiguousCommentFollows() ***/
void printToNextComment(void)
{
	putchar('\n');
	for(++memptr;isNonNLSpace(*memptr);++memptr) putchar(*memptr);
	assert(*memptr == '/');
	// Replace // with 2 spaces 
	printf("  ");
	// Skip last slash in // 
	++memptr;
}
