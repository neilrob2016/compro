#include "globals.h"

void doIndent(void);
bool notNLSpace(char c);


void startCComment(void)
{
	in_c_comment = true;
	switch(mode)
	{
	case MODE_C_TO_CPP:
		c_indent_cnt = (int)(memptr - line_start_ptr - 1);
		printf("//");
		break;
	case MODE_COM_LINENUM:
		// Check linenum as don't want to print linenum if more than 1 
		// C comment on a line.
		if (linenum > c_end_linenum) printLineNum();
		// Fall through
	case MODE_COM:
		if (linenum > c_start_linenum && option_l)
		{
			printLineStart();
			print_line = true;
		}
		// Fall through
	case MODE_CPP_TO_C:
		c_start_linenum = linenum;
		printf("/*");
		break;
	case MODE_TYPE_LINENUM:
		++c_cnt;
		// Don't increment line count if we have 2+ C comments on the 
		// same line.
		if (linenum > c_start_linenum) ++linecnt;
		c_start_linenum = linenum;
		printf("%-4d  C     ",++comcnt);
		break;
	case MODE_CODE:
		break;
	default:
		assert(0);
	}
}



void inCComment(char c)
{
	if (c == '\n')
	{
		switch(mode)
		{
		case MODE_C_TO_CPP:
			if (option_i)
				doIndent();
			else
				printf("\n//");
			break;
		case MODE_CPP_TO_C:
			putchar('\n');
			break;
		case MODE_CODE:
			break;
		case MODE_COM:
			putchar('\n');
			print_line = option_l;
			break;
		case MODE_COM_LINENUM:
			putchar('\n');
			if (option_x) printLineNum();
			print_line = option_l;
			break;
		case MODE_TYPE_LINENUM:
			++linecnt;
			break;
		default:
			assert(0);
		}
		return;
	}

	if (c != '*' || memptr == memend)
	{
		modePutchar1(c);
		return;
	}

	// Check for end of comment
	if (*++memptr != '/')
	{
		modePutchar1(*--memptr);
		return;
	}

	// End of the comment
	in_c_comment = false;
	c_indent_cnt = 0;
	switch(mode)
	{
	case MODE_C_TO_CPP:
		// If following the end of the comment its just whitespace 
		// before the newline then skip it.
		for(char *ptr=memptr+1;ptr <= memend && isspace(*ptr);++ptr)
		{
			if (*ptr == '\n')
			{
				memptr = ptr;
				line_start_ptr = ptr + 1;
				break;
			}
		}
		putchar('\n');
		break;
	case MODE_COM:
	case MODE_COM_LINENUM:
		// Want to print newline at end of the line with the comment 
		// on otherwise C comments will all appear on one line 
		print_nl = true;
		c_end_linenum = linenum;
		// Fall through
	case MODE_CPP_TO_C:
		printf("*/");
		break;
	case MODE_TYPE_LINENUM:
		printCCommentFromTo();
		break;
	default:
		break;
	}
}



void printCCommentFromTo(void)
{
	if (linenum > c_start_linenum)
	{
		printf("%d to %d (%d)\n",
			c_start_linenum,linenum,linenum - c_start_linenum + 1);
	}
	else printf("%d\n",linenum);
}



/*** Indent the new C++ comment by the same amount of the previous multi line 
     C comment ***/
void doIndent(void)
{
	putchar('\n');
	char *end = memptr + c_indent_cnt;
	for(++memptr;memptr <= end && 
	             memptr <= memend && 
	             notNLSpace(*memptr);++memptr)
	{
		putchar(*memptr);
	}
	printf("//");
	if (memptr < memend && notNLSpace(*memptr))
	{
		if (notNLSpace(*(memptr+1)))
			++memptr;
		else
			putchar(' ');
	}
	else --memptr;
}



inline bool notNLSpace(char c)
{
	return (c != '\n' && isspace(c));
}

