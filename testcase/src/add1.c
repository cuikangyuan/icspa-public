#include "trap.h"
int main()
{
	BREAK_POINT;
	HIT_GOOD_TRAP;
	return 0;
}
//./scripts/objdump4nemu-i386 -d ./testcase/src/add1 > ./testcase/src/add1.dd
