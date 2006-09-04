#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/*===================================================================================
 * Generates a md5 hash of a random 65k length string n times
 *==================================================================================*/

#define UINT_MAX 4294967295U	/* highest number for unsigned int */
#define LEN 65535	/* length of input string */

static unsigned int *pX;	/* global buffer for calculations */

/* initialization table */
static const unsigned int T[] = {
   0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
   0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
   0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
   0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
   0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
   0xd62f105d,  0x2441453, 0xd8a1e681, 0xe7d3fbc8,
   0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
   0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
   0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
   0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
   0x289b7ec6, 0xeaa127fa, 0xd4ef3085,  0x4881d05,
   0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
   0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
   0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
   0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
   0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

/* rotates x for n-bits */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* misc. functions */
static unsigned int F(unsigned int X, unsigned int Y, unsigned int Z)
{
   return (X & Y) | (~X & Z);
}
static unsigned int G(unsigned int X, unsigned int Y, unsigned int Z)
{
   return (X & Z) | (Y & ~Z);
}
static unsigned int H(unsigned int X, unsigned int Y, unsigned int Z)
{
   return X ^ Y ^ Z;
}
static unsigned int I(unsigned int X, unsigned int Y, unsigned int Z)
{
   return Y ^ (X | ~Z);
}

static unsigned int Round(unsigned int a, unsigned int b, unsigned int FGHI,
   unsigned int k, unsigned int s, unsigned int i)
{
   return b + ROTATE_LEFT(a + FGHI + pX[k] + T[i], s);
}

static void Round1(unsigned int *a, unsigned int b, unsigned int c,
   unsigned int d,unsigned int k, unsigned int s, unsigned int i)
{
   *a = Round(*a, b, F(b,c,d), k, s, i);
}
static void Round2(unsigned int *a, unsigned int b, unsigned int c,
   unsigned int d,unsigned int k, unsigned int s, unsigned int i)
{
   *a = Round(*a, b, G(b,c,d), k, s, i);
}
static void Round3(unsigned int *a, unsigned int b, unsigned int c,
   unsigned int d,unsigned int k, unsigned int s, unsigned int i)
{
   *a = Round(*a, b, H(b,c,d), k, s, i);
}
static void Round4(unsigned int *a, unsigned int b, unsigned int c,
   unsigned int d,unsigned int k, unsigned int s, unsigned int i)
{
   *a = Round(*a, b, I(b,c,d), k, s, i);
}

/* preprocessing */
static void round_calculate(const unsigned char *block,
   unsigned int *A2, unsigned int *B2, unsigned int *C2, unsigned int *D2)
{
   unsigned int X[16];	/* 64 bytes */
   int j,k;

   /* set A as AA, B as BB, C as CC, and and D as DD (saving of A, B, C, and D) */
   unsigned int A=*A2, B=*B2, C=*C2, D=*D2;
   unsigned int AA = A,BB = B,CC = C,DD = D;

   /* copy into pX for Round 1-4 shortly afterwards */
   pX = X;

   /* copy message blocks into X */
   for (j=0,k=0; j<64; j+=4,k++)
      X[k] = ( (unsigned int )block[j] )
         | ( ((unsigned int )block[j+1]) << 8 )
         | ( ((unsigned int )block[j+2]) << 16 )
         | ( ((unsigned int )block[j+3]) << 24 );


   /* Round 1 */
   Round1(&A,B,C,D,  0, 7,  0); Round1(&D,A,B,C,  1, 12,  1); Round1(&C,D,A,B,  2, 17,  2); Round1(&B,C,D,A,  3, 22,  3);
   Round1(&A,B,C,D,  4, 7,  4); Round1(&D,A,B,C,  5, 12,  5); Round1(&C,D,A,B,  6, 17,  6); Round1(&B,C,D,A,  7, 22,  7);
   Round1(&A,B,C,D,  8, 7,  8); Round1(&D,A,B,C,  9, 12,  9); Round1(&C,D,A,B, 10, 17, 10); Round1(&B,C,D,A, 11, 22, 11);
   Round1(&A,B,C,D, 12, 7, 12); Round1(&D,A,B,C, 13, 12, 13); Round1(&C,D,A,B, 14, 17, 14); Round1(&B,C,D,A, 15, 22, 15);

   /* Round 2 */
   Round2(&A,B,C,D,  1, 5, 16); Round2(&D,A,B,C,  6, 9, 17); Round2(&C,D,A,B, 11, 14, 18); Round2(&B,C,D,A,  0, 20, 19);
   Round2(&A,B,C,D,  5, 5, 20); Round2(&D,A,B,C, 10, 9, 21); Round2(&C,D,A,B, 15, 14, 22); Round2(&B,C,D,A,  4, 20, 23);
   Round2(&A,B,C,D,  9, 5, 24); Round2(&D,A,B,C, 14, 9, 25); Round2(&C,D,A,B,  3, 14, 26); Round2(&B,C,D,A,  8, 20, 27);
   Round2(&A,B,C,D, 13, 5, 28); Round2(&D,A,B,C,  2, 9, 29); Round2(&C,D,A,B,  7, 14, 30); Round2(&B,C,D,A, 12, 20, 31);

   /* Round 3 */
   Round3(&A,B,C,D,  5, 4, 32); Round3(&D,A,B,C,  8, 11, 33); Round3(&C,D,A,B, 11, 16, 34); Round3(&B,C,D,A, 14, 23, 35);
   Round3(&A,B,C,D,  1, 4, 36); Round3(&D,A,B,C,  4, 11, 37); Round3(&C,D,A,B,  7, 16, 38); Round3(&B,C,D,A, 10, 23, 39);
   Round3(&A,B,C,D, 13, 4, 40); Round3(&D,A,B,C,  0, 11, 41); Round3(&C,D,A,B,  3, 16, 42); Round3(&B,C,D,A,  6, 23, 43);
   Round3(&A,B,C,D,  9, 4, 44); Round3(&D,A,B,C, 12, 11, 45); Round3(&C,D,A,B, 15, 16, 46); Round3(&B,C,D,A,  2, 23, 47);

   /* Round 4 */
   Round4(&A,B,C,D,  0, 6, 48); Round4(&D,A,B,C,  7, 10, 49); Round4(&C,D,A,B, 14, 15, 50); Round4(&B,C,D,A,  5, 21, 51);
   Round4(&A,B,C,D, 12, 6, 52); Round4(&D,A,B,C,  3, 10, 53); Round4(&C,D,A,B, 10, 15, 54); Round4(&B,C,D,A,  1, 21, 55);
   Round4(&A,B,C,D,  8, 6, 56); Round4(&D,A,B,C, 15, 10, 57); Round4(&C,D,A,B,  6, 15, 58); Round4(&B,C,D,A, 13, 21, 59);
   Round4(&A,B,C,D,  4, 6, 60); Round4(&D,A,B,C, 11, 10, 61); Round4(&C,D,A,B,  2, 15, 62); Round4(&B,C,D,A,  9, 21, 63);

   /* Perform additions */
   *A2 = A + AA;
   *B2 = B + BB;
   *C2 = C + CC;
   *D2 = D + DD;

   /* unset data in buffer */
   memset(pX, 0, sizeof(X));
}

/* convert string into binary sequence */
void stringtobinary(const char * string, char * output)
{
   unsigned char padding_message[64];	/* 64 bytes buffer */
   unsigned char *pstring;				/* position of substring in the entire message */

   unsigned int string_byte_len,	/* string byte length */
      string_bit_len,	/* bit length */
      copy_len,			/* number of bytes used for 1-3 */
      msg_digest[4];	/* 4 bytes message digest */
   unsigned int *A = &msg_digest[0],	/* message digest */
      *B = &msg_digest[1],
      *C = &msg_digest[2],
      *D = &msg_digest[3];
   int i;

   /* initialise variables */
   *A = 0x67452301;
   *B = 0xefcdab89;
   *C = 0x98badcfe;
   *D = 0x10325476;

   string_byte_len = strlen(string);	/* get length */
   pstring = (unsigned char *)string;	/* set position to start of string */

   /* process message in 64 byte chunks */
   for (i=string_byte_len; 64<=i; i-=64,pstring+=64)
        round_calculate(pstring, A,B,C,D);

   /* starting to append padding bits */
   copy_len = string_byte_len % 64;
   strncpy((char *)padding_message, (char *)pstring, copy_len);
   memset(padding_message+copy_len, 0, 64 - copy_len);
   padding_message[copy_len] |= 0x80;

   /* extend to 64 bytes if insufficient length */
   if (56 <= copy_len) {
       round_calculate(padding_message, A,B,C,D);
       memset(padding_message, 0, 56);
   }

   /* append length */
   string_bit_len = string_byte_len * 8;
   memcpy(&padding_message[56], &string_bit_len, 4);

  /* set to high endian if low endian is insufficient to store 32 bytes */
  if (UINT_MAX / 8 < string_byte_len) {
      unsigned int high = (string_byte_len - UINT_MAX / 8) * 8;
      memcpy(&padding_message[60], &high, 4);
  } else
      memset(&padding_message[60], 0, 4);

   /* process */
   round_calculate(padding_message, A,B,C,D);
   memcpy(output,msg_digest,16);
}

void calc_md5(const char * string, char * output)
{
   unsigned char digest[16];

   /* convert to binary sequence and calculate md5 */
   stringtobinary(string,(char*)digest);
   /* save md5 digest in output */
   sprintf(output,
      "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
      digest[ 0], digest[ 1], digest[ 2], digest[ 3],
      digest[ 4], digest[ 5], digest[ 6], digest[ 7],
      digest[ 8], digest[ 9], digest[10], digest[11],
      digest[12], digest[13], digest[14], digest[15]);
}

/* child function */
void child (double c)
{
	int i,j;
	char buf[LEN];	/* buffer for random string */
	char out[33];	/* 32-bit buffer for md5 calculations */

	sleep(1);	/* sleep a bit to allow parent to spawn all children */

	/* overflow protection */
	buf[LEN-1] = '\0';
	out[32] = '\0';

	/* start calculation loop */
	for (j=0; j<100; j++) {
		/* generate a 65k long string of random characters */
		for (i=0; i<LEN-1; i++)
			buf[i]=rand()%80 + '0';
		/* calculate md5 hash for the string */
		calc_md5(buf, out);
		/* sleep a bit */
		sleep(c);
	}
	/* return to main() */
	return;
}

/* main program */
int main (void)
{
	int i;
	/* create 4 child processes, then quit */
	for (i = 1; i <= 4; i++) {
		/* create child */
		if (fork() == 0) {
			/* fork succeeds, pass 'sleep time'
			* to the calculation function */
			child(1/i);
			/* quit when finished */
			exit(0);
		}
	}
	return 0;
}
