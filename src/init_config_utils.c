/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   init_config_utils.c                               :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: username <username@student.42tokyo.jp>    #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/05/23 17:55:26 by username         #+#    #+#              */
/*   Updated: 2026/05/23 17:55:26 by username        ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	init_watcher(t_config *config)
{
	config->stop_watcher = 0;
	if (pthread_cond_init(&config->cond_stop, NULL))
		return (0);
	if (pthread_mutex_init(&config->lock_stop, NULL))
	{
		pthread_cond_destroy(&config->cond_stop);
		return (0);
	}
	return (1);
}

int	init_print(t_config *config)
{
	if (pthread_mutex_init(&config->print, NULL))
	{
		des_cond(config, 1, 1, -1);
		des_mtx(config, (t_params){1, 0, 1, 1, -1});
		return (0);
	}
	return (1);
}

int	init_operation(t_config *config)
{
	if (pthread_cond_init(&config->operation.op_cond, NULL))
	{
		des_cond(config, 1, 0, -1);
		des_mtx(config, (t_params){1, 0, 1, 0, -1});
		return (0);
	}
	if (pthread_mutex_init(&config->operation.op_lock, NULL))
	{
		des_cond(config, 1, 1, -1);
		des_mtx(config, (t_params){1, 0, 1, 0, -1});
		return (0);
	}
	return (1);
}

int	init_shuled(t_dongle *dongle, t_config *config, int i)
{
	config->dongles[i].id = i;
	config->dongles[i].available_at = 0;
	config->dongles[i].is_taken = 0;
	if (pthread_mutex_init(&dongle->locker_d, NULL))
	{
		des_cond(config, 1, 1, -1);
		des_mtx(config, (t_params){1, 1, 1, 1, i});
		return (0);
	}
	dongle->scheduler.heap = malloc(sizeof(t_req) * config->number_coders);
	if (!dongle->scheduler.heap)
	{
		des_cond(config, 1, 1, -1);
		des_mtx(config, (t_params){1, 1, 1, 1, i + 1});
		return (0);
	}
	dongle->scheduler.size = 0;
	dongle->scheduler.mode = config->scheduler_mode;
	return (1);
}

void	assing_coder(t_coder *coder, t_dongle *dongles, int tot, int i)
{
	if ((i + 1) == 1)
	{
		coder->f = &dongles[0];
		coder->l = &dongles[tot - 1];
	}
	else
	{
		if ((i + 1) % 2 == 1)
		{
			coder->f = &dongles[i];
			coder->l = &dongles[i - 1];
		}
		else
		{
			coder->f = &dongles[i - 1];
			coder->l = &dongles[i];
		}
	}
}
