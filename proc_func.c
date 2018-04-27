# include "proc_func.h"
# include "command.h"
# include "not_proc_func.h"


# include <stdio.h>
# include <assert.h>
# include <stdlib.h>


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


void do_halt (struct P_Command PC)
{
	fprintf (com, "\n");
	fprintf (com, "------------------------------");
	fprintf (com, "halted");
	fprintf (com, "------------------------------\n");
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
