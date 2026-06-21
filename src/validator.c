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

static int	is_error(int *error)
{
	*error = 1;
	return (-1);
}

static void	error_logs(int *err_digit, int *err_overflow)
{
	if (*err_digit)
		printf("The Arguments must be integer\n");
	else if (*err_overflow)
		printf("The Arguments overflow\n");
	else
		printf("The Argument is Negative Value\n");
}

long	ft_atoi(char *nbr, int *not_digit, int *overflow)
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
			return (is_error(not_digit));
		res = res * 10 + nbr[i] - 48;
		if ((sign == 1 && res > INT_MAX)
			|| (sign == -1 && res - 1 > INT_MAX))
			return (is_error(overflow));
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
	printf("scheduler Must equal fifo or edf\n");
	return (0);
}

int	is_valid(char **arv, int arc)
{
	int	i;
	int	not_digit;
	int	overflow;

	i = 0;
	if (!add_validator(arc))
		return (0);
	not_digit = 0;
	overflow = 0;
	while (i < 7)
	{
		if (ft_atoi(arv[i], &not_digit, &overflow) < 0)
		{
			error_logs(&not_digit, &overflow);
			return (0);
		}
		if (!add_validator1(i, ft_atoi(arv[i], &not_digit, &overflow)))
			return (0);
		++i;
	}
	return (is_shuld(arv[7]));
}
