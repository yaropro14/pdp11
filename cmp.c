# include <stdio.h>
# include <assert.h>
# include <stdlib.h>

typedef unsigned char byte;
typedef unsigned short int word;
typedef word adr;

byte mem [64 * 1024];
word reg [8];
FILE * com;

# define pc reg[7]
# define REG 1
# define NO_PARAM 0
# define HAS_SS 1
# define HAS_DD (1 << 1)
# define HAS_NN (1 << 2)
# define HAS_XX (1 << 3)

# define LO(x) ((x) & 0xFF)
# define HI(x) (((x) >> 8) & 0xFF)


void test_mem ();
  
word w_read  (adr a);
void w_write (adr a, word val);
byte b_read  (adr a);            
void b_write (adr a, byte val); 
           
void load_file ( );
void mem_dump (adr start, word n);
void print_reg ();
struct P_Command create_command (word w);
void print_command (struct P_Command c);
void run (adr pc0);

void get_ss (word r, word mode, word b);
void get_dd (word r, word mode, word b);

void do_halt (struct P_Command PC); // ?
void do_mov (struct P_Command PC); // ?
void do_add (struct P_Command PC); // ?
void do_unknown (struct P_Command PC);
void do_sob (struct P_Command PC);// ??????? 


struct P_Command
{
	word w;
	int B;
	word command;
	word mode_r1;
	word r1;
	word mode_r2;
	word r2;
};


struct mr 
{
	word ad;		// address
	word val;		// value
	word res;		// result
	word space; 	// address in mem[ ] or reg[ ]
} ss, dd;


struct Command
{
	word opcode;
	char * name;
	void (* func)(struct P_Command PC);
	byte param;
} commands [] = 
{
	{00,	"halt",		do_halt,	NO_PARAM		},
	{001,	"mov",		do_mov, 	HAS_SS | HAS_DD	},
	{006, 	"add",		do_add,		HAS_DD | HAS_SS	},
	{077,	"sob",		do_sob,		HAS_NN			},
	{00,	"unknown",	do_unknown,	NO_PARAM		}
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
    assert (start % 2 == 0);
    FILE * f = fopen ("cmp.o", "w");
    int i = 0;
    for(i = 0; i < n; i = i + 2)
    {
        fprintf(f, "%06o : %06o\n", (start + i), w_read((adr)(start + i)));
    }
    fclose (f);
}


