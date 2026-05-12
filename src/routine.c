#include "codexion.h"

int	wait_coders(t_config *config)
{
	pthread_mutex_lock(&config->operation.op_lock);
	while (!config->operation.is_finish)
		pthread_cond_wait(&config->operation.op_cond,
			&config->operation.op_lock);
	if (config->operation.op_coders == config->number_coders + 1)
	{
		pthread_mutex_unlock(&config->operation.op_lock);
		return (1);
	}
	pthread_mutex_unlock(&config->operation.op_lock);
	return (0);
}

static void	req_dongle(t_dongle *dongle, t_coder *coder)
{
	pthread_mutex_lock(&dongle->locker_d);
	if (!dup_coder(dongle, coder->id))
		insert(dongle, coder);
	pthread_mutex_unlock(&dongle->locker_d);
}

static void	take_dongle(t_dongle *dongle, t_coder *coder, int *index)
{
	pthread_mutex_lock(&dongle->locker_d);
	log_hanlder(coder->config, coder->id, "has taken a dongle");
	dongle->is_taken = 1;
	delete_smaller(dongle);
	(*index)++;
	pthread_mutex_unlock(&dongle->locker_d);
}

/*
** FIX: wake_mtx must be LOCKED before entering the while loop because
** pthread_cond_wait atomically unlocks it while waiting.
** The caller (routine) locks wake_mtx before calling take_dongles.
** We unlock it exactly ONCE after the loop exits.
*/
static void	take_dongles(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*last;
	int			index;

	first = coder->f;
	last = coder->l;
	if (coder->l < coder->f)
	{
		last = coder->f;
		first = coder->l;
	}
	/* wake_mtx is already LOCKED by the caller - do NOT unlock here */
	while (
		!is_dongle_avaible(first)
		|| !is_dongle_avaible(last)
		|| get_smaller(first) != coder->id
		|| get_smaller(last) != coder->id
	)
	{
		if (should_stop(coder->config))
		{
			pthread_mutex_unlock(&coder->wake_mtx);
			return ;
		}
		pthread_cond_wait(&coder->wake_cond, &coder->wake_mtx);
	}
	pthread_mutex_unlock(&coder->wake_mtx); /* unlock ONCE after loop */
	if (should_stop(coder->config))
		return ;
	index = 0;
	take_dongle(first, coder, &index);
	take_dongle(last, coder, &index);
}

/*
** FIX: broadcast on ALL coders' wake_cond, not just the releasing coder.
** Without this, waiting coders depend on the watcher's 500µs poll to wake up,
** causing delays that can trigger spurious burnout.
*/
static void	release_dongle(t_dongle *dongle, t_config *config)
{
	int	i;

	pthread_mutex_lock(&dongle->locker_d);
	dongle->is_taken = 0;
	dongle->available_at = get_time_ms() + config->dongle_cooldown;
	pthread_mutex_unlock(&dongle->locker_d);
	i = 0;
	while (i < config->number_coders)
	{
		pthread_mutex_lock(&config->coders[i].wake_mtx);
		pthread_cond_broadcast(&config->coders[i].wake_cond);
		pthread_mutex_unlock(&config->coders[i].wake_mtx);
		i++;
	}
}

static void	compile(t_coder *coder)
{
	if (should_stop(coder->config))
		return ;
	pthread_mutex_lock(&coder->lock);
	coder->last_compile = get_time_ms();
	coder->is_waiting = 0; /* now compiling — watcher may check deadline again */
	pthread_mutex_unlock(&coder->lock);
	if (!log_hanlder(coder->config, coder->id, "is compiling"))
		return ;
	usleep(coder->config->time_to_compile * 1000);
	pthread_mutex_lock(&coder->c_lock);
	coder->compiles++;
	pthread_mutex_unlock(&coder->c_lock);
}

static void	debug(t_coder *coder)
{
	if (should_stop(coder->config))
		return ;
	if (!log_hanlder(coder->config, coder->id, "is debuging"))
		return ;
	usleep(coder->config->time_to_debug * 1000);
}

static void	refactor(t_coder *coder)
{
	if (should_stop(coder->config))
		return ;
	if (!log_hanlder(coder->config, coder->id, "is refactoring"))
		return ;
	pthread_mutex_lock(&coder->lock);
	coder->is_waiting = 1;
	pthread_mutex_unlock(&coder->lock);
	usleep(coder->config->time_to_refactor * 1000);
}

void	*routine(void *arg)
{
	t_coder		*coder;
	t_config	*config;

	coder = (t_coder *)arg;
	config = coder->config;
	if (!wait_coders(config))
		return (NULL);
	if (!coder->config->time_to_burnout)
		return (NULL);
	if (coder->id % 2 == 0)
		usleep(config->time_to_compile * 1000 / 2);
	if (config->number_coders == 1)
	{
		while (!should_stop(config))
			usleep(500);
		return (NULL);
	}
	while (1)
	{
		if (get_compile(coder) == config->required_compiles)
			break ;
		if (should_stop(config))
			break ;
		/*
		** Mark is_waiting=1 HERE — immediately after refactoring ends and
		** before we touch any dongle. This closes the race window where the
		** watcher could fire burnout between end-of-refactor and the moment
		** req_dongle sets is_waiting. The coder is actively seeking to compile.
		*/
		pthread_mutex_lock(&coder->lock);
		coder->is_waiting = 1;
		pthread_mutex_unlock(&coder->lock);
		coder->req_time = get_time_ms();
		if (coder->f < coder->l)
		{
			req_dongle(coder->f, coder);
			req_dongle(coder->l, coder);
		}
		else
		{
			req_dongle(coder->l, coder);
			req_dongle(coder->f, coder);
		}
		/* FIX: lock wake_mtx HERE, before take_dongles, so cond_wait
		** inside take_dongles receives a properly locked mutex. */
		pthread_mutex_lock(&coder->wake_mtx);
		take_dongles(coder);
		if (should_stop(config))
			break ;
		compile(coder);
		release_dongle(coder->f, config);
		release_dongle(coder->l, config);
		debug(coder);
		refactor(coder);
	}
	return (NULL);
}
