# include <stdio.h>
# include <assert.h>
# include <stdlib.h>


typedef unsigned char byte;
typedef unsigned short int word;
typedef word adr;


byte mem [64 * 1024];
word reg [8];

# define pc reg[7]


# define NO_PARAM 0
# define HAS_SS 1
# define HAS_DD (1 << 1)
# define HAS_NN (1 << 2)
# define HAS_XX (1 << 3)


# define LO(x) ((x) & 0xFF)
# define HI(x) (((x) >> 8) & 0xFF)


void test_mem ();
byte b_read  (adr a);            
void b_write (adr a, byte val);  
word w_read  (adr a);
void w_write (adr a, word val);            
void load_file ( );
void mem_dump (adr start, word n);
struct P_Command create_command (word w);
void run (adr pc0);
word mode_take (word r, word mode, word b);
word mode_take_np (word r, word mode, word b);
void mode_push_dd (word r, word mode, word b, word val);
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
    //mem_dump (a, b);
}


void mem_dump (adr start, word n)
{
    assert (start % 2 == 0);
    int i = 0;
    for(i = 0; i < n; i = i + 2)
    {
        printf("%06o : %06o\n", (start + i), w_read((adr)(start + i)));
    }
}


void print_reg ()
{
	int i = 0;
	printf ("Print registers\n");
	for (i = 0; i < 8; i ++)
	{
		printf("reg[%d] = %o\n", i, reg[i]);
	}
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
}


void do_halt (struct P_Command PC)
{
	printf ("halt\n");
	print_reg();
	exit (0);
}


void do_mov (struct P_Command PC)
{
	word ss = 0;
	printf ("mov ");
	ss = mode_take (PC.r1, PC.mode_r1, PC.B);
	printf (" , ");
	mode_push_dd (PC.r2, PC.mode_r2, PC.B, ss);
	printf ("\n");
}


void do_add (struct P_Command PC)
{
	word ss = 0, dd = 0;
	printf ("add ");
	ss = mode_take (PC.r1, PC.mode_r1, PC.B);
	dd = mode_take_np (PC.r2, PC.mode_r2, PC.B);
	printf (" , ");
	mode_push_dd (PC.r2, PC.mode_r2, PC.B, (ss + dd));
	printf ("\n");
}


void do_unknown (struct P_Command PC)
{
	printf ("unknown\n");
}


void do_sob (struct P_Command PC) // ????????
{
	//return w_read (ss);// ???????
}


word mode_take (word r, word mode, word b)
{
	switch (mode)
	{
		case 0:
		{
			printf ("R%o", r);
			return reg[r];
		}
		case 1:
		{
			printf ("@R%o", r);
			return w_read ((adr) reg[r]);
		}
		case 2:
		{
			printf ("#%o", w_read ((adr) reg[r]));
			if (r == 7 || r == 6 || b == 0)
			{
				reg[r] += 2;
				return w_read ((adr) (reg[r] - 2));
			}
			else
			{
				reg[r] ++;
				return b_read ((adr) (reg[r] - 1));
			}
		}
		case 3:
		{
			printf ("@#%o", w_read((adr) (reg[r])));
			if (r == 7 || r == 6 || b == 0)
			{
				reg[r] += 2;
				return w_read ((adr) w_read ((adr) (reg[r] - 2)));
			}
			else
			{
				reg[r] ++;
				return b_read ((adr) w_read ((adr) (reg[r] - 1)));
			}
		}
		case 4:
		{
			printf ("-(R%o)", r);
			reg[r] -= 2;
			return w_read ((adr) reg[r]);
			
		}
		case 5:
		{
			printf ("-(R%o)", r);
			printf ("-(R%o)", r);
			reg[r] -= 2;
			return w_read ((adr) w_read ((adr) reg[r]));
		}
	}
	return 0;
}


word mode_take_np (word r, word mode, word b)
{
	switch (mode)
	{
		case 0:
		{
			return reg[r];
		}
		case 1:
		{
			return w_read ((adr) reg[r]);
		}
		case 2:
		{
			if (r == 7 || r == 6 || b == 0)
			{
				reg[r] += 2;
				return w_read ((adr) (reg[r] - 2));
			}
			else
			{
				reg[r] ++;
				return b_read ((adr) (reg[r] - 1));
			}
		}
		case 3:
		{
			if (r == 7 || r == 6 || b == 0)
			{
				reg[r] += 2;
				return w_read ((adr) w_read ((adr) (reg[r] - 2)));
			}
			else
			{
				reg[r] ++;
				return b_read ((adr) w_read ((adr) (reg[r] - 1)));
			}
		}
		case 4:
		{
			reg[r] -= 2;
			return w_read ((adr) reg[r]);
			
		}
		case 5:
		{
			reg[r] -= 2;
			return w_read ((adr) w_read ((adr) reg[r]));
		}
	}
	return 0;
}


void mode_push_dd (word r, word mode, word b, word val)
{
	switch (mode)
	{
		case 0:
			printf ("R%o ", r);
			reg[r] = val;
			break;
		case 1:
			printf ("@R%o ", r);
			w_write ((adr) reg[r], val);
			break;
		case 2: //								????????
			if (r == 7 || r == 6 || b == 0)
			{
				printf ("#%o ", w_read((adr) reg[r]));
				w_write (reg[r], val);
				reg[r] += 2;
				break;
			}
			else 
			{
				printf ("#R%o ", r);
				w_write (reg[r], val);
				reg[r] ++;
				break;
			}
		case 3: //								????????
			reg[r] += 2;
			if (r == 7 || r == 6 || b == 0)
			{
				printf ("#R%o ", r);
				w_write (reg[r], val);
				reg[r] += 2;
			}
			else 
			{
				printf ("#R%o ", r);
				w_write (reg[r], val);
				reg[r] ++;
			}
			break;
		case 4:
		{
			printf ("-(R%o) ", r);
			reg[r] -= 2;
			w_write ((adr) reg[r], val);
			break;
		}
		case 5:
		{
			printf ("@-(R%o) ", r);
			reg[r] -= 2;
			w_write ((adr) w_read((adr) reg[2]), val);
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
