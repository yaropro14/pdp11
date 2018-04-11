# include <stdio.h>
# include <assert.h>
# include <stdlib.h>


typedef unsigned char byte;
typedef unsigned short int word;
typedef word adr;


word mem [64 * 1024];
word reg [8];


//# define pc reg [7];


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
void run (adr pc0);
void do_halt (word ss, word dd);
void do_mov (word ss, word dd); // ?
void do_add (word ss, word dd); // ?
void do_unknown (word ss, word dd);
void do_sob (word ss, word dd);// ??????? 
word get_ss (word w);// ?
word get_dd (word w);// ?
word get_nn (word w);
// word get_xx (word w);
word mode_0 (int i);
word mode_1 (int i);
word mode_2 (int i);
word mode_3 (int i);
word mode_4 (int i);
word mode_5 (int i);


struct Command
{
	word opcode;
	word mask;
	char * name;
	void (* func)(word ss, word dd);
	byte param;
} commands [] = 
{
	{0,			0177777,	"halt",		do_halt,	NO_PARAM		},
	{0010000,	0170000, 	"mov",		do_mov, 	HAS_SS | HAS_DD	},
	{0060000,	0170000, 	"add",		do_add,		HAS_DD | HAS_SS	},
	{077000,	0177000,	"sob",		do_sob,		HAS_NN		},
	{0,			0,			"unknown",	do_unknown,	NO_PARAM		}
};


//.............................Modes................................
struct Modes
{
	adr (* fun)(int i);
} mode[] = {{mode_0}, {mode_1}, {mode_2}, {mode_3}, {mode_4}, {mode_5}};
//..................................................................


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
    return mem[a];
}


void w_write (adr a, word val)
{
    mem[a] = val;
}


byte b_read (adr a)
{   
    byte res;
    if (a % 2 == 0)
    {
    res = LO(mem[a]);
    }
    else
    {
    res = HI (mem[a - 1]);
    a = a - 1;
    }
    return res;
}


void b_write (adr a, byte val)
{
    word b = 0;
    if (a % 2 == 0)
	{
        mem[a] |= val;
	}
    else if (a % 2 != 0)
    {
        b = val;
        b = (b << 8);
        mem[a - 1] = mem[a - 1] | b;
	}
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
    run(a);
}


void mem_dump (adr start, word n)
{
    assert (start % 2 == 0);
    int i = 0;
    for(i = 0; i < n; i = i + 2)
    {
        printf("%06o : %06o\n", (start + i), mem[start + i]);
    }
}


void run (adr pc0)
{
	reg[7] = pc0;
	int i = 0;
	word ss = 0, dd = 0, nn = 0;// xx = 0;
	while (1)
	{
		word w = w_read(reg[7]);
		//printf ("%06o 5 %06o \n", pc, w);
		reg[7] += 2;
		for (i = 0; i <= 3; i++)
		{
			struct Command cmd = commands[i];
			if ((w & cmd.mask) == cmd.opcode)
			{
				// args
				if (cmd.param & HAS_NN)
				{
					nn = get_nn (w);
				}
				if (cmd.param & HAS_SS)
				{
					ss = get_ss (w);
				}
				if (cmd.param & HAS_DD)
				{
					dd = get_dd (w);
				}
				printf("%o : ", w);
				cmd.func(ss, dd);
			}
		}
	}
}


void do_halt (word ss, word dd)
{
	printf ("halt\n");
	exit (0);
}


void do_mov (word ss, word dd)
{
	word SS = 0;
	SS = w_read(mode[(ss >> 1) & 1].fun(ss & 1));
	w_write (mode[(dd >> 1) & 1].fun(dd & 1), SS);
	printf ("mov\n SS = %06o \n", SS);
}


void do_add (word ss, word dd)
{
	word SS = 0, DD = 0;
	SS = w_read(mode[(ss >> 1) & 1].fun(ss & 1));
	DD = w_read(mode[(dd >> 1) & 1].fun(dd & 1));
	w_write (mode[(dd >> 1) & 1].fun(dd & 1), SS + DD);
	printf ("add\n SS = %06o, DD = %06o\n", SS, DD);
}


void do_unknown (word ss, word dd)
{
	printf ("unknown\n");
}


void do_sob (word ss, word dd) // ????????
{
	//return w_read (ss);// ???????
}


word get_ss (word w)
{
	return w & 077;
}

word get_dd (word w)
{
	return w & 07700;
}


word get_nn (word w)
{
	return w & 077;
}


adr mode_0 (int i)
{
	return reg[i];
}


adr mode_1 (int i)
{
	return reg[i];
}


adr mode_2 (int i)
{
	reg[i] += 2;
	return (reg[i] - 2);
}


adr mode_3 (int i)
{
	adr ad_op = w_read (reg[i]);
	reg[i] += 2;
	return ad_op;
}


adr mode_4 (int i)
{
	reg[i] -= 2;
	return reg[i];
}


adr mode_5 (int i)
{
	reg[i] -= 2;
	adr ad_op = w_read (reg[i]);
	return ad_op;
}


int main (int argc, char * argv[])
{
    //int i = 0;
    load_file(argv[1]);
    //run(01000);
    return 0;
}
