# include <stdio.h>
# include <assert.h>
# include <stdlib.h>

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

# define pc reg[7]
# define REG 1
# define MEM 0
# define NO_PARAM 0
# define HAS_SS 1
# define HAS_DD (1 << 1)
# define HAS_NN (1 << 2)
# define HAS_XX (1 << 3)

# define LO(x) ((x) & 0xFF)
# define HI(x) (((x) >> 8) & 0xFF)


union offsetof
{
	char a;
	unsigned char b;
} xx, x;


union valu
{
	short int a;
	unsigned short int b;
} y;


void test_mem ();
  
word w_read  (adr a);
void w_write (adr a, word val);
byte b_read  (adr a);            
void b_write (adr a, byte val); 
           
void load_file ( );
void mem_dump (adr start, word n);
void mem_dump_all (adr start, word n);
void print_reg ();
struct P_Command create_command (word w);
void print_command (struct P_Command c);
void print_status_flags();

void run (adr pc0);

struct mr get_mode (word r, word mode, word b);
void get_nn (word w);
void get_xx (word w);

void do_halt (struct P_Command PC);
void do_mov (struct P_Command PC);
void do_movb (struct P_Command PC);
void do_add (struct P_Command PC);
void do_sob (struct P_Command PC);
void do_clr (struct P_Command PC);
void do_beq (struct P_Command PC);
void do_br (struct P_Command PC); 
void do_unknown (struct P_Command PC);


struct P_Command
{
	word w; // word 
	int B; // Byte
	word command; // command
	word mode_r1; //mode 1 operand 
	word r1; // 1 operand 
	word mode_r2; // mode 2 operand
	word r2; // 2 operand
};


struct mr 
{
	word ad;		// address
	word val;		// value
	word res;		// result
	word space; 	// address in mem[ ] or reg[ ]
} ss, dd, hh, nn;


struct Command
{
	word opcode;
	word mask;
	char * name;
	void (* func)(struct P_Command PC);
	byte param;
} commands [] = 
{
	{00,	0177777,		"halt",		do_halt,	NO_PARAM		},
	{0010000,	0170000,	"mov",		do_mov, 	HAS_SS | HAS_DD	},
	{0110000,	0170000,	"movb",		do_mov, 	HAS_SS | HAS_DD	},
	{0060000, 	0170000,	"add",		do_add,		HAS_DD | HAS_SS	},
	{077000,	0177000,	"sob",		do_sob,		HAS_NN			},
	{005000, 	0177000,	"clr",		do_clr,		HAS_DD			},
	{001400,	0xff00,		"beq",		do_beq,		HAS_XX			},
	{000400,	0xff00,		"br",		do_br,		HAS_XX			},
	{00,		0,			"unknown",	do_unknown,	NO_PARAM		}
};


void test_mem ()
{
    byte b0, b1;
    word w;
    b0 = 0x0a;
    b1 = 0x0b;
    b_write(2, b0);
    //printf ("%x", w)
    b_write(3, b1);
    w = w_read(2);
    printf ("%04x = %02x%02x\n", w, b1, b0);
    assert (w == 0x0a0b);
    //
    w = 0x0d0c;
    w_write (4, w);
    b0 = b_read (4);
    b1 = b_read (5);
    printf ("%04x = %02x%02x\n", w, b1, b0);
    assert (b0 == 0x0c);
    assert (b1 == 0x0d);
}


word w_read (adr a)
{
    word res;
    res = mem[a];
    res += mem[a + 1] * 256;
    return res;
}


void w_write (adr a, word val)
{
	mem[a] = (byte) val;
	mem[a + 1] = (byte) (val >> 8);
}


byte b_read (adr a)
{     
    return mem[a];
}


void b_write (adr a, byte val)
{
	mem[a] = val;
}


void load_file (char * file)
{
    unsigned int a;
    unsigned int b;
    unsigned int val;
    int i = 0;
    FILE *f = NULL;
    f = fopen(file, "r");
    if (f == NULL)
    {
		perror("file");
        exit (1);
    }
    while (fscanf (f, "%x%x", &a, &b) == 2)
    {
        for (i = a; i < (a + b); i++)
        {
            //fprintf(stderr, "%d: %s\n", __LINE__, __FUNCTION__);
            fscanf(f,"%x", & val);
            b_write (i,val);
        }
    }
    fclose (f);
    mem_dump (a, b);
}


void mem_dump (adr start, word n)
{
	mem_dump_all (start, n);
    assert (start % 2 == 0);
    FILE * f = fopen ("cmp.o", "w");
    int i = 0;
    for(i = 0; i < n; i = i + 2)
    {
        fprintf(f, "%06o : %06o\n", (start + i), w_read((adr)(start + i)));
    }
    fclose (f);
}


