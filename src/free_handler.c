/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free_handler.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elachgar <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/08 16:37:14 by elachgar          #+#    #+#             */
/*   Updated: 2026/05/08 16:37:18 by elachgar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "codexion.h"

void	free_handler(t_config *config, int is_finish)
{
	int	tot;

	if (is_finish)
	{
		tot = config->number_coders;
		des_coder_mtx(config, tot, tot, tot);
		des_cond(config, 1, 1, tot);
		des_mtx(config, (t_params){1, 1, 1, 1, tot});
	}
	if (config->coders)
	{
		free(config->coders);
		config->coders = NULL;
	}
	if (config->dongles)
	{
		free(config->dongles);
		config->dongles = NULL;
	}
}

void	des_coder_mtx(t_config *config, int c_l, int c_c, int w_c)
{
	while (--c_l >= 0)
		pthread_mutex_destroy(&config->coders[c_l].c_lock);
	while (--c_c >= 0)
		pthread_mutex_destroy(&config->coders[c_c].c_lock);
	while (--w_c >= 0)
		pthread_mutex_destroy(&config->coders[w_c].wake_mtx);
}

void	des_mtx(t_config *config, t_params params)
{
	if (config->dongles)
	{
		while (--params.d >= 0)
		{
			pthread_mutex_destroy(&config->dongles[params.d].locker_d);
			if (config->dongles[params.d].scheduler.heap)
				free(config->dongles[params.d].scheduler.heap);
		}
	}
	if (params.p)
		pthread_mutex_destroy(&config->print);
	if (params.s)
		pthread_mutex_destroy(&config->lock_stop);
	if (params.op)
		pthread_mutex_destroy(&config->operation.op_lock);
	if (params.t)
		pthread_mutex_destroy(&config->time_lock);
}

void	des_cond(t_config *config, int s, int op, int d)
{
	if (s)
		pthread_cond_destroy(&config->cond_stop);
	if (op)
		pthread_cond_destroy(&config->operation.op_cond);
	if (config->dongles)
	{
		while (--d >= 0)
			pthread_cond_destroy(&config->coders[d].wake_cond);
	}
}
