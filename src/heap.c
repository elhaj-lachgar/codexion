#include "codexion.h"

static void	swap(t_req *a, t_req *b)
{
	t_req	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

static int	compaire(int parent, t_scheduler *scheduler)
{
	int	smaller;
	int	left;
	int	rigth;

	smaller = parent;
	left = parent * 2 + 1;
	rigth = parent * 2 + 2;
	if (left < scheduler->size
		&& scheduler->heap[left].propriete < scheduler->heap[smaller].propriete)
		smaller = left;
	if (rigth < scheduler->size
		&& scheduler->heap[rigth].propriete < scheduler->heap[smaller].propriete)
		smaller = rigth;
	return (smaller);
}

static void	heap_down(t_dongle *dongle, int i)
{
	int	smaller;
	int	parent;

	while (1)
	{
		parent = i;
		smaller = compaire(parent, &dongle->scheduler);
		if (smaller == parent)
			break ;
		swap(&dongle->scheduler.heap[parent], &dongle->scheduler.heap[smaller]);
		i = smaller;
	}
}

static void	heap_up(t_dongle *dongle)
{
	int	curr;
	int	parent;

	curr = dongle->scheduler.size - 1;
	while (curr > 0)
	{
		parent = (curr - 1) / 2;
		if (dongle->scheduler.heap[parent].propriete
			> dongle->scheduler.heap[curr].propriete)
			swap(&dongle->scheduler.heap[curr],
				&dongle->scheduler.heap[parent]);
		else
			break ;
		curr = parent;
	}
}

void	insert(t_dongle *dongle, t_coder *coder)
{
	int		index;
	long	last;

	index = dongle->scheduler.size;
	dongle->scheduler.heap[index].coder = coder;
	if (dongle->scheduler.mode)
	{
		last = get_last_compile(coder);
		dongle->scheduler.heap[index].propriete =
			last + coder->config->time_to_burnout;
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
