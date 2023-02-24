
NAME :=	ft_ping
SRCDIR := src
OBJDIR := obj
LIBFTDIR := libft

SUBDIR := libft
INC_DIRS = -I./ $(addprefix -I, $(SUBDIR))
LIB_DIRS = -L./ -L./libft

C_FILE =	main.c

SRCS =	$(addprefix $(SRCDIR)/, $(C_FILE))
OBJS = 	$(addprefix $(OBJDIR)/, $(C_FILE:%.c=%.o))

# CFLAGS += -Wall -Werror -Wextra

CC=gcc

all:	${NAME}

${NAME}: ${OBJS}
	${CC} ${CFLAGS} -o ${NAME}${OBJS}

$(OBJDIR)/%.o:	$(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	${CC} ${CFLAGS} -c $< -o $@

clean:
	-rm -rf ${OBJDIR}

fclean:		clean
	-rm -rf ${NAME} a.out *.txt

re:		fclean all

.PHONY:		all clean fclean re