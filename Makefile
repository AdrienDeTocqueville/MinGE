NAME = bin/minge
CC = g++
CFLAGS = #-Wall -Wextra -Werror


#LDFLAGS := -lstdc++ -lglib-2.0 -lpthread -lm
LDFLAGS := -lGLEW -lGLU -lGL
LDFLAGS += -lsfml-audio -lsfml-graphics -lsfml-window -lsfml-network -lsfml-system

IPATH = -I. -IEngine
LPATH =


SRC = $(shell find test -name '*.cpp')
#INC = $(shell find test -name '*.h')

# Engine
SRC += $(shell find Engine -name '*.cpp')
INC += $(shell find Engine -name '*.h')


OBJDIR = obj
OBJ := $(addprefix $(OBJDIR)/, $(SRC:.cpp=.o))


all: $(NAME)
#all: debug

debug: CFLAGS += -DDEBUG -g3 -ggdb
debug: $(NAME)

run: debug
	cd bin
	./minge


$(NAME): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LPATH) $(LDFLAGS)

$(OBJDIR)/%.o: %.cpp $(INC)
	$(CC) $(CFLAGS) $(IPATH) -c $< -o $@

$(OBJ): | $(OBJDIR)

$(OBJDIR):
	mkdir -p $(dir $(OBJ))

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean
	make -j4 all
