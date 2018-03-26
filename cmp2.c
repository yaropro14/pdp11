# include <stdio.h>
# include <assert.h>

typedef unsigned char byte;
typedef unsigned short int word;
typedef word adr;

word mem [64 * 1024];

# define LO(x) ((x) & 0xFF)
# define HI(x) (((x) >> 8) & 0xFF)

byte b_read  (adr a);            // читает из "старой памяти" mem байт с "адресом" a.
void b_write (adr a, byte val);  // пишет значение val в "старую память" mem в байт с "адресом" a.
word w_read  (adr a);            // читает из "старой памяти" mem слово с "адресом" a.
void w_write (adr a, word val);  // пишет значение val в "старую память" mem в слово с "адресом" a.
void load_file( );

word w_read  (adr a)
{
    return mem[a];
}

void w_write (adr a, word val)
{
    mem[a] = val;
}

byte b_read  (adr a)
{   // w_write (4, 0x0d0c)
    byte res;
    if (a % 2 == 0)
    {
    res = LO(mem[a]);
    // b_read(4) = 0x0c
    }
    else
    {
    // b_read(5) = 0x0d
    res = HI (mem[a - 1]);
    a = a - 1;
    //printf ("mem[%d] = %04x res = %02x HI = %02x\n", a, res, HI(mem[a]));
    }
    return res;
}

void b_write (adr a, byte val)
{
    word b = 0;
    if (a % 2 == 0)
    {
        mem[a] |= val;
        //printf("%04x\n", mem[a]);
    }
    else if (a % 2 != 0)
    {
        b = val;
        b = (b << 8);
        mem[a - 1] = mem[a - 1] | b;
        //printf ("%04x %04x %04x \n", val, b, mem[a - 1]);
    }
    //printf ("%04x\n", mem[a - 1]);
}

void mem_dump(adr start, word n);

void load_file( )
{
    unsigned int a;
    unsigned int b;
    unsigned int val;
    int i = 0;
    FILE *f = stdin;
    while (fscanf (f, "%x%x", &a, &b) == 2)
    {
        for (i = a; i < (a + b); i++)
        {
            //fprintf(stderr, "%d: %s\n", __LINE__, __FUNCTION__);
            fscanf(f,"%x", & val);
            //fprintf(stderr, "%d: %s\n", __LINE__, __FUNCTION__);
            b_write (i,val);
            //fprintf(stderr, "%d: %s\n", __LINE__, __FUNCTION__);
        }
        //fprintf(stderr, "%d: %s\n", __LINE__, __FUNCTION__);
    }
}

void mem_dump(adr start, word n)
{
    assert (start % 2 == 0);
    int i = 0;
    for(i = 0; i < n; i = i + 2)
    {
        printf("%06o : %06o\n", (start + i), mem[start + i]);
    }
}

void test_mem()
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
    /
    w = 0x0d0c;
    w_write (4, w);
    b0 = b_read (4);
    b1 = b_read (5);
    printf ("%04x = %02x%02x\n", w, b1, b0);
    */
    //assert (b0 == 0x0c);
    //assert (b1 == 0x0d);
}

int main ()
{
load_file( );
return 0;
}
