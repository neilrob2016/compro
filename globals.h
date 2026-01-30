#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>

#define VERSION "20260130"

#ifdef MAINFILE
#define EXTERN
#else
#define EXTERN extern
#endif

enum
{
	MODE_C_TO_CPP,
	MODE_CPP_TO_C,
	MODE_CODE,
	MODE_COM,
	MODE_COM_LINENUM,
	MODE_TYPE_LINENUM,
	NUM_MODES
};

// Command line
EXTERN char *filename;
EXTERN int mode;
EXTERN bool option_i;
EXTERN bool option_l;
EXTERN bool option_x;

// Runtime
EXTERN char *memstart;
EXTERN char *memend;
EXTERN char *memptr;
EXTERN char *cpp_c99_str;
EXTERN char *line_start_ptr;
EXTERN int linenum;
EXTERN int c_start_linenum;
EXTERN int c_end_linenum;
EXTERN int c_cnt;
EXTERN int c_indent_cnt;
EXTERN int cpp_cnt;
EXTERN int linecnt;
EXTERN int comcnt;
EXTERN bool in_c_comment;
EXTERN bool in_cpp_comment;
EXTERN bool print_nl;
EXTERN bool print_line;

// file.c
void loadFile(void);
void parseFile(void);

// parse_c.c
void startCComment(void);
void inCComment(char c);
void printCCommentFromTo(void);

// parse_cpp.c
void startCPPComment(void);
void inCPPComment(char c);

// print.c
void printLineStart(void);
void printLineNum(void);
void printTotals(void);
void modePutchar1(char c);
void modePutchar2(char c);
