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

void	my_sleep(long time, t_config *config, int add)
{
	long	curr;

	if (add && time < 10)
		time = 10;
	curr = get_time_ms();
	while (get_time_ms() - curr < time)
	{
		if (should_stop(config))
			break ;
		usleep(500);
	}
}

int	add_validator(int arc)
{
	if (arc != 8)
	{
		fprintf(stderr, "ERROR: lenght of arguments must be 8\n");
		return (0);
	}
	return (1);
}

int	add_validator1(int index, int value)
{
	if (index != 0 && index != 5)
		return (1);
	if (value == 0)
	{
		if (index == 0)
			fprintf(stderr, "number of coders must be bigger 0\n");
		if (index == 5)
			fprintf(stderr, "compile required must be bigger 0\n");
		return (0);
	}
	return (1);
}