void mem_dump_all (adr start, word n)
{
	assert (start % 2 == 0);
    FILE * f = fopen ("all_mem", "w");
    int i = 0;
    for(i = 0; i < start + n; i = i + 2)
    {
        fprintf(f, "%06o : %06o\n", (i), w_read((adr)(i)));
    }
    fclose (f);
}


void print_reg ()
{
	int i = 0;
	FILE * f = fopen ("registers.txt", "w");
	fprintf (com, "Print registers\n");
	fprintf (f, "Print registers\n");
	for (i = 0; i < 8; i += 2)
	{
		fprintf(com, "reg[%d] = %o\t", i, reg[i]);
		fprintf(f, "reg[%d] = %o\t", i, reg[i]);
	}
	fprintf(com, "\n");
	fprintf(f, "\n");
	for (i = 1; i < 9; i += 2)
	{
		fprintf(com, "reg[%d] = %o\t", i, reg[i]);
		fprintf(f, "reg[%d] = %o\t", i, reg[i]);
	}
	fprintf(com, "\n");
	fprintf(f, "\n");
	fclose(f);
}


struct P_Command create_command (word w)
{
	struct P_Command c;
	c.w = w;
	c.B = w >> 15;
	c.command = (w >> 12) & 15;
	c.mode_r1 = (w >> 9) & 7;
	c.r1 = (w >> 6) & 7;
	c.mode_r2 = (w >> 3) & 7;
	c.r2 = w & 7;
	return c;
}


void print_command (struct P_Command c)
{
	printf ("w = %o\n", c.w);
	printf ("B = %d\n", c.B);
	printf ("r1 = %o\n", c.r1);
	printf ("mr1 = %o\n", c.mode_r1);
	printf ("r2 = %o\n", c.r2);
	printf ("mr2 = %o\n", c.mode_r2);
	printf ("com = %o\n", c.command);
}


void print_status_flags()
{
	fprintf (flag, "%x,\t%x,\t%x,\t%x\n", n, z, v, c);             
}


void run (adr pc0)
{
	pc = (word) pc0;
	int i = 0;
	com = fopen("com.txt", "w");
	flag = fopen ("status_flags.txt", "w"); 
	fprintf (com, "------------------------------running------------------------------\n");
	while (1)
	{
		word w = w_read(pc);
		//printf("%o\n", w);
		pc += 2;
		struct P_Command PC = create_command (w);
		//print_command (PC);
		//printf ("%06o : %06o \n", pc, w);
		for (i = 0; i <= sizeof(commands)/sizeof(struct Command); i++)
		{
			struct Command cmd = commands[i];
			if ((w & commands[i].mask) == commands[i].opcode)
			{
				fprintf (com, "%06o : %06o \t", pc - 2, w);
				fprintf (com, "%s ", cmd.name);
				if (cmd.param & HAS_SS)
				{
					ss = get_mode (PC.r1, PC.mode_r1, PC.B);
					fprintf (com, " , ");
				}
				if (cmd.param & HAS_DD)
				{
					dd = get_mode (PC.r2, PC.mode_r2, PC.B);
				}
				if (cmd.param & HAS_NN)
				{
					get_nn (w);
				}
				if (cmd.param & HAS_XX)
				{
					get_xx (w);
				}
				cmd.func(PC);
				print_status_flags();
				//print_reg ();
				break;
			}
		}
	}
	fclose(com);
	fclose (flag);  
}


void do_halt (struct P_Command PC)
{
	fprintf (com, "\n");
	fprintf (com, "------------------------------halted------------------------------\n");
	print_reg();
	fprintf(com, "print flags\n");
	fprintf(com, "n = %o,\tz = %o,\tv = %o,\tc = %o\n", n, z, v, c);
	exit (0);
}


void do_mov (struct P_Command PC)
{
	dd.res = ss.val;
	if (ss.val == 0)
	{
		z = 1;
	}
	else if (ss.val > 0)
	{
		n = 0;
		z = 0;
	}
	else 
	{
		n = 1;
		z = 0;
	}
	if (dd.space == REG)
	{
		reg[dd.ad] = dd.res;
	}
	else
	{
		w_write (dd.ad, dd.res);
	}
	fprintf (com, "\n");
}


void do_movb (struct P_Command PC)
{
	if (ss.val == 0)
	{
		z = 1;
	}
	else if (ss.val > 0)
	{
		n = 0;
		z = 0;
	}
	else 
	{
		n = 1;
		z = 0;
	}
	if (dd.space == REG)
	{
			reg[dd.ad] = dd.res;
	}
	else
	{
		b_write (dd.ad, (byte) dd.res);
	}
	fprintf (com, "\n");
}


