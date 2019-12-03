BINDIR = bin
OBJDIR = obj

LD = g++
CC = g++
CFLAGS = #-Wall -Wextra -Werror

LDFLAGS := -lGLEW -lGLU -lGL -lpthread
LDFLAGS += -lassimp
LDFLAGS += -lsfml-audio -lsfml-graphics -lsfml-window -lsfml-network -lsfml-system

IPATH = -I. -IEngine
LPATH =


SRC = $(shell find test -name '*.cpp')
INC = $(shell find test -name '*.h')

# Engine
SRC += $(shell find Engine -name '*.cpp')
INC += $(shell find Engine -name '*.h')

TARGETS		:= debug dev release
CFLAGS_debug	:= -DDEBUG -g3 -ggdb
CFLAGS_dev	:= -DPROFILE -DDEBUG -O2 -g -ggdb
CFLAGS_release	:= -O3

dev: #default target
all: $(TARGETS)


define Template
OBJ_$(1) := $(addprefix $(OBJDIR)/$(1)/, $(SRC:.cpp=.o))
DEP_$(1) := $(addprefix $(OBJDIR)/$(1)/, $(SRC:.cpp=.d))

$$(CFLAGS_$(1)) += $$(CFLAGS)

$(1): $(BINDIR)/$(1)

$(BINDIR)/$(1): $$(OBJ_$(1))
	$(LD) -o $$@ $$(OBJ_$(1)) $(LPATH) $(LDFLAGS)

$(OBJDIR)/$(1)/%.o: %.cpp
	$(CC) $$(CFLAGS_$(1)) $(IPATH) -MMD -c $$< -o $$@

-include $$(DEP_$(1))

$$(OBJ_$(1)): | $(OBJDIR)/$(1)

$(OBJDIR)/$(1):
	mkdir -p $$(dir $$(OBJ_$(1)))

clean_$(1):
	rm -rf $(OBJDIR)/$(1)
endef

$(foreach target,$(TARGETS),$(eval $(call Template,$(target))))


clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean
	make -j$(shell nproc) all
