#include "codexion.h"

static int	helper_coders(t_coder *coder, int i)
{
	if (pthread_mutex_init(&coder->lock, NULL))
	{
		des_coder_mtx(coder->config, i - 1, i - 1, i - 1);
		des_cond(coder->config, 1, 1, i - 1);
		des_mtx(coder->config, 1, 1, 1, coder->config->number_coders);
		return (0);
	}
	if (pthread_mutex_init(&coder->c_lock, NULL))
	{
		des_coder_mtx(coder->config, i, i - 1, i - 1);
		des_cond(coder->config, 1, 1, i - 1);
		des_mtx(coder->config, 1, 1, 1, coder->config->number_coders);
		return (0);
	}
	if (pthread_mutex_init(&coder->wake_mtx, NULL))
	{
		des_coder_mtx(coder->config, i, i, i - 1);
		des_cond(coder->config, 1, 1, i - 1);
		des_mtx(coder->config, 1, 1, 1, coder->config->number_coders);
		return (0);
	}
	if (pthread_cond_init(&coder->wake_cond, NULL))
	{
		des_coder_mtx(coder->config, i, i, i );
		des_cond(coder->config, 1, 1, i - 1);
		des_mtx(coder->config, 1, 1, 1, coder->config->number_coders);
		return (0);
	}
	return (1);
}

static int	init_watcher(t_config *config)
{
	config->stop_watcher = 0;
	if (pthread_cond_init(&config->cond_stop, NULL))
		return (0);
	if (pthread_mutex_init(&config->lock_stop, NULL))
	{
		pthread_cond_destroy(&config->cond_stop);
		return (0);
	}
	return (1);
}

static int	init_print(t_config *config)
{
	if (pthread_mutex_init(&config->print, NULL))
	{
		des_cond(config, 1, 1, -1);
		des_mtx(config, 0, 1, 1, -1);
		return (0);
	}
	return (1);
}

static int	init_operation(t_config *config)
{
	if (pthread_cond_init(&config->operation.op_cond, NULL))
	{
		des_cond(config, 1, 0, -1);
		des_mtx(config, 0, 1, 0, -1);
		return (0);
	}
	if (pthread_mutex_init(&config->operation.op_lock, NULL))
	{
		des_cond(config, 1, 1, -1);
		des_mtx(config, 0, 1, 0, -1);
		return (0);
	}
	return (1);
}

static int	init_shuled(t_dongle *dongle, t_config *config, int i)
{
	if (pthread_mutex_init(&dongle->locker_d, NULL))
	{
		des_cond(config, 1, 1, -1);
		des_mtx(config, 1, 1, 1, i);
		return (0);
	}
	dongle->scheduler.heap = malloc(sizeof(t_req) * config->number_coders);
	if (!dongle->scheduler.heap)
	{
		des_cond(config, 1, 1, -1);
		des_mtx(config, 1, 1, 1, i + 1);
		return (0);
	}
	dongle->scheduler.size = 0;
	dongle->scheduler.mode = config->scheduler_mode;
	return (1);
}

static void	assing_coder(t_coder *coder, t_dongle *dongles, int tot, int i)
{
	coder->f = &dongles[i];
	coder->l = &dongles[(i + tot - 1) % tot];
	if ((i + 1) % 2 == 0)
	{
		coder->f = &dongles[(i + tot - 1) % tot];
		coder->l = &dongles[i];
	}
}

static int	init_coders(t_config *config)
{
	int	i;

	i = 0;
	config->coders = malloc(sizeof(t_coder) * config->number_coders);
	if (!config->coders)
	{
		des_cond(config, 1, 1, -1);
		des_mtx(config, 1, 1, 1, config->number_coders);
		return (0);
	}
	while (i < config->number_coders)
	{
		config->coders[i].id = i + 1;
		config->coders[i].config = config;
		config->coders[i].last_compile = config->start;
		config->coders[i].req_time = 0;
		config->coders[i].compiles = 0;
		config->coders[i].is_waiting = 0;
		if (!helper_coders(config->coders + i, i))
			return (0);
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
	pthread_mutex_init(&config->time_lock, NULL);
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
		des_mtx(config, 1, 1, 1, -1);
		return (0);
	}
	while (i < config->number_coders)
	{
		config->dongles[i].id = i;
		config->dongles[i].available_at = 0;
		config->dongles[i].is_taken = 0;
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
	config->start = get_time_ms();
	if (!init_dongles(config))
		return (free_handler(config, 0), 0);
	if (!init_coders(config))
		return (free_handler(config, 0), 0);
	return (1);
}
