#include "not_proc_func.h"
#include "command.h"
#include "proc_func.h"


# include <stdio.h>
# include <assert.h>
# include <stdlib.h>


# define NO_PARAM 0
# define HAS_SS 1
# define HAS_DD (1 << 1)
# define HAS_NN (1 << 2)
# define HAS_XX (1 << 3)
# define COMMANDS_LEN 23


extern byte mem [64 * 1024];
extern word reg [8];
extern byte n;
extern byte z;
extern byte v;
extern byte c;


extern FILE * com;
extern FILE * flag; 


extern union offsetof xx, x;
extern union valu y;
extern struct mr ss, dd, hh, nn;


extern struct Command commands[];


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
	fprintf (com, "%x,\t%x,\t%x,\t%x\n", n, z, v, c);             
}


void run (adr pc0)
{
	pc = (word) pc0;
	int i = 0;
	com = fopen("com.txt", "w");
	flag = fopen ("status_flags.txt", "w"); 
	fprintf (com, "------------------------------");
	fprintf (com, "running");
	fprintf (com, "------------------------------\n");
	while (pc < 17777)
	{
		//fprintf (com, "%o\n", pc);
		word w = w_read(pc);
		//printf("%o\n", w);
		pc += 2;
		struct P_Command PC = create_command (w);
		//print_command (PC);
		//printf ("%06o : %06o \n", pc, w);
		for (i = 0; i <= COMMANDS_LEN; i++)
		{
			struct Command cmd = commands[i];
			if ((w & commands[i].mask) == commands[i].opcode)
			{
				word NN = w_read(pc);
				//fprintf (com, "NN = %o\n", NN);
				//fprintf (com, "pc = %o\n", pc);
				fprintf (com, "%06o : %06o \t", pc - 2, w);
				fprintf (com, "%s\t", cmd.name);
				if (cmd.param & HAS_SS)
				{
					ss = get_mode (PC.r1, PC.mode_r1, PC.B, NN);
					//fprintf (com, "\n%o/n",w_read((adr) (reg[7])));    /////////
					fprintf (com, " , ");
				}
				if (cmd.param & HAS_DD)
				{
					//fprintf (com, "\n%o/n",w_read((adr) (reg[7])));    ////////////
					dd = get_mode (PC.r2, PC.mode_r2, PC.B, NN);
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
				mem_dump_all (1000, 1000);
				//fprintf (com, "\n%o/n",w_read((adr) (reg[7])));
				print_reg ();
				//print_status_flags();
				break;
			}
		} 
	}
	fclose(com);
	fclose (flag); 
}
