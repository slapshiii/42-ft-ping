<<<<<<< HEAD
#include "ft_ping.h"

int	parse_arg(int ac, char **av) {
	for (int i = 1; i < ac; i++) {
		if (ft_strcmp(av[1], "-V") == 0) {
			ft_putendl_fd("ping from ft_ping v1", 1);
			exit(0);
		} else if (ft_strcmp(av[1], "-v") == 0) {
			;
		} else if (ft_strcmp(av[1], "-h") == 0) {
			return (1);
		}
	}
	return (0);
}

int main(int ac, char **av)
{
    if (parse_arg(ac, av)) {
		return (1);
	}
    return (0);
}
=======
#include <ft_ping.h>
>>>>>>> 759c14044795be5d2ea8a66254283c808d76308f
