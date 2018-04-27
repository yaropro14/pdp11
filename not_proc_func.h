#ifndef NOT_PROC_FUNC_H
#define NOT_PROC_FUNC_H


typedef unsigned char byte;
typedef unsigned short int word;
typedef word adr;


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


#endif
