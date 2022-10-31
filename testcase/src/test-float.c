#include "trap.h"

int main()
{

	float a = 1.2, b = 1;
	float c = a + b;
	if (c == 2.2)
		;
	else
		HIT_BAD_TRAP;

	c = a * b;
	if (c == 1.2)
		;
	else
		HIT_BAD_TRAP;

	c = a / b;
	if (c == 1.2)
		;
	else
		HIT_BAD_TRAP;

	/*
	    30051:	80 fc 40             	cmp    $0x40,%ah
   		30054:	74 06                	je     3005c <main+0x57>
   		30056:	b8 01 00 00 00       	mov    $0x1,%eax
   		3005b:	82                   	nemu_trap 
  		3005c:	b8 00 00 00 00       	mov    $0x0,%eax
	*/
	c = a - b;
	if (c == 0.2) // this will fail, and also fails for native program, interesting, can be used as a quiz
		;
	else
		HIT_BAD_TRAP;

	HIT_GOOD_TRAP;
	return 0;
}
