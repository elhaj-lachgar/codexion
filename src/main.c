/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   main.c                                            :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: username <username@student.42tokyo.jp>    #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/05/23 17:55:36 by username         #+#    #+#              */
/*   Updated: 2026/05/23 17:55:36 by username        ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	main(int arc, char **arv)
{
	t_config	*config;

	config = NULL;
	if (!is_valid(arv + 1, arc - 1))
		return (printf("ERROR\n"), 1);
	config = set_config(arv + 1);
	if (!config)
		return (printf("ERROR\n"), 1);
	if (!init_config(config))
		return (free(config), printf("ERROR\n"), 1);
	if (!start_coders(config))
		return (free(config), printf("ERROR\n"), 1);
	free_handler(config, 1);
	free(config);
	return (0);
}
