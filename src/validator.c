/* ************************************************************************** */
/*                                                                            */
/*                                                       :::      ::::::::    */
/*   validator.c                                       :+:      :+:    :+:    */
/*                                                   +:+ +:+         +:+      */
/*   By: username <username@student.42tokyo.jp>    #+#  +:+       +#+         */
/*                                               +#+#+#+#+#+   +#+            */
/*   Created: 2026/05/23 17:56:03 by username         #+#    #+#              */
/*   Updated: 2026/05/23 17:56:09 by username        ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long	ft_atoi(char *nbr)
{
	long	res;
	int		i;
	int		sign;

	res = 0;
	i = 0;
	sign = 1;
	if (nbr[i] == '+' || nbr[i] == '-')
	{
		if (nbr[i] == '-')
			sign = -1;
		++i;
	}
	while (nbr[i])
	{
		if (!(nbr[i] <= 57 && nbr[i] >= 48))
			return (-1);
		res = res * 10 + nbr[i] - 48;
		if (res > INT_MAX)
			return (-1);
		++i;
	}
	return (res * sign);
}

static int	is_shuld(char *pos)
{
	if (!pos)
		return (0);
	if (!strcmp(pos, "fifo"))
		return (1);
	if (!strcmp(pos, "edf"))
		return (1);
	return (0);
}

int	is_valid(char **arv, int arc)
{
	int	i;

	i = 0;
	if (arc != 8)
		return (0);
	while (i < 6)
	{
		if (ft_atoi(arv[i]) < 0)
			return (0);
		++i;
	}
	if (ft_atoi(arv[0]) <= 0 || ft_atoi(arv[5]) <= 0)
		return (0);
	return (is_shuld(arv[7]));
}
