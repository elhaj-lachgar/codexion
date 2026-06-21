/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   init_config.c                                     :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: username <username@student.42tokyo.jp>    #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/05/23 17:55:32 by username         #+#    #+#              */
/*   Updated: 2026/05/23 17:55:32 by username        ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	set_coder_info(t_config *config, int i)
{
	config->coders[i].id = i + 1;
	config->coders[i].config = config;
	config->coders[i].last_compile = config->start;
	config->coders[i].req_time = 0;
	config->coders[i].compiles = 0;
}

static int	helper_coders(t_coder *coder, int i)
{
	if (pthread_mutex_init(&coder->lock, NULL))
	{
		des_coder_mtx(coder->config, i - 1, i - 1, i - 1);
		des_cond(coder->config, 1, 1, i - 1);
		return (0);
	}
	if (pthread_mutex_init(&coder->c_lock, NULL))
	{
		des_coder_mtx(coder->config, i, i - 1, i - 1);
		des_cond(coder->config, 1, 1, i - 1);
		return (0);
	}
	if (pthread_mutex_init(&coder->wake_mtx, NULL))
	{
		des_coder_mtx(coder->config, i, i, i - 1);
		des_cond(coder->config, 1, 1, i - 1);
		return (0);
	}
	if (pthread_cond_init(&coder->wake_cond, NULL))
	{
		des_coder_mtx(coder->config, i, i, i);
		des_cond(coder->config, 1, 1, i - 1);
		return (0);
	}
	return (1);
}

static int	init_coders(t_config *config)
{
	int	i;

	i = 0;
	config->coders = malloc(sizeof(t_coder) * config->number_coders);
	if (!config->coders)
	{
		des_cond(config, 1, 1, -1);
		des_mtx(config, (t_params){1, 1, 1, 1, config->number_coders});
		return (0);
	}
	while (i < config->number_coders)
	{
		set_coder_info(config, i);
		if (!helper_coders(config->coders + i, i))
		{
			des_mtx(config, (t_params){1, 1, 1, 1, config->number_coders});
			return (0);
		}
		assing_coder(&config->coders[i], config->dongles,
			config->number_coders, i);
		++i;
	}
	return (1);
}

static int	init_dongles(t_config *config)
{
	int	i;

	i = 0;
	if (pthread_mutex_init(&config->time_lock, NULL))
		return (0);
	if (!init_watcher(config))
		return (0);
	if (!init_operation(config))
		return (0);
	if (!init_print(config))
		return (0);
	config->dongles = malloc(sizeof(t_dongle) * config->number_coders);
	if (!config->dongles)
	{
		des_cond(config, 1, 1, -1);
		des_mtx(config, (t_params){1, 1, 1, 1, -1});
		return (0);
	}
	while (i < config->number_coders)
	{
		if (!init_shuled(&config->dongles[i], config, i))
			return (0);
		++i;
	}
	return (1);
}

int	init_config(t_config *config)
{
	if (!config)
		return (0);
	if (!init_dongles(config))
	{
		printf("fail to init dongles\n");
		free_handler(config, 0);
		free(config);
		return (0);
	}
	if (!init_coders(config))
	{
		printf("fail to init coders\n");
		free_handler(config, 0);
		free(config);
		return (0);
	}
	return (1);
}
