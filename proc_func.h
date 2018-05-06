#ifndef PROC_FUNC_H
#define PROC_FUNC_H


# include "command.h"


# define pc reg[7]
# define REG 1
# define MEM 0


typedef unsigned char byte;
typedef unsigned short int word;
typedef word adr;


struct mr get_mode (word r, word mode, word b, word NN);
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
void do_bpl (struct P_Command PC);
void do_tst (struct P_Command PC);
void do_tstb (struct P_Command PC);
void do_jsr (struct P_Command PC);
void do_rts (struct P_Command PC);
void do_mul (struct P_Command PC);
void do_dec (struct P_Command PC);
void do_div (struct P_Command PC);
void do_inc (struct P_Command PC);
void do_sub (struct P_Command PC);
void do_bne (struct P_Command PC);
void do_cmp (struct P_Command PC);
void do_jmp (struct P_Command PC);
void do_acr (struct P_Command PC);
void do_bmi (struct P_Command PC);
void do_unknown (struct P_Command PC);


#endif

