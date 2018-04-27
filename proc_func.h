#ifndef PROC_FUNC_H
#define PROC_FUNC_H


# include "command.h"


# define pc reg[7]
# define REG 1
# define MEM 0


typedef unsigned char byte;
typedef unsigned short int word;
typedef word adr;


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


#endif

