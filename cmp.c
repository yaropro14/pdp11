# include "command.h"
# include "proc_func.h"
# include "not_proc_func.h"


# include <stdio.h>
# include <assert.h>
# include <stdlib.h>


# define NO_PARAM 0
# define HAS_SS 1
# define HAS_DD (1 << 1)
# define HAS_NN (1 << 2)
# define HAS_XX (1 << 3)
# define COMMANDS_LEN 9


typedef unsigned char byte;
typedef unsigned short int word;
typedef word adr;


byte mem [64 * 1024];
word reg [8];
byte n;
byte z;
byte v;
byte c;


FILE * com;
FILE * flag; 


union offsetof xx, x;
union valu y;
struct mr ss, dd, hh, nn;


struct Command commands[] = 
{
	{00,		0177777,	"halt",		do_halt,	NO_PARAM		},
	{0010000,	0170000,	"mov",		do_mov, 	HAS_SS | HAS_DD	},
	{0110000,	0170000,	"movb",		do_mov, 	HAS_SS | HAS_DD	},
	{0060000, 	0170000,	"add",		do_add,		HAS_DD | HAS_SS	},
	{077000,	0177000,	"sob",		do_sob,		HAS_NN			},
	{005000, 	0177000,	"clr",		do_clr,		HAS_DD			},
	{001400,	0xff00,		"beq",		do_beq,		HAS_XX			},
	{000400,	0xff00,		"br",		do_br,		HAS_XX			},
	{00,		0,			"unknown",	do_unknown,	NO_PARAM		}
};


int main (int argc, char * argv[])
{
    
    //int i = 0;
    load_file(argv[1]);
    run(01000);
    return 0;
}
