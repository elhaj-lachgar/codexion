/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   routine_utils2.c                                  :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: username <username@student.42tokyo.jp>    #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/05/23 17:52:25 by username         #+#    #+#              */
/*   Updated: 2026/05/23 17:55:40 by username        ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	assign_req(t_coder *coder)
{
	coder->req_time = get_time_ms();
	if (coder->f < coder->l)
	{
		req_dongle(coder->f, coder);
		req_dongle(coder->l, coder);
	}
	else
	{
		req_dongle(coder->l, coder);
		req_dongle(coder->f, coder);
	}
}

void	one_coder(t_coder *coder)
{
	int	index;

	index = 1;
	take_dongle(coder->f, coder, &index);
}

void	release_dongle(t_dongle *dongle, t_config *config)
{
	int	i;

	pthread_mutex_lock(&dongle->locker_d);
	dongle->is_taken = 0;
	dongle->available_at = get_time_ms() + config->dongle_cooldown;
	pthread_mutex_unlock(&dongle->locker_d);
	i = 0;
	while (i < config->number_coders)
	{
		pthread_mutex_lock(&config->coders[i].wake_mtx);
		pthread_cond_broadcast(&config->coders[i].wake_cond);
		pthread_mutex_unlock(&config->coders[i].wake_mtx);
		i++;
	}
}

void	assign_order(t_dongle **first, t_dongle **last, t_coder *coder)
{
	*first = coder->f;
	*last = coder->l;
	if (coder->l < coder->f)
	{
		*last = coder->f;
		*first = coder->l;
	}
}