void do_add (struct P_Command PC)
{
	dd.res = dd.val + ss.val;
	c = dd.res >> 15;
	if (ss.val > 0 && dd.val > 0 && dd.res < 0)
	{
		v = 1;
		n = 1;
	}
	else if (ss.val < 0 && dd.val < 0 && dd.res > 0)
	{
		v = 1;
		n = 1;
	}
	else if (dd.res == 0)
	{
		z = 1;
	}
	else if (dd.res > 0)
	{
		n = 0;
	}
	else 
	{
		n = 1;
	}
	if (dd.space == REG)
	{
		reg[dd.ad] = dd.res;
	}
	else
	{
		w_write (dd.ad, dd.res);
	}
	fprintf (com, "\n");
}


void do_unknown (struct P_Command PC)
{
	fprintf (com, "unknown\n");
	print_reg();
	exit (0);
}


void do_sob (struct P_Command PC)
{
	reg[nn.ad] --;
	if (reg[nn.ad] != 0)
	{
		reg[7] -= 2 * nn.val;
	}
	fprintf (com, "\n");
}


void do_clr (struct P_Command PC)
{
	n = 0;
	z = 1;
	v = 0;
	c = 0;
	w_write (dd.ad, 0);
	fprintf (com, "\n");
}


void do_beq (struct P_Command PC)
{
	if (z == 1)
		do_br (PC);
	else
	{
		fprintf (com, "%o", pc + 2 * xx.a);
		fprintf (com, "\n");
	}
}


void do_br (struct P_Command PC)
{
	pc += 2 * xx.a;
	fprintf (com, "%o", pc);
	fprintf (com, "\n");
}


struct mr get_mode (word r, word mode, word b)
{
	switch (mode)
	{
		case 0:
		{
			fprintf (com, "R%o", r);
			hh.ad = r;
			hh.val = reg[r];
			hh.space = REG;
			break;
		}
		case 1:
		{
			fprintf (com, "@R%o", r);
			hh.ad = reg[r];
			hh.val = w_read ((adr) reg[r]);
			hh.space = MEM;
			break;
		}
		case 2:
		{
			if (r == 7 || r == 6 || b == 0)
			{
				fprintf (com, "#%o", w_read ((adr) reg[r]));
				hh.ad = reg[r];
				hh.val = w_read ((adr) reg[r]);
				hh.space = MEM;
				reg[r] += 2;
			}
			else
			{
				fprintf (com, "(R%o)+", r);
				x.b = b_read ((adr) reg[r]);
				y.b = x.a;
				hh.ad =  reg[r];
				hh.val = y.a;
				hh.space = MEM;
				reg[r] ++;
			}
			break;
		}
		case 3:
		{
			if (r == 7 || r == 6 || b == 0)
			{
				fprintf (com, "@#%o", w_read((adr) (reg[r])));
				hh.ad = w_read ((adr) reg[r]);
				hh.val = w_read ((adr) w_read ((adr) (reg[r])));
				hh.space = MEM;
				reg[r] += 2;
			}
			else
			{
				fprintf (com, "@(R%o)+", r);
				x.b = b_read ((adr) w_read ((adr) (reg[r])));
				y.b = x.a;
				hh.ad = w_read ((adr) reg[r]);
				hh.val = y.a;
				hh.space = MEM;
				reg[r] ++;
			}
			break;
		}
		case 4:
		{
			fprintf (com, "-(R%o)", r);
			if (r == 7 || r == 6 || b == 0)
			{
				reg[r] -= 2;
				hh.ad = reg[r];
				hh.val = w_read ((adr) reg[r]);
				hh.space = MEM;
				break;
			}
			else 
			{
				reg[r] --;
				x.b = b_read ((adr) reg[r]);
				y.b = x.a;
				hh.ad = reg[r];
				hh.val = y.a;
				hh.space = MEM;
				break;
			}
		}
		case 5:
		{
			fprintf (com, "@-(R%o)", r);
			reg[r] -= 2;
			hh.ad = w_read ((adr) reg[r]);
			hh.val = w_read ((adr) w_read ((adr) (reg[r])));
			hh.space = MEM;
			break;
		}
	}
	return hh;
}


void get_nn (word w)
{
	nn.ad = (w >> 6) & 07;
	nn.val = w & 077;
	fprintf (com, "R%o , #%o", nn.ad, nn.val);
	//fprintf(com, "------\n%o\n------\n", w);
}


void get_xx (word w)
{
	xx.b = w & 0xff;
}


int main (int argc, char * argv[])
{
    //int i = 0;
    load_file(argv[1]);
    run(01000);
    return 0;
}
