#include "codexion.h"

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
		&& scheduler->heap[rigth].propriete
		< scheduler->heap[smaller].propriete)
		smaller = rigth;
	return (smaller);
}

void	swap(t_req *a, t_req *b)
{
	t_req	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

void	heap_down(t_dongle *dongle, int i)
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

void	heap_up(t_dongle *dongle)
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
