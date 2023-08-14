MAKE_MT = --jobs=$(shell nproc) --output-sync=target 

PROJECT_NAME = Webserv
NAME = webserv

SRC_FILES = Settings.cpp utils/Logger.cpp utils/misc.cpp \
						Yaml/Node.cpp Yaml/Parser.cpp Yaml/tests.cpp \
						socket/Connection.cpp socket/Parallel.cpp \
						http/Methods.cpp http/Request.cpp http/PendingRequest.cpp \
						http/Response.cpp \
						http/Headers.cpp http/utils.cpp \
						http/WebSocket.cpp \
						http/DirectoryBuilder.cpp \
						http/routing/modules/Static.cpp http/routing/modules/Redirect.cpp \
						http/routing/types.cpp http/routing/mount.cpp http/routing/Module.cpp \
						http/Route.cpp http/routes/Default.cpp \
						http/ServerManager.cpp http/ServerConfiguration.cpp \
						main.cpp

SRC_DIR = src

SRCS = $(addprefix $(SRC_DIR)/, $(SRC_FILES))

OBJ_DIR = objs
OBJ_FILES = $(addprefix $(OBJ_DIR)/, $(SRCS:.cpp=.o))

DEP_DIR = deps
DEP_FILES = $(addprefix $(DEP_DIR)/, $(SRCS:.cpp=.d))

INCLUDES = includes

CXX = c++
CXXFLAGS = \
					-I$(INCLUDES) -I. \
					-MT $@ -MMD -MP -MF $(DEP_DIR)/$*.d \
					-Wall -Wextra -Werror -std=c++98 \
					-g -gdwarf-4 -fsanitize=address,undefined


### COLORS ###

RED = \033[0;31m
GREEN = \033[0;92m
YELLOW = \033[93m
BLUE = \033[0;34m
MAGENTA = \033[0;35m
CYAN = \033[96m
RESET = \033[0m

TAG = [$(CYAN)$(PROJECT_NAME)$(RESET)]

### END OF COLORS ###

all: $(NAME)

$(NAME): $(OBJ_FILES)
	@echo "$(TAG) compiling $(YELLOW)$(NAME)$(RESET).."
	@$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "$(CYAN)Done!$(RESET)"

$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR) $(DEP_DIR)
	@echo "$(TAG) compiling $(YELLOW)$<$(RESET).."
	@mkdir -p $(dir $@)
	@mkdir -p $(dir $(DEP_DIR)/$*.d)
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJ_DIR):
	@mkdir -p $@
$(DEP_DIR):
	@mkdir -p $@

$(DEP_FILES):

include $(wildcard $(DEP_FILES))

clean:
	@rm -rf $(OBJ_DIR)
	@echo "$(TAG) cleaned $(YELLOW)objects$(RESET)!"

fclean: clean
	@rm -f $(NAME)
	@echo "$(TAG) cleaned $(YELLOW)executable$(RESET)!"


re: fclean all

watch:
	@while true; do \
		make $(MAKE_MT) all --no-print-directory --no-print; \
		inotifywait -qre close_write --exclude ".*\.d" $(SRCS) $(INCLUDES); \
		echo "$(TAG) $(YELLOW)recompiling$(RESET).."; \
	done



.PHONY: all clean fclean re