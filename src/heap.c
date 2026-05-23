#include "codexion.h"

void	insert(t_dongle *dongle, t_coder *coder)
{
	int		index;
	long	last;

	index = dongle->scheduler.size;
	dongle->scheduler.heap[index].coder = coder;
	if (dongle->scheduler.mode)
	{
		last = get_last_compile(coder);
		dongle->scheduler.heap[index].propriete = last
			+ coder->config->time_to_burnout;
	}
	else
		dongle->scheduler.heap[index].propriete = coder->req_time;
	dongle->scheduler.size++;
	heap_up(dongle);
}

int	get_smaller(t_dongle *dongle)
{
	if (!dongle->scheduler.size)
		return (-1);
	return (dongle->scheduler.heap[0].coder->id);
}

void	delete_smaller(t_dongle *dongle)
{
	int	index;

	if (dongle->scheduler.size == 0)
		return ;
	index = dongle->scheduler.size - 1;
	dongle->scheduler.heap[0] = dongle->scheduler.heap[index];
	dongle->scheduler.size--;
	heap_down(dongle, 0);
}

int	dup_coder(t_dongle *dongle, int id)
{
	int	i;

	i = 0;
	while (i < dongle->scheduler.size)
	{
		if (dongle->scheduler.heap[i].coder->id == id)
			return (1);
		++i;
	}
	return (0);
}
