/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   time.c                                            :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: username <username@student.42tokyo.jp>    #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/05/23 17:55:54 by username         #+#    #+#              */
/*   Updated: 2026/05/23 17:55:54 by username        ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long	get_time_ms(void)
{
	struct timeval	t;

	if (gettimeofday(&t, NULL) == -1)
		return (-1);
	return (t.tv_sec * 1000 + t.tv_usec / 1000);
}
