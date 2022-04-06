#include <stdio.h>

// See AtomicFile.cpp for usage
int fabortAtomic (FILE *fp);
FILE *fopenAtomic(const char *path, const char *mode);
int fcloseAtomic(FILE *fp);
int fabortAllAtomic ();

