/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   routine.c                                         :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: username <username@student.42tokyo.jp>    #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/05/23 17:52:40 by username         #+#    #+#              */
/*   Updated: 2026/05/23 17:53:48 by username        ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	compile(t_coder *coder)
{
	if (should_stop(coder->config))
		return ;
	pthread_mutex_lock(&coder->lock);
	coder->last_compile = get_time_ms();
	pthread_mutex_unlock(&coder->lock);
	if (!log_hanlder(coder->config, coder->id, "is compiling"))
		return ;
	usleep(coder->config->time_to_compile * 1000);
	pthread_mutex_lock(&coder->c_lock);
	coder->compiles++;
	pthread_mutex_unlock(&coder->c_lock);
}

static void	debug(t_coder *coder)
{
	if (should_stop(coder->config))
		return ;
	if (!log_hanlder(coder->config, coder->id, "is debuging"))
		return ;
	usleep(coder->config->time_to_debug * 1000);
}

static void	refactor(t_coder *coder)
{
	if (should_stop(coder->config))
		return ;
	if (!log_hanlder(coder->config, coder->id, "is refactoring"))
		return ;
	usleep(coder->config->time_to_refactor * 1000);
}

static int	actions(t_config *config, t_coder *coder)
{
	assign_req(coder);
	take_dongles(coder);
	if (should_stop(config))
		return (0);
	compile(coder);
	release_dongle(coder->f, config);
	release_dongle(coder->l, config);
	debug(coder);
	refactor(coder);
	return (1);
}

void	*routine(void *arg)
{
	t_coder		*coder;
	t_config	*config;

	coder = (t_coder *) arg;
	config = coder->config;
	if (!wait_coders(config) || !config->time_to_burnout)
		return (NULL);
	if (config->number_coders == 1)
	{
		take_dongle(coder->f, coder);
		return (NULL);
	}
	if (coder->id % 2 == 0)
		usleep(config->time_to_compile * 1000 / 2);
	while (1)
	{
		if (get_compile(coder) == config->required_compiles
			|| should_stop(config))
			break ;
		if (!actions(config, coder))
			break ;
	}
	return (NULL);
}
