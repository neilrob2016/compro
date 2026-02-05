/****************************************************************************
 COMPRO
 A simple single pass, non tokenising parser that processes C and C++/
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
	option_x = false;

	for(int i=1;i < argc;++i)
	{
		if (argv[i][0] != '-' || strlen(argv[i]) != 2) goto USAGE;

		char c = argv[i][1];
		switch(c)
		{
		case 'h':
			goto USAGE;
		case 'l':
			option_l = true;
			continue;
		case 'x':
			option_x = true;
			continue;
		case 'v':
			puts("\n*** C/C++ Comments Processor ***\n");
			puts("Copyright (C) Neil Robertson 2026\n");
			printf("Version   : %s\n",VERSION);
			printf("Build date: %s, %s\n\n",__DATE__,__TIME__);
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
			if (mode < 1 || mode > NUM_MODES)
			{
				fprintf(stderr,"ERROR: Invalid mode \"%s\".\n",argv[i]);
				exit(1);
			}
			--mode;
			break;
		default:
			goto USAGE;
		}	
	}
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
	       "       -l               : Modified line processing in mode:\n"
	       "                             1 - With a multi line C comment attempt to indent\n"
	       "                                 the replacement C++ comments the same way.\n"
	       "                             2 - Merge multiple contiguous (seperated only by\n"
	       "                                 whitespace and a single newline) C++ comments\n"
	       "                                 into a single C comment.\n"
	       "                             3 - Don't output a blank line if there is only\n"
	       "                                 whitespace before a C++ comment on a line. Not\n"
	       "                                 implemented for C comments.\n"
	       "                             4 - Print the entire line the comment is on (ie\n"
	       "                                 include program text).\n"
	       "                             5 - As mode 4.\n"
	       "                             6 - No effect.\n"
	       "       -x               : Extra info in mode:\n"
	       "                             5 - Print line numbers for every line of a multi\n"
	       "                                 line C comment instead of just the first.\n"
	       "                             6 - Print header and footer.\n"
	       "                          No effect in other modes.\n"
	       "       -h               : Print this usage.\n"
	       "       -v               : Print version and build info then exit.\n"
	       "Note:\n"
	       "   - All arguments are optional.\n"
	       "   - All output goes to stdout except errors which go to stderr.\n",
		argv[0],NUM_MODES,mode+1);
	exit(0);
}
