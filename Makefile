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
						http/Route.cpp http/routes/Default.cpp http/routes/Redirect.cpp http/routes/Static.cpp \
						http/RouteStorage.cpp http/ServerManager.cpp http/ServerConfiguration.cpp \
						main.cpp

SRC_DIR = src

SRCS = $(addprefix $(SRC_DIR)/, $(SRC_FILES))

INCLUDES = -Iincludes

CXX = c++
CXXFLAGS = $(INCLUDES) -I. -Wall -Wextra -Werror -std=c++98 -g -gdwarf-4 -fsanitize=address,undefined

OBJ_DIR = objs
OBJ_FILES = $(addprefix $(OBJ_DIR)/, $(SRCS:.cpp=.o))

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

$(OBJ_DIR)/%.o: %.cpp
	@echo "$(TAG) compiling $(YELLOW)$<$(RESET).."
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	@rm -rf $(OBJ_DIR)
	@echo "$(TAG) cleaned $(YELLOW)objects$(RESET)!"

fclean: clean
	@rm -f $(NAME)
	@echo "$(TAG) cleaned $(YELLOW)executable$(RESET)!"


re: fclean all

.PHONY: all clean fclean re