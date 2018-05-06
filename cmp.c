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
# define COMMANDS_LEN 25


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
union valu y, k, res, d;
struct mr ss, dd, hh, nn;


struct Command commands[] = 
{
	{00,		0177777,	"halt",		do_halt,	NO_PARAM		},
	{0010000,	0170000,	"mov",		do_mov, 	HAS_SS | HAS_DD	},
	{0110000,	0170000,	"movb",		do_movb, 	HAS_SS | HAS_DD	},
	{0060000, 	0170000,	"add",		do_add,		HAS_DD | HAS_SS	},
	{077000,	0177000,	"sob",		do_sob,		HAS_NN			},
	{005000, 	0177700,	"clr",		do_clr,		HAS_DD			},
	{001400,	0xff00,		"beq",		do_beq,		HAS_XX			},
	{000400,	0xff00,		"br",		do_br,		HAS_XX			},
	{0100000,	0177400,	"bpl",		do_bpl,		HAS_XX			},
	{005700,	0177700,	"tst",		do_tst,		HAS_DD			},
	{0105700,	0177700,	"tstb",		do_tst,		HAS_DD			},
	{004000,	0177000,	"jsr",		do_jsr,		HAS_DD			},
	{0000200,	0177770,	"rts",		do_rts,		NO_PARAM		},
	{0070000,	0177000,	"mul",		do_mul,		HAS_DD			},
	{0005300,	0177700, 	"dec",		do_dec,		HAS_DD			},
	{0071000,	0177000, 	"div",		do_div,		HAS_DD			},
	{0071000,	0177000, 	"div",		do_div,		HAS_DD			},
	{0005200,	0177700, 	"inc",		do_inc,		HAS_DD			},
	{0160000,	0170000,  	"sub",		do_sub,		HAS_DD | HAS_SS	},
	{001000,	0xff00,		"bne",		do_bne,		HAS_XX			},
	{020000,	0170000,	"cmp",		do_cmp,		HAS_DD | HAS_SS	},
	{000100,	0177700,	"jmp",		do_jmp,		HAS_DD | HAS_SS	},
	{006200,	0177700, 	"acr",		do_acr,		HAS_DD			},
	{0100400,	0xff00,		"bmi",		do_bmi,		HAS_XX			},
	{00,		0,			"unknown",	do_unknown,	NO_PARAM		}
};


int main (int argc, char * argv[])
{
    
    //int i = 0;
    load_file(argv[1]);
    run(01000);
    return 0;
}
