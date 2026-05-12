#include "codexion.h"

static int	get_sheduler(char *string)
{
	char	*shulder_lower;
	int		mode;

	shulder_lower = lower(string);
	if (!shulder_lower)
		return (-1);
	mode = 0;
	if (!strcmp("edf", shulder_lower))
		mode = 1;
	free(shulder_lower);
	return (mode);
}

t_config	*set_config(char **arv)
{
	t_config	*config;
	int			mode;

	config = (t_config *)malloc(sizeof(t_config));
	if (!config)
		return (NULL);
	memset(config, 0, sizeof(t_config));
	config->number_coders = ft_atoi(arv[0]);
	config->time_to_burnout = ft_atoi(arv[1]);
	config->time_to_compile = ft_atoi(arv[2]);
	config->time_to_debug = ft_atoi(arv[3]);
	config->time_to_refactor = ft_atoi(arv[4]);
	config->required_compiles = ft_atoi(arv[5]);
	config->dongle_cooldown = ft_atoi(arv[6]);
	mode = get_sheduler(arv[7]);
	if (mode < 0)
		return (free(config), NULL);
	config->scheduler_mode = mode;
	return (config);
}
