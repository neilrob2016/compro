/****************************************************************************
 COMPRO
 A simple single pass, non tokenising parser that processes C and C++ comments 
 in a C/C++ source file.

 There are 5 output modes:
    1 - Convert C comments to C++
    2 - Convert C++ comments to C
    3 - Only output code
    4 - Only output comments (with linenum if -l option given)
    5 - Print comment line numbers and type

 Input is from stdin or a source file given with the -f option. Output is to 
 stdout, errors to stderr. This has been compiled and tested on Linux and MacOS.

 Compilation: cc -std=c99 compro.c -o compro

 Written by Neil Robertson
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define VERSION "20260119"

enum
{
	MODE_C_TO_CPP,
	MODE_CPP_TO_C,
	MODE_ONLY_CODE,
	MODE_ONLY_COM,
	MODE_LINENUM,
	NUM_MODES
};

FILE *fp;
char *filename;
int mode;
int linenum;
int start_linenum;
int c_cnt;
int cpp_cnt;
int linecnt;
bool in_c_com;
bool in_cpp_com;
bool print_nl;
bool extra_info;

void parseCmdLine(int argc, char **argv);
void openFile(void);
void processFile(void);
void inCComment(char c);
void inCPPComment(char c);
void printWhiteSpaceAtEnd(void);
bool checkForCommentStart(void);
void printCComLines(void);
void printTotals(void);


int main(int argc, char **argv)
{
	parseCmdLine(argc,argv);
	openFile();
	processFile();
	return 0;
}



void parseCmdLine(int argc, char **argv)
{
	filename = NULL;
	mode = MODE_C_TO_CPP;
	extra_info = false;

	for(int i=1;i < argc;++i)
	{
		if (argv[i][0] != '-' || strlen(argv[i]) != 2) goto USAGE;

		char c = argv[i][1];
		switch(c)
		{
		case 'x':
			extra_info = true;
			continue;
		case 'v':
			puts("\n*** C/C++ Comments Processor ***\n");
			printf("Version   : %s\n",VERSION);
			printf("Build date: %s,%s\n\n",__DATE__,__TIME__);
			exit(0);
		}

		if (++i == argc) goto USAGE;	
		switch(c)
		{
		case 'f':
			filename = argv[i];
			break;
		case 'm':
			mode = atoi(argv[i]);
			if (!mode || mode > NUM_MODES) goto USAGE;
			--mode;
			break;
		default:
			goto USAGE;
		}	
	}
	return;

	USAGE:
	fprintf(stderr,"Usage: %s\n"
	       "       -f <C/C++ source file> : Default = stdin\n"
	       "       -m <mode 1 to %d>       : Default = %d\n"
	       "                                   1 - Convert C comments to C++\n"
	       "                                   2 - Convert C++ comments to C\n"
	       "                                   3 - Only output code\n"
	       "                                   4 - Only output comments\n"
	       "                                   5 - Print comment line numbers and type\n"
	       "       -x                     : Extra info in mode:\n"
	       "                                   4 - Print line numbers\n"
	       "                                   5 - Print header and footer\n"
	       "                                No effect in other modes.\n"
	       "                              : other modes.\n"
	       "       -v                     : Print version info then exit.\n"
	       "Note:\n"
	       "   - All arguments are optional.\n"
	       "   - All output goes to stdout except errors which go to stderr.\n",
		argv[0],NUM_MODES,mode+1);
	exit(0);
}



void openFile(void)
{
	if (filename)
	{
		if (!(fp = fopen(filename,"r")))
		{
			fprintf(stderr,"ERROR: fopen(\"%s\"): %s\n",
				filename,strerror(errno));
			exit(1);
		}
	}
	else fp = stdin;
}


#define MODEPUTCHAR1(C) \
	if (mode != MODE_ONLY_CODE && mode != MODE_LINENUM) putchar(C)
#define MODEPUTCHAR2(C) \
	if (mode != MODE_ONLY_COM && mode != MODE_LINENUM) putchar(C)

/*** Read the file and parse it ***/
void processFile(void)
{
	char c = getc(fp);
	char prev_c = 0;
	char quotes = 0;
	bool escaped = false;
	int comcnt = 0;

	in_c_com = false;
	in_cpp_com = false;
	c_cnt = 0;
	cpp_cnt = 0;
	print_nl = false;
	linenum = 1;
	linecnt = 0;

	if (mode == MODE_LINENUM && extra_info)
	{
		puts("\nCom#  Type  Line(s)");
		puts("----  ----  -------");
	}

	while(!feof(fp))
	{
		if (c == '\n') ++linenum;

		if (in_c_com)
		{
			inCComment(c);
			goto NEXT;
		}
		if (in_cpp_com)
		{
			inCPPComment(c);
			goto NEXT;
		}

		// Not yet in a comment
		switch(c)
		{
		case '\n':
			escaped = false;
			if ((mode != MODE_LINENUM && mode != MODE_ONLY_COM) || 
			    print_nl || quotes)
			{
				putchar('\n');
				print_nl = false;
			}
			break;
		case '\\':
			MODEPUTCHAR2(c);
			escaped = !escaped;
			break;
		case '\'':
		case '"':
			/* In C/C++ single quotes should only quote a single
			   character but treat the same as double quotes for
			   simplicity */
			MODEPUTCHAR2(c);
			if (quotes == c)
			{
				if (escaped)
					escaped = false;
				else
					quotes = 0;
			}
			// From C++14 onwards single quotes can also be digit 
			// separators, eg 1'000'000, so need to check for that.
			else if (!quotes && (c == '"' || !isdigit(prev_c)))
				quotes = c;
			break;
		case '/':
			escaped = false;
			if (quotes)
			{
				MODEPUTCHAR2(c);
				break;
			}
			if (prev_c == '/')
			{
				// Start of C++ comment
				in_cpp_com = true;
				switch(mode)
				{
				case MODE_CPP_TO_C:
					putchar('*');
					break;
				case MODE_ONLY_COM:
					if (extra_info)
						printf("Line %d: ",linenum);
					printf("//");
					break;
				case MODE_LINENUM:
					++cpp_cnt;
					++linecnt;
					printf("%-4d  C++   %-4d\n",
						++comcnt,linenum);
					break;
				default:
					MODEPUTCHAR1(c);
				}
				break;
			}
			if (mode == MODE_ONLY_CODE)
			{
				// In this mode we need to look 1 char ahead
				// as we don't want to output the first '/'
				if (!checkForCommentStart()) exit(0);
				if (!in_c_com && !in_cpp_com)
				{
					putchar(c);
					goto NEXT;
				}
				break;
			}
			else MODEPUTCHAR2(c);
			break;
		case '*':
			escaped = false;
			if (quotes || prev_c != '/')
			{
				MODEPUTCHAR2(c);
				break;
			}

			// Start of C comment
			in_c_com = true;
			start_linenum = linenum;
			switch(mode)
			{
			case MODE_C_TO_CPP:
				putchar('/');
				break;
			case MODE_ONLY_COM:
				if (extra_info)
					printf("Line %d: ",linenum);
				printf("/*");
				break;
			case MODE_LINENUM:
				++c_cnt;
				printf("%-4d  C     ",++comcnt);
				break;
			default:
				MODEPUTCHAR1(c);
			}
			break;
		default:
			escaped = false;
			MODEPUTCHAR2(c);
		}

		NEXT:
		fflush(stdout);
		prev_c = c;
		c = getc(fp);
	}
	if (mode == MODE_LINENUM)
	{
		if (in_c_com) printCComLines();
		printTotals();
	}
}



