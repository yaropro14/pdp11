# include <stdio.h>
# include <assert.h>
# include <stdlib.h>


typedef unsigned char byte;
typedef unsigned short int word;
typedef word adr;


word mem [64 * 1024];


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
void do_halt ();
void do_mov ();
void do_add ();
void do_unknown();


struct Command
{
	word opcode;
	word mask;
	char * name;
	void (* func)();
} commands [] = 
{
	{0,			0177777, 	"halt",		do_halt},
	{0010000,	0170000, 	"mov",		do_mov},
	{0060000,	0170000, 	"add",		do_add},
	{0,			0,			"unknown",	do_unknown}
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
	adr pc = pc0;
	int i = 0;
	while (1)
	{
		word w = w_read(pc);
		//printf ("%06o 5 %06o \n", pc, w);
		pc += 2;
		for (i = 0; i <= 3; i++)
		{
			struct Command cmd = commands[i];
			if ((w & cmd.mask) == cmd.opcode)
			{
				//printf("%s ", cmd.name);
				cmd.func();
			}
		}
	}
}


void do_halt ()
{
	printf ("halt");
	exit (0);
}


void do_mov ()
{
	printf ("mov");
}


void do_add ()
{
	printf ("add");
}


void do_unknown ()
{
	printf ("unknown");
}


int main (int argc, char * argv[])
{
    int i = 0;
    load_file(argv[1]);
    run(01000);
    return 0;
}
