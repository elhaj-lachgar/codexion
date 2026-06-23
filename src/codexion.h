/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elachgar <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/23 18:22:04 by elachgar          #+#    #+#             */
/*   Updated: 2026/05/23 18:36:21 by elachgar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#ifndef CODEXION_H
# define CODEXION_H

# include <limits.h>
# include <string.h>
# include <stdlib.h>
# include <stdio.h>
# include <pthread.h>
# include <sys/time.h>
# include <unistd.h>

typedef struct s_config	t_config;
typedef struct s_coder	t_coder;

typedef struct s_params
{
	int	t;
	int	p;
	int	s;
	int	op;
	int	d;
}	t_params;

typedef struct s_req
{
	t_coder		*coder;
	long		propriete;
}	t_req;

typedef struct s_op
{
	int				is_finish;
	int				op_coders;
	pthread_mutex_t	op_lock;
	pthread_cond_t	op_cond;
}	t_op;

typedef struct s_scheduler
{
	int		mode;
	t_req	*heap;
	int		size;
}	t_scheduler;

typedef struct s_dongle
{
	pthread_mutex_t	locker_d;
	int				id;
	int				is_taken;
	long			available_at;
	t_scheduler		scheduler;
}	t_dongle;

typedef struct s_coder
{
	int				id;
	pthread_mutex_t	lock;
	pthread_mutex_t	c_lock;
	pthread_cond_t	wake_cond;
	pthread_mutex_t	wake_mtx;
	pthread_t		thread;
	t_config		*config;
	long			last_compile;
	long			req_time;
	int				compiles;
	t_dongle		*f;
	t_dongle		*l;
}	t_coder;

typedef struct s_config
{
	int				number_coders;
	int				required_compiles;
	long			time_to_burnout;
	long			time_to_compile;
	long			time_to_debug;
	long			time_to_refactor;
	long			dongle_cooldown;
	long			start;
	int				scheduler_mode;
	pthread_mutex_t	lock_stop;
	pthread_cond_t	cond_stop;
	pthread_mutex_t	print;
	pthread_mutex_t	time_lock;
	pthread_t		watcher;
	int				stop_watcher;
	t_op			operation;
	t_coder			*coders;
	t_dongle		*dongles;
}	t_config;

long		ft_atoi(char *nbr, int *not_digit, int *overflow);
int			is_valid(char **arv, int arc);
long		get_time_ms(void);
void		free_handler(t_config *config, int isfinish);
void		insert(t_dongle *dongle, t_coder *coder);
int			get_smaller(t_dongle *dongle);
void		delete_smaller(t_dongle *dongle);
int			log_hanlder(t_config *config, int id, char *msg);
void		*routine(void *args);
void		*watcher(void *args);
int			start_coders(t_config *config);
void		des_mtx(t_config *config, t_params params);
void		des_cond(t_config *config, int s, int op, int d);
int			init_config(t_config *config);
int			should_stop(t_config *config);
int			wait_coders(t_config *config);
int			is_finish(t_config *config);
int			get_compile(t_coder *coder);
long		get_last_compile(t_coder *coder);
int			is_dongle_avaible(t_dongle *dongle);
void		des_coder_mtx(t_config *config, int c_l, int c_c, int w_c);
void		swap(t_req *a, t_req *b);
void		heap_down(t_dongle *dongle, int i);
void		heap_up(t_dongle *dongle);
void		assing_coder(t_coder *coder, t_dongle *dongles, int tot, int i);
int			init_shuled(t_dongle *dongle, t_config *config, int i);
int			init_operation(t_config *config);
int			init_print(t_config *config);
int			init_watcher(t_config *config);
int			wait_coders(t_config *config);
int			add_validator(int arc);
int			add_validator1(int index, int value);
void		req_dongle(t_dongle *dongle, t_coder *coder);
void		take_dongle(t_dongle *dongle, t_coder *coder);
void		take_dongles(t_coder *coder);
void		assign_req(t_coder *coder);
void		release_dongle(t_dongle *dongle, t_config *config);
void		my_sleep(long time, t_config *config, int add);
t_config	*set_config(char **arv);

#endif