void print_reg ()
{
	int i = 0;
	FILE * f = fopen ("registers.txt", "w");
	fprintf (f, "Print registers\n");
	for (i = 0; i < 8; i ++)
	{
		fprintf(f, "reg[%d] = %o\n", i, reg[i]);
	}
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


void run (adr pc0)
{
	pc = (word) pc0;
	int i = 0;
	com = fopen("com.txt", "w");
	while (1)
	{
		word w = w_read(pc);
		//printf("%o\n", w);
		struct P_Command PC = create_command (w);
		//print_command (PC);
		//printf ("%06o : %06o \n", pc, w);
		pc += 2;
		for (i = 0; i <= 5; i++)
		{
			struct Command cmd = commands[i];
			if (PC.command == commands[i].opcode)
			{
				//printf("%o : ", PC.w);
				cmd.func(PC);
			}
		}
	}
	fclose(com);
}


void do_halt (struct P_Command PC)
{
	fprintf (com, "halt\n");
	print_reg();
	exit (0);
}


void do_mov (struct P_Command PC)
{
	fprintf (com, "mov ");
	get_ss (PC.r1, PC.mode_r1, PC.B);
	fprintf (com, " , ");
	get_dd (PC.r2, PC.mode_r2, PC.B);
	dd.res = ss.val;
	if (dd.space == REG)
	{
		reg[dd.ad] = dd.res;
	}
	else
	{
		w_write (dd.space, dd.res);
	}
	fprintf (com, "\n");
}


void do_add (struct P_Command PC)
{
	fprintf (com, "add ");
	get_ss (PC.r1, PC.mode_r1, PC.B);
	fprintf (com, " , ");
	get_dd (PC.r2, PC.mode_r2, PC.B);
	dd.res = dd.val + ss.val;
	if (dd.space == REG)
	{
		reg[dd.ad] = dd.res;
	}
	else
	{
		w_write (dd.space, dd.res);
	}
	fprintf (com, "\n");
}


void do_unknown (struct P_Command PC)
{
	fprintf (com, "unknown\n");
}


void do_sob (struct P_Command PC) // ????????
{
	//return w_read (ss);// ???????
}


void get_ss (word r, word mode, word b)
{
	switch (mode)
	{
		case 0:
		{
			fprintf (com, "R%o", r);
			ss.ad = r;
			ss.val = reg[r];
			ss.space = REG;
			break;
		}
		case 1:
		{
			fprintf (com, "@R%o", r);
			ss.ad = r;
			ss.val = w_read ((adr) reg[r]);
			ss.space = reg[r];
			break;
		}
		case 2:
		{
			fprintf (com, "#%o", w_read ((adr) reg[r]));
			if (r == 7 || r == 6 || b == 0)
			{
				ss.ad = r;
				ss.val = w_read ((adr) reg[r]);
				ss.space = reg[r];
				reg[r] += 2;
			}
			else
			{
				ss.ad = r;
				ss.val = b_read ((adr) reg[r]);
				ss.space = reg[r];
				reg[r] ++;
			}
			break;
		}
		case 3:
		{
			fprintf (com, "@#%o", w_read((adr) (reg[r])));
			if (r == 7 || r == 6 || b == 0)
			{
				ss.ad = r;
				ss.val = w_read ((adr) w_read ((adr) (reg[r])));
				ss.space = w_read ((adr) reg[r]);
				reg[r] += 2;
			}
			else
			{
				ss.ad = r;
				ss.val = b_read ((adr) w_read ((adr) (reg[r])));
				ss.space = w_read ((adr) reg[r]);
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
				ss.ad = r;
				ss.val = w_read ((adr) reg[r]);
				ss.space = reg[r];
				break;
			}
			else 
			{
				reg[r] --;
				ss.ad = r;
				ss.val = b_read ((adr) reg[r]);
				ss.space = reg[r];
				break;
			}
		}
		case 5:
		{
			fprintf (com, "-(R%o)", r);
			reg[r] -= 2;
			ss.ad = r;
			ss.val = w_read ((adr) w_read ((adr) (reg[r])));
			ss.space = w_read ((adr) reg[r]);
			break;
		}
	}
}


void get_dd (word r, word mode, word b)
{
	switch (mode)
	{
		case 0:
		{
			fprintf (com, "R%o", r);
			dd.ad = r;
			dd.val = reg[r];
			dd.space = REG;
			break;
		}
		case 1:
		{
			fprintf (com, "@R%o", r);
			dd.ad = r;
			dd.val = w_read ((adr) reg[r]);
			dd.space = reg[r];
			break;
		}
		case 2:
		{
			fprintf (com, "#%o", w_read ((adr) reg[r]));
			if (r == 7 || r == 6 || b == 0)
			{
				dd.ad = r;
				dd.val = w_read ((adr) reg[r]);
				dd.space = reg[r];
				reg[r] += 2;
			}
			else
			{
				dd.ad = r;
				dd.val = b_read ((adr) reg[r]);
				dd.space = reg[r];
				reg[r] ++;
			}
			break;
		}
		case 3:
		{
			fprintf (com, "@#%o", w_read((adr) (reg[r])));
			if (r == 7 || r == 6 || b == 0)
			{
				dd.ad = r;
				dd.val = w_read ((adr) w_read ((adr) (reg[r])));
				dd.space = w_read ((adr) reg[r]);
				reg[r] += 2;
			}
			else
			{
				dd.ad = r;
				dd.val = b_read ((adr) w_read ((adr) (reg[r])));
				dd.space = w_read ((adr) reg[r]);
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
				dd.ad = r;
				dd.val = w_read ((adr) reg[r]);
				dd.space = reg[r];
			}
			else 
			{
				reg[r] --;
				dd.ad = r;
				dd.val = b_read ((adr) reg[r]);
				dd.space = reg[r];
			}
			break;
		}
		case 5:
		{
			fprintf (com, "-(R%o)", r);
			reg[r] -= 2;
			dd.ad = r;
			dd.val = w_read ((adr) w_read ((adr) (reg[r])));
			dd.space = w_read ((adr) reg[r]);
			break;
		}
	}
}


int main (int argc, char * argv[])
{
    //int i = 0;
    load_file(argv[1]);
    run(01000);
    return 0;
}
