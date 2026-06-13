/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   set_config.c                                      :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: username <username@student.42tokyo.jp>    #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/05/23 17:55:46 by username         #+#    #+#              */
/*   Updated: 2026/05/23 17:55:46 by username        ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	get_sheduler(char *string)
{
	int		mode;

	if (!string)
		return (-1);
	mode = 0;
	if (!strcmp("edf", string))
		mode = 1;
	return (mode);
}

t_config	*set_config(char **arv)
{
	t_config	*config;
	int			mode;

	config = (t_config *) malloc(sizeof(t_config));
	if (!config)
		return (NULL);
	memset(config, 0, sizeof(t_config));
	config->number_coders = ft_atoi(arv[0], NULL, NULL);
	config->time_to_burnout = ft_atoi(arv[1], NULL, NULL);
	config->time_to_compile = ft_atoi(arv[2], NULL, NULL);
	config->time_to_debug = ft_atoi(arv[3], NULL, NULL);
	config->time_to_refactor = ft_atoi(arv[4], NULL, NULL);
	config->required_compiles = ft_atoi(arv[5], NULL, NULL);
	config->dongle_cooldown = ft_atoi(arv[6], NULL, NULL);
	mode = get_sheduler(arv[7]);
	if (mode < 0)
	{
		free(config);
		return (NULL);
	}
	config->scheduler_mode = mode;
	return (config);
}
