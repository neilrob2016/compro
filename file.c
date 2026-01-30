#include "globals.h"

/*** We load the file into memory instead of just reading it direct. Using a
     pointer to the data instead of seeking in the file makes the parsing code
     simpler at the expense of the memory required ***/
void loadFile(void)
{
	int fd;
	long blocksize;

	if (filename)
	{
		// I could memory map the file it but would mean more code and 
		// doesn't work for stdin anyway 
		struct stat fs;
		if ((fd = open(filename,O_RDONLY)) == -1)
		{
			fprintf(stderr,"ERROR: open(): %s\n",strerror(errno));
			exit(1);
		}
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
	char quotes = 0;                                                                bool escaped = false;

	in_c_comment = false;
	in_cpp_comment = false;
	print_nl = false;
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

	for(memptr=memstart;memptr <= memend;++memptr)
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
			continue;
		}

		// Not yet in a comment
		switch(c)
		{
		case '\n':
			escaped = false;
			if (mode < MODE_COM || print_nl || quotes)
			{
				putchar('\n');
				print_nl = false;
			}
			break;
		case '\\':
			modePutchar2(c);
			escaped = !escaped;
			break;
		case '\'':
		case '"':
			/* In C/C++ single quotes should only quote a single
			   character but treat the same as double quotes for
			   simplicity */
			modePutchar2(c);
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
				modePutchar2(c);
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
				modePutchar2(c);
				--memptr;
			}
			break;
		default:
			escaped = false;
			modePutchar2(c);
		}
	}
	if (mode == MODE_TYPE_LINENUM)
	{
		if (in_c_comment) printCCommentFromTo();
		printTotals();
	}
	else fflush(stdout);
}
