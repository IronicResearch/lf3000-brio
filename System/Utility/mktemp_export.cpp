#ifdef LF1000
#include <stdlib.h>

extern "C"
char * __libc_mktemp(char* temp)
{
	return mktemp(temp);
}
#endif
