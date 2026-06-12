/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   routine_utils1.c                                  :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: username <username@student.42tokyo.jp>    #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/05/23 17:53:52 by username         #+#    #+#              */
/*   Updated: 2026/05/23 17:54:47 by username        ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	is_valid_cond(t_dongle *first, t_dongle *last, t_coder *coder)
{
	return (
		!is_dongle_avaible(first)
		|| !is_dongle_avaible(last)
		|| get_smaller(first) != coder->id
		|| get_smaller(last) != coder->id);
}

int	wait_coders(t_config *config)
{
	pthread_mutex_lock(&config->operation.op_lock);
	while (!config->operation.is_finish)
		pthread_cond_wait(&config->operation.op_cond,
			&config->operation.op_lock);
	if (config->operation.op_coders == config->number_coders + 1)
	{
		pthread_mutex_unlock(&config->operation.op_lock);
		return (1);
	}
	pthread_mutex_unlock(&config->operation.op_lock);
	return (0);
}

void	req_dongle(t_dongle *dongle, t_coder *coder)
{
	pthread_mutex_lock(&dongle->locker_d);
	if (!dup_coder(dongle, coder->id))
		insert(dongle, coder);
	pthread_mutex_unlock(&dongle->locker_d);
}

void	take_dongle(t_dongle *dongle, t_coder *coder)
{
	pthread_mutex_lock(&dongle->locker_d);
	log_hanlder(coder->config, coder->id, "has taken a dongle");
	dongle->is_taken = 1;
	delete_smaller(dongle);
	pthread_mutex_unlock(&dongle->locker_d);
}

void	take_dongles(t_coder *coder)
{
	pthread_mutex_lock(&coder->wake_mtx);
	while (is_valid_cond(coder->f, coder->l, coder))
	{
		if (should_stop(coder->config))
		{
			pthread_mutex_unlock(&coder->wake_mtx);
			return ;
		}
		pthread_cond_wait(&coder->wake_cond, &coder->wake_mtx);
	}
	pthread_mutex_unlock(&coder->wake_mtx);
	if (should_stop(coder->config))
		return ;
	take_dongle(coder->f, coder);
	take_dongle(coder->l, coder);
}
