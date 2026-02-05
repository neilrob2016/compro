#include "globals.h"

bool skipToCPP(char *ptr);
void codePutchar(char c);
void printTotals(void);

/*** We load the file into memory instead of just reading it direct. Using a
     pointer to the data instead of seeking in the file makes the parsing code
     simpler at the expense of the memory required ***/
void loadFile(void)
{
	int fd;
	long blocksize;

	if (filename)
	{
		// I could memory map the file but you can't do that with stdin
		// so there'd need to be seperate load code for stdin and files.
		struct stat fs;
		if ((fd = open(filename,O_RDONLY)) == -1)
		{
			fprintf(stderr,"ERROR: open(\"%s\"): %s\n",
				filename,strerror(errno));
			exit(1);
		}
		// Shouldn't fail but can on rare occasions, eg NFS issue.
		if (fstat(fd,&fs) == -1)
		{
			fprintf(stderr,"ERROR: fstat(): %s\n",strerror(errno));
			exit(1);
		}
		blocksize = (long)fs.st_size;
	}
	else
	{
		fd = STDIN_FILENO;
		blocksize = 1000; // 1K seems reasonable for stdin
	}

	// Read into memory
	memstart = NULL;
	long pos = 0;
	long alloced = 0;
	for(long len=1;len;pos+=len)
	{
		if (pos >= alloced)
		{
			// Expanding by 1K at a time seems reasonable
			alloced += blocksize;
			if (!(memstart = (char *)realloc(memstart,alloced)))
			{
				fprintf(stderr,"ERROR: Out of memory!\n");
				exit(1);
			}
		}
		if ((len = read(fd,memstart + pos,blocksize)) == -1)
		{
			fprintf(stderr,"ERROR: read(): %s\n",strerror(errno));
			exit(1);
		}
	}
	memend = memstart + pos - 1;
	line_start_ptr = memstart;
	close(fd);
}



void parseFile(void)
{
	char c = 0;
	char prev_c = 0;
	char quotes = 0;
	bool escaped = false;
	bool skip_to_cpp = (mode == MODE_CODE && option_l);
	
	in_c_comment = false;
	in_cpp_comment = false;
	mode_3_print_nl = true;
	mode_45_print_nl = false;
	print_line = false;
	c_cnt = 0;
	cpp_cnt = 0;
	comcnt = 0;
	linenum = 1;
	linecnt = 0;
	c_start_linenum = 0;
	c_end_linenum = 0;
	c_indent_cnt = 0;

	if (mode == MODE_TYPE_LINENUM && option_x)
	{
		puts("\nCom#  Type  Line#");
		puts("----  ----  -----");
	}

	memptr = memstart;
	if (skip_to_cpp) skipToCPP(memstart);

	for(;memptr <= memend;++memptr)
	{
		prev_c = c;
		c = *memptr;
		if (c == '\n')
		{
			++linenum;
			line_start_ptr = memptr + 1;
			print_line = (option_l && in_c_comment);
		}
		if (in_c_comment)
		{
			inCComment(c);
			continue;
		}
		if (in_cpp_comment)
		{
			inCPPComment(c);
			if (!skip_to_cpp || c != '\n' || !skipToCPP(memptr+1)) 
				continue;
			c = *memptr;
			prev_c = 0;
		}

		// Not yet in a comment
		switch(c)
		{
		case '\n':
			escaped = false;
			if (mode < MODE_COM || mode_45_print_nl || quotes)
			{
				putchar('\n');
				mode_45_print_nl = false;
			}
			if (skip_to_cpp && !quotes && skipToCPP(memptr+1))
			{
				// Drop back because for() will increment again
				--memptr;
				prev_c = 0;
			}
			break;
		case '\\':
			codePutchar(c);
			escaped = !escaped;
			break;
		case '\'':
		case '"':
			/* In C/C++ single quotes should only quote a single
			   character but treat the same as double quotes for
			   simplicity */
			codePutchar(c);
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
			if (quotes || memptr == memend)
			{
				codePutchar(c);
				break;
			}
			switch(*++memptr)
			{
			case '*':
				startCComment();
				break;
			case '/':
				startCPPComment();
				break;
			default:
				codePutchar(c);
				--memptr;
			}
			break;
		default:
			escaped = false;
			codePutchar(c);
		}
	}
	fflush(stdout);
	c = *memend;

	// In case we had an EOF without a newline or still in a comment...
	switch(mode)
	{
	case MODE_C_TO_CPP:
		if (in_c_comment) putchar('\n');
		break;
	case MODE_CPP_TO_C:
		if (in_cpp_comment) printf(" */\n");
		break;
	case MODE_CODE:
		if (!in_c_comment && !in_cpp_comment && c != '\n')
			putchar('\n');
		break;
	case MODE_COM:
	case MODE_COM_LINENUM:
		if (mode_45_print_nl || in_cpp_comment) putchar('\n');
		break;
	case MODE_TYPE_LINENUM:
		if (in_c_comment) printCCommentFromTo();
		printTotals();
		break;
	default:
		assert(0);
	}
}



/*** See if we have nothing but whitespace then a C++ comment start ***/
bool skipToCPP(char *ptr)
{
	for(;ptr < memend && isNonNLSpace(*ptr);++ptr);
	if (ptr < memend && *ptr == '/' && *(ptr+1) == '/') 
	{
		mode_3_print_nl = false;
		memptr = ptr;
		return true;
	}
	mode_3_print_nl = true;
	return false;
}



inline void codePutchar(char c)
{
	if (mode < MODE_COM || 
	    (print_line && (mode == MODE_COM || mode == MODE_COM_LINENUM)))
	{
		putchar(c);
	}
}



void printTotals(void)
{
	if (option_x)
	{
		if (*memend != '\n') ++linenum;
		printf("\nTotal of %d lines out of %d consisting of %d C comments and %d C++ comments.\n\n",
			linecnt,linenum-1,c_cnt,cpp_cnt);
	}
	exit(0);
}
