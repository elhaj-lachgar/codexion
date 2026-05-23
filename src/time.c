#include "codexion.h"

long	get_time_ms(void)
{
	struct timeval	t;

	if (gettimeofday(&t, NULL) == -1)
		return (-1);
	return (t.tv_sec * 1000 + t.tv_usec / 1000);
}
