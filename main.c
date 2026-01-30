/****************************************************************************
 COMPRO
 A simple single pass, non tokenising parser that processes C and C++/C99 
 comments in a C/C++ source file. See README for more information.
 ****************************************************************************/
#define MAINFILE
#include "globals.h"

void parseCmdLine(int argc, char **argv);

/////////////////////////////////// START /////////////////////////////////////

int main(int argc, char **argv)
{
	parseCmdLine(argc,argv);
	loadFile();
	parseFile();
	return 0;
}



void parseCmdLine(int argc, char **argv)
{
	filename = NULL;
	mode = MODE_C_TO_CPP;
	option_l = false;
	option_i = false;
	option_x = false;

	for(int i=1;i < argc;++i)
	{
		if (argv[i][0] != '-' || strlen(argv[i]) != 2) goto USAGE;

		char c = argv[i][1];
		switch(c)
		{
		case 'i':
			option_i = true;
			continue;
		case 'l':
			option_l = true;
			continue;
		case 'x':
			option_x = true;
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
			if (mode < 1 || mode > NUM_MODES) goto USAGE;
			--mode;
			break;
		default:
			goto USAGE;
		}	
	}
	if (filename)
	{
		// If a double slash comment is in a .c file then its C99, 
		// not C++
		int len = strlen(filename);
		if (len > 2 && filename[len-1] == 'c' && filename[len-2] == '.')
			cpp_c99_str = "C99";
		else
			cpp_c99_str = "C++";
	}
	else cpp_c99_str = "C++";
	return;

	USAGE:
	fprintf(stderr,"Usage: %s\n"
	       "       -f <C/C++ file>  : Default = stdin\n"
	       "       -m <mode 1 to %d> : Default = %d\n"
	       "                             1 - Convert C comments to C++.\n"
	       "                             2 - Convert C++ comments to C.\n"
	       "                             3 - Output code (ie strip out comments).\n"
	       "                             4 - Output comments.\n"
	       "                             5 - Output comments with line numbers.\n"
	       "                             6 - Print a list of comment types and line numbers.\n"
	       "       -i               : In mode 1 with a multi line C comment, attempt to\n"
	       "                          indent the replacement C++ comments the same way.\n"
	       "                          Doesn't always get it right. No effect in other modes.\n"
	       "       -l               : In modes 4 and 5 print the entire line the comment is\n"
	       "                          on (ie include program text). No effect in other\n"
	       "                          modes.\n"
	       "       -x               : Extra info in mode:\n"
	       "                             5 - Print line numbers for every line of a multi\n"
	       "                                 line C comment instead of just the first.\n"
	       "                             6 - Print header and footer.\n"
	       "                          No effect in other modes.\n"
	       "       -v               : Print version and build info then exit.\n"
	       "Note:\n"
	       "   - All arguments are optional.\n"
	       "   - All output goes to stdout except errors which go to stderr.\n",
		argv[0],NUM_MODES,mode+1);
	exit(0);
}
