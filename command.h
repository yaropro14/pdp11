#ifndef COMMAND_H
#define COMMAND_H


typedef unsigned char byte;
typedef unsigned short int word;
typedef word adr;


union offsetof
{
	char a;
	unsigned char b;
};


union valu
{
	short int a;
	unsigned short int b;
};


struct mr 
{
	word ad;		// address
	word val;		// value
	word res;		// result
	word space; 	// address in mem[ ] or reg[ ]
};


struct P_Command
{
	word w; // word 
	int B; // Byte
	word command; // command
	word mode_r1; //mode 1 operand 
	word r1; // 1 operand 
	word mode_r2; // mode 2 operand
	word r2; // 2 operand
};


struct Command
{
	word opcode;
	word mask;
	char * name;
	void (* func)(struct P_Command PC);
	byte param;
};


#endif
