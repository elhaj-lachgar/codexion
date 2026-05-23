#include "codexion.h"

int	is_dongle_avaible(t_dongle *dongle)
{
	int	is_av;

	pthread_mutex_lock(&dongle->locker_d);
	is_av = (dongle->available_at < get_time_ms()
			&& !dongle->is_taken);
	pthread_mutex_unlock(&dongle->locker_d);
	return (is_av);
}

int	get_is_waiting(t_coder *coder)
{
	int	waiting;

	pthread_mutex_lock(&coder->lock);
	waiting = coder->is_waiting;
	pthread_mutex_unlock(&coder->lock);
	return (waiting);
}
