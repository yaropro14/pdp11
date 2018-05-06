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
extern union valu y, k, res, d;
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
	//fprintf (com, "nooooo");
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
	//fprintf (com, "nooooo");
	x.b = (byte)ss.val;
	y.b = x.a;
	dd.res = y.a;
	if (y.a == 0)
	{
		z = 1;
	}
	else if (y.a > 0)
	{
		n = 0;
		z = 0;
	}
	else 
	{
		n = 1;
		z = 0;
	}
	if (dd.val == 0177566)
	{
		fprintf (com, " %c ", dd.res);
	}
	else if (dd.space == REG)
	{
			
			reg[dd.ad] = ss.val;
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
	if (dd.space == MEM)
		w_write (dd.ad, 0);
	else
		reg[dd.ad] = 0;
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


void do_bpl (struct P_Command PC)
{
	if (n == 0)
	{
		do_br (PC);
		//fprintf (com, "n");
	}
	else
	{
		fprintf (com, "%o", pc + 2 * xx.a);
		fprintf (com, "\n");
	}
}


void do_tst (struct P_Command PC)
{
	y.b = dd.val;
	n = y.a < 0 ? 1 : 0;
	z = y.a == 0 ? 1 : 0;
	v = 0; c = 0;
	fprintf (com, "\n");
}


void do_tstb (struct P_Command PC)
{
	fprintf (com, "hi\n");
	x.b = (byte) dd.val;
	n = x.a < 0 ? 1 : 0;
	z = x.a == 0 ? 1 : 0;
	v = 0; c = 0;
	fprintf (com, "\n");
}


void do_jsr (struct P_Command PC)
{
	fprintf (com, " , R%o\n", PC.r1);
    w_write(reg[6], reg[PC.r2]);
    reg[6]-= 2;
    reg[PC.r2] = reg[PC.r1];
    reg[PC.r1] = dd.val;
}


void do_rts (struct P_Command PC)
{
	fprintf (com, "R%o\n", PC.r2);
    //word dst = (PC.w & 7);
    reg[7] = reg[PC.r2];
    reg[6]+= 2;
    reg[PC.r2] = w_read(reg[6]);
}


void do_mul (struct P_Command PC)
{
	fprintf (com, "R%o\n", PC.r1);
	y.b = reg[PC.r1];
	k.b = dd.val;
	res.a = y.a * k.a;
	n = res.a < 0 ? 1 : 0;
    z = res.a == 0 ? 1 : 0;
    c = (res.a * res.a < k.a * k.a || res.a * res.a < y.a * y.a) ? 1 : 0;
    v = 0;
    if(PC.r1 % 2 == 0)
    {
        reg[PC.r1] = (word)(res.b >> 16);
        reg[PC.r1 + 1] = (word)((res.b << 16)>>16);
    }
    else
    {
        reg[PC.r1] = (word)((res.b << 16)>>16);
    }
}


void do_dec (struct P_Command PC)
{
	dd.val --;
    y.b = dd.val;
    n = y.a < 0 ? 1 : 0;
    z = y.a == 0 ? 1 : 0;
    v = (y.a+1) == 01000000 ? 1 : 0;
    if (dd.space == MEM)
		w_write(dd.ad, dd.val);
	else 
		reg[dd.ad] = dd.val;
    fprintf (com, "\n");
}


void do_div (struct P_Command PC)
{
    y.b = dd.val;
    if(y.a == 0)
    {
        c = 1;
        v = 1;
    }
    else
    {
		v = 0;
		fprintf(com, " R%o \n", PC.r1);
		if(PC.r1 % 2 == 0)
		{
			k.b = (reg[PC.r1]<<16) + reg[PC.r1+1];
			res.a = k.a / y.a;
			z = res.a == 0 ? 1 : 0;
			if(res.b > 0b1111111111111111) {
				v = 1;
			}
			reg[PC.r1] = res.b;
			reg[PC.r1+1] = (word)(k.a % y.a);
		}
		else
		{
			d.b = reg[PC.r1];
			res.a = d.a / y.a;
			k.a = d.a % y.a;
			reg[PC.r1] = k.b;
			if (res.b != 0)
				reg[PC.r1 - 1] = res.b;
			else 
			{
				reg[PC.r1 - 1] = 020000;
				//v = 1;
			}
		}
	}
}


void do_inc (struct P_Command PC)
{
    dd.val ++;
    y.b = dd.val;
    n = y.a < 0 ? 1 : 0;
    z = y.a == 0 ? 1 : 0;
    v = (y.a - 1) == 00777777 ? 1 : 0;
    if (dd.space == MEM)
    {
		w_write(dd.ad, dd.val);
	}
	else
	{
		reg[dd.ad] = dd.val;
	}
	fprintf (com, "\n");
}


void do_sub (struct P_Command PC)
{
	y.b = ss.val;
    k.b = dd.val;
    k.a = k.a - y.a;
    n = k.a < 0 ? 1 : 0;
    z = k.a == 0 ? 1 : 0;
    if (dd.space == MEM)
    {
		w_write(dd.ad, k.b);
	}
	else
	{
		reg[dd.ad] = k.b;
	}
    fprintf (com, "\n");
}


void do_cmp (struct P_Command PC)
{
	k.b = ss.val;
	y.b = dd.val;
	k.a -= y.a;
	ss.res = k.b;
	n = k.a < 0 ? 1 : 0;
	z = k.a == 0 ? 1 : 0;
	v = (y.a - 1) == 00777777 ? 1 : 0;
	fprintf (com, "\n");
}


void do_bne (struct P_Command PC)
{
	if (z == 0)
		do_br (PC);
	else
	{
		fprintf (com, "%o", pc + 2 * xx.a);
		fprintf (com, "\n");
	}
}


void do_jmp (struct P_Command PC)
{
	reg[7] = dd.val;
}


void do_acr (struct P_Command PC)
{
	y.b = dd.val;
	y.a = y.a / 2;
	dd.res = y.b;
	 if (dd.space == MEM)
    {
		w_write(dd.ad, dd.res);
	}
	else
	{
		reg[dd.ad] = dd.res;
	}
	fprintf (com, "\n");
}


void do_bmi (struct P_Command PC)
{
	if (n == 1)
	{
		do_br (PC);
		//fprintf (com, "n");
	}
	else
	{
		fprintf (com, "%o", pc + 2 * xx.a);
		fprintf (com, "\n");
	}
}


struct mr get_mode (word r, word mode, word b, word NN)
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
				y.b = w_read ((adr) reg[r]);
				hh.val = y.a;
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
				//hh.ad = pc;
				hh.ad = reg[r]; //w_read ((adr) reg[r]);
				hh.val = w_read (reg[r]); //((adr) w_read ((adr) (reg[r])));
				hh.space = MEM;
				reg[r] += 2;
			}
			else
			{
				fprintf (com, "@(R%o)+", r);
				x.b = b_read ((adr) reg[r]);
				y.b = x.a;
				hh.ad = reg[r];
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
		case 6:
		{
				y.b = w_read(reg[7]);
				if (r == 7)
				{
					hh.ad = 7;
					hh.val = reg[7] + 2 + w_read (reg[7]);
					reg[7] +=2;
					fprintf (com, "%o", hh.val);
				}
				else
				{
					fprintf (com, "%o(R%o)", w_read (reg[7]), r);
					hh.ad = r;
					hh.val = w_read ((adr) reg[r] + w_read(reg[7]));
					reg[7] += 2;
				}
				//fprintf (com, "?????%o??????\n",  hh.val);
				hh.space = REG;
				//reg[r] += 2;
				//fprintf (com, "\n%o/n",hh.val);
				break;
		}
		 case 7:
			fprintf (com, "@%o(R%o)", w_read (reg[7]), r);
            y.b = reg[7] + w_read(reg[7]);
            hh.ad = w_read(y.a);
            hh.val = w_read(hh.ad);
            break;
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
