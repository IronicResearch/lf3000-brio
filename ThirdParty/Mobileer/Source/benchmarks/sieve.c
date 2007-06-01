/*
 * sieve.c
 *
 * Description:
 *
 *   The Sieve of Eratosthenes benchmark, from Byte Magazine
 *   early 1980s, when a PC would do well to run this in 10 
 *   seconds. This version really does count prime numbers
 *   but omits the numbers 1, 3 and all even numbers. The
 *   expected count is 1899.
 *
 * ChangeLog:
 *
 *   $Log: sieve.c,v $
 *   Revision 1.2  2005/05/03 22:04:00  philjmsl
 *   fixed tabs and indentation using astyle
 *
 *   Revision 1.1  2005/03/29 03:43:19  philjmsl
 *   move sieve.c to benchmarks
 *
 *   Revision 1.2  2005/03/29 03:42:35  philjmsl
 *   add more mains
 *
 *   Revision 1.1  2005/03/28 03:52:33  philjmsl
 *   for benchmarking
 *
 *   Revision 1.1  1998/06/25 13:46:42  nettleto
 *   Initial revision
 *
 */
#include <stdio.h>
#if 0
#include <time.h>
#include <report.h>
#endif

#define SIZE 8190

int
sieve ()
{
	unsigned char flags [SIZE + 1];
	int iter;
	int count;

	for (iter = 1; iter <= 10; iter++)
	{
		int i, prime, k;
		count = 0;

		for (i = 0; i <= SIZE; i++)
			flags [i] = 1;

		for (i = 0; i <= SIZE; i++)
		{
			if (flags [i])
			{
				prime = i + i + 3;
				k = i + prime;
				while (k <= SIZE)
				{
					flags [k] = 0;
					k += prime;
				}
				count++;
			}
		}
	}

	return count;
}


#if 0
int
main ()
{
	int ans;

	printf ("Sieve benchmark\n");

	ans = sieve ();

	if (ans != 1899)
		printf ("Sieve result wrong, ans = %d, expected 1899", ans);

	printf ("Done\n");
	return 0;
}
#endif

#if 0
int
main ()
{
	int ans;
	clock_t t1, t2;

	test ("sieve", "Sieve benchmark,  $Revision: 1.2 $");

	t1 = clock ();
	ans = sieve ();
	t2 = clock ();

	if (ans != 1899)
		failed ("Sieve result wrong, ans = %d, expected 1899", ans);

	comment ("Time taken = %.3e mSecs",
	         1000.0 * ((double)t2 - (double)t1) / (double)CLOCKS_PER_SEC);

	result ();
}
#endif
