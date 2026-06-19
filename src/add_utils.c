/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   add_utils.c                                       :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: username <username@student.42tokyo.jp>    #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/05/23 17:55:00 by username         #+#    #+#              */
/*   Updated: 2026/05/23 17:55:00 by username        ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

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

void	my_sleep(long time, t_config *config)
{
	long	curr;

	if (time < 10)
		time += 10;
	curr = get_time_ms();
	while (get_time_ms() - curr < time)
	{
		if (should_stop(config))
			break ;
		usleep(500);
	}
}
