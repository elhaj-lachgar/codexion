#include "codexion.h"

static void	wake_up_all(t_config *config)
{
	int	i;

	i = 0;
	while (i < config->number_coders)
	{
		pthread_mutex_lock(&config->coders[i].wake_mtx);
		pthread_cond_broadcast(&config->coders[i].wake_cond);
		pthread_mutex_unlock(&config->coders[i].wake_mtx);
		++i;
	}
}

static void	bournout_null(t_config *config)
{
	log_hanlder(config, config->coders[0].id, "burned out");
	pthread_mutex_lock(&config->lock_stop);
	config->stop_watcher = 1;
	pthread_cond_broadcast(&config->cond_stop);
	pthread_mutex_unlock(&config->lock_stop);
	wake_up_all(config);
}

int	is_finish(t_config *config)
{
	int	i;

	i = 0;
	while (i < config->number_coders)
	{
		if (get_compile(config->coders + i) != config->required_compiles)
			return (0);
		++i;
	}
	pthread_mutex_lock(&config->lock_stop);
	config->stop_watcher = 1;
	pthread_cond_broadcast(&config->cond_stop);
	pthread_mutex_unlock(&config->lock_stop);
	return (1);
}

static int	is_bournout(t_config *config)
{
	long	last;
	long	elapsed;
	long	now;
	int		i;

	i = 0;
	now = get_time_ms();
	while (i < config->number_coders)
	{
		last = get_last_compile(&config->coders[i]);
		elapsed = now - last;
		if (elapsed > config->time_to_burnout
			&& get_compile(config->coders + i) != config->required_compiles)
		{
			pthread_mutex_lock(&config->lock_stop);
			config->stop_watcher = 1;
			pthread_mutex_unlock(&config->lock_stop);
			log_hanlder(config, config->coders[i].id, "burned out");
			wake_up_all(config);
			return (1);
		}
		++i;
	}
	return (0);
}

void	*watcher(void *args)
{
	t_config	*config;

	config = (t_config *)args;
	if (!wait_coders(config))
		return (NULL);
	if (!config->time_to_burnout)
	{
		bournout_null(config);
		return (NULL);
	}
	while (1)
	{
		if (is_finish(config))
			return (NULL);
		if (should_stop(config))
			return (NULL);
		if (is_bournout(config))
			return (NULL);
		wake_up_all(config);
		usleep(500);
	}
	return (NULL);
}
