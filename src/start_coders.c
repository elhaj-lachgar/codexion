/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   start_coders.c                                    :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: username <username@student.42tokyo.jp>    #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/05/23 17:55:51 by username         #+#    #+#              */
/*   Updated: 2026/05/23 17:55:51 by username        ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	join_coders(t_config *config, int i)
{
	int	coders_created;

	coders_created = config->number_coders;
	if (i < config->number_coders)
		coders_created = i;
	while (--coders_created >= 0)
		pthread_join(config->coders[coders_created].thread, NULL);
	if (i > config->number_coders)
		pthread_join(config->watcher, NULL);
}

static void	set_time(t_config *config)
{
	int	i;

	i = 0;
	while (i < config->number_coders)
	{
		pthread_mutex_lock(&config->coders[i].c_lock);
		config->coders[i].last_compile = config->start;
		pthread_mutex_unlock(&config->coders[i].c_lock);
		++i;
	}
}

static int	create_coders(t_config *config)
{
	int	i;

	i = 0;
	while (i < config->number_coders)
	{
		if (pthread_create(&config->coders[i].thread, NULL,
				routine, (void *)(&config->coders[i])))
			return (i);
		++i;
	}
	if (pthread_create(&config->watcher, NULL, watcher, (void *) config))
		return (i);
	return (++i);
}

static int	wait(t_config *config)
{
	int	i;

	i = create_coders(config);
	pthread_mutex_lock(&config->operation.op_lock);
	config->operation.is_finish = 1;
	config->operation.op_coders = i;
	pthread_mutex_lock(&config->time_lock);
	config->start = get_time_ms();
	pthread_mutex_unlock(&config->time_lock);
	if (config->number_coders == i - 1)
		set_time(config);
	pthread_cond_broadcast(&config->operation.op_cond);
	pthread_mutex_unlock(&config->operation.op_lock);
	return (i);
}

int	start_coders(t_config *config)
{
	int	coders_created;

	coders_created = wait(config);
	join_coders(config, coders_created);
	if (coders_created != config->number_coders + 1)
	{
		fprintf(stderr, "fail to create all coders\n");
		free_handler(config, 1);
		free(config);
		return (0);
	}
	return (coders_created == config->number_coders + 1);
}
