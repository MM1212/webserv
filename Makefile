PROJECT_NAME = Webserv
NAME = webserv

SRC_FILES = Events/Events.cpp \
						utils/Logger.cpp utils/misc.cpp \
						Socket/Events.cpp Socket/Exceptions.cpp \
						Socket/Connection.cpp Socket/Socket.cpp \
						HTTP/Methods.cpp HTTP/Headers.cpp HTTP/Events.cpp \
						HTTP/Request.cpp HTTP/Response.cpp \
						HTTP/Route.cpp HTTP/Server.cpp \
						main.cpp

SRC_DIR = src

SRCS = $(addprefix $(SRC_DIR)/, $(SRC_FILES))

INCLUDES = -Iincludes

CXX = c++
CXXFLAGS = $(INCLUDES) -I. -Wall -Wextra -Werror -std=c++98  -g -fsanitize=address

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