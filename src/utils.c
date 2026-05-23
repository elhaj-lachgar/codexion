/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   utils.c                                           :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: username <username@student.42tokyo.jp>    #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/05/23 17:56:00 by username         #+#    #+#              */
/*   Updated: 2026/05/23 17:56:00 by username        ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long	start_time(t_config *config)
{
	long	start;

	pthread_mutex_lock(&config->time_lock);
	start = config->start;
	pthread_mutex_unlock(&config->time_lock);
	return (start);
}

int	log_hanlder(t_config *config, int id, char *msg)
{
	pthread_mutex_lock(&config->print);
	if (!should_stop(config) || !strcmp(msg, "burned out"))
		printf("%ld %d %s\n", get_time_ms() - start_time(config), id, msg);
	pthread_mutex_unlock(&config->print);
	return (1);
}

int	should_stop(t_config *config)
{
	int	stop;

	pthread_mutex_lock(&config->lock_stop);
	stop = config->stop_watcher;
	pthread_mutex_unlock(&config->lock_stop);
	return (stop);
}

long	get_last_compile(t_coder *coder)
{
	long	last;

	pthread_mutex_lock(&coder->lock);
	last = coder->last_compile;
	pthread_mutex_unlock(&coder->lock);
	return (last);
}

int	get_compile(t_coder *coder)
{
	int	compiles;

	pthread_mutex_lock(&coder->c_lock);
	compiles = coder->compiles;
	pthread_mutex_unlock(&coder->c_lock);
	return (compiles);
}
