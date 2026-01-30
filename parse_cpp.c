#include "globals.h"

void startCPPComment(void)
{
	in_cpp_comment = true;
	switch(mode)
	{
	case MODE_CPP_TO_C:
		printf("/*");
		break;
	case MODE_CODE:
		break;
	case MODE_COM_LINENUM:
		printLineNum();
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
		printf("%-4d  %s   %-4d\n",++comcnt,cpp_c99_str,linenum);
		break;
	default:
		assert(0);
	}
}




void inCPPComment(char c)
{
	if (c == '\n')
	{
		in_cpp_comment = false;
		switch(mode)
		{
		case MODE_CPP_TO_C:
			puts(" */");
			in_cpp_comment = false;
			break;
		case MODE_CODE:
		case MODE_TYPE_LINENUM:
			break;
		default:
			putchar(c);
		}
	}
	else modePutchar1(c);
}
