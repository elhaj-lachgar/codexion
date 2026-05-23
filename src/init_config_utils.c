#include "codexion.h"

int	init_watcher(t_config *config)
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

int	init_print(t_config *config)
{
	if (pthread_mutex_init(&config->print, NULL))
	{
		des_cond(config, 1, 1, -1);
		des_mtx(config, (t_params){1, 0, 1, 1, -1});
		return (0);
	}
	return (1);
}

int	init_operation(t_config *config)
{
	if (pthread_cond_init(&config->operation.op_cond, NULL))
	{
		des_cond(config, 1, 0, -1);
		des_mtx(config, (t_params){1, 0, 1, 0, -1});
		return (0);
	}
	if (pthread_mutex_init(&config->operation.op_lock, NULL))
	{
		des_cond(config, 1, 1, -1);
		des_mtx(config, (t_params){1, 0, 1, 0, -1});
		return (0);
	}
	return (1);
}

int	init_shuled(t_dongle *dongle, t_config *config, int i)
{
	if (pthread_mutex_init(&dongle->locker_d, NULL))
	{
		des_cond(config, 1, 1, -1);
		des_mtx(config, (t_params){1, 1, 1, 1, i});
		return (0);
	}
	dongle->scheduler.heap = malloc(sizeof(t_req) * config->number_coders);
	if (!dongle->scheduler.heap)
	{
		des_cond(config, 1, 1, -1);
		des_mtx(config, (t_params){1, 1, 1, 1, i + 1});
		return (0);
	}
	dongle->scheduler.size = 0;
	dongle->scheduler.mode = config->scheduler_mode;
	return (1);
}

void	assing_coder(t_coder *coder, t_dongle *dongles, int tot, int i)
{
	coder->f = &dongles[i];
	coder->l = &dongles[(i + tot - 1) % tot];
	if ((i + 1) % 2 == 0)
	{
		coder->f = &dongles[(i + tot - 1) % tot];
		coder->l = &dongles[i];
	}
}
