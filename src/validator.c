#include "codexion.h"

long	ft_atoi(char *nbr)
{
	long	res;
	int		i;
	int	sign;
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

char	*lower(char *shuld)
{
	char	*lower_shuld;
	int		i;

	if (!shuld)
		return (NULL);
	lower_shuld = malloc(sizeof(char) * (strlen(shuld) + 1));
	if (!lower_shuld)
		return (NULL);
	i = 0;
	while (shuld[i])
	{
		if (shuld[i] >= 65 && shuld[i] <= 90)
			lower_shuld[i] = shuld[i] + 32;
		else
			lower_shuld[i] = shuld[i];
		++i;
	}
	lower_shuld[i] = 0;
	return (lower_shuld);
}

static int	is_shuld(char *pos)
{
	char	*lower_shuld;

	lower_shuld = lower(pos);
	if (!lower_shuld)
		return (0);
	if (!strcmp(lower_shuld, "fifo"))
		return (free(lower_shuld), 1);
	if (!strcmp(lower_shuld, "edf"))
		return (free(lower_shuld), 1);
	return (free(lower_shuld), 0);
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
