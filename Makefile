
NAME :=	ft_ping
SRCDIR := src
OBJDIR := obj

H_FILES :=	$(shell find -L ./ -name '*.h' -exec dirname {} \; | sed 's/ /\\ /g' | uniq)
C_FILE =	ft_ping.c ft_ping_tools.c 

INCLUDES :=	$(H_FILES:%=-I%)
SRCS =	$(addprefix $(SRCDIR)/, $(C_FILE))
OBJS = 	$(addprefix $(OBJDIR)/, $(C_FILE:%.c=%.o))

# CFLAGS += -Wall -Werror -Wextra

CC := gcc

all:	${NAME}

${NAME}: ${OBJS} libft.a
	${CC} ${CFLAGS} -L ./lib/libft/ -o ${NAME} ${OBJS} -lft

$(OBJDIR)/%.o:	$(SRCDIR)/%.c ${H_FILES}
	@mkdir -p $(OBJDIR)
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@

libft.a:
	cd lib/libft && make all

clean:
	-rm -rf ${OBJDIR}
	cd lib/libft && make clean

fclean:		clean
	-rm -rf ${NAME} a.out *.txt
	cd lib/libft && make fclean

re:		fclean all

.PHONY:		all clean fclean re