void inCComment(char c)
{
	if (c == '\n')
	{
		switch(mode)
		{
		case MODE_C_TO_CPP:
			printf("\n//");
			break;
		case MODE_ONLY_CODE:
		case MODE_LINENUM:
			break;
		default:
			putchar('\n');
		}
		return;
	}
	if (c != '*')
	{
		MODEPUTCHAR1(c);
		return;
	}

	// Check for end of comment
	char prev_c = c;
	c = getc(fp);
	if (feof(fp))
	{
		// '*' was last character in file
		if (mode == MODE_LINENUM)
		{
			printCComLines();
			printTotals();
		}
		else MODEPUTCHAR1(prev_c);
		exit(0);
	}

	if (c != '/')
	{
		// Not the end of comment
		MODEPUTCHAR1(prev_c);
		ungetc(c,fp);
		return;
	}

	// End of the comment
	in_c_com = false;
	switch(mode)
	{
	case MODE_C_TO_CPP:
	case MODE_ONLY_CODE:
		printWhiteSpaceAtEnd();
		break;
	case MODE_ONLY_COM:
		// Want to print newline at end of the line with the comment 
		// on otherwise C comments will all appear on one line 
		print_nl = true;
		// Fall through
	case MODE_CPP_TO_C:
		printf("*/");
		break;
	case MODE_LINENUM:
		printCComLines();
		break;
	default:
		break;
	}
}



void inCPPComment(char c)
{
	if (c == '\n')
	{
		in_cpp_com = false;
		switch(mode)
		{
		case MODE_CPP_TO_C:
			puts(" */");
			in_cpp_com = false;
			break;
		case MODE_LINENUM:
			break;
		default:
			putchar(c);
		}
	}
	else MODEPUTCHAR1(c);
}



/*** Following the end of a C comment we've converted to C++ print any
     whitespace and end at \n or non whitespace ***/
void printWhiteSpaceAtEnd(void)
{
	char c = getc(fp);
	while(!feof(fp))
	{
		if (c == '\n')
		{
			putchar('\n');
			return;
		}
		if (!isspace(c))
		{
			if (mode == MODE_C_TO_CPP) putchar('\n');
			ungetc(c,fp);
			return;
		}
		putchar(c);
		c = getc(fp);
	}
	// End of file
	exit(0);
}



/*** Check for the start of a comment for MODE_ONLY_CODE ***/
bool checkForCommentStart(void)
{
	char c = getc(fp);
	if (feof(fp)) return false;
	if (c == '/') in_cpp_com = true;
	else if (c == '*') in_c_com = true;
	else ungetc(c,fp);
	return true;
}



void printCComLines(void)
{
	if (linenum > start_linenum)
		printf("%d to %d\n",start_linenum,linenum);
	else
		printf("%d\n",linenum);
	linecnt += (linenum - start_linenum + 1);
}



void printTotals(void)
{
	if (extra_info)
	{
		printf("\nTotal of %d lines of %d C comments and %d C++ comments.\n\n",
			linecnt,c_cnt,cpp_cnt);
	}
	exit(0);
}
