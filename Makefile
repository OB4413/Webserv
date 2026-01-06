pro = webser
com = c++
flags = -Wall -Wextra -Werror -std=c++98

src = main.cpp ConfigFile/parse_configfile.cpp
obj = $(src:.cpp=.o)

all: $(pro) clean

$(pro): $(obj)
	$(com) $(flags) $(obj) -o $(pro)

%.o: %.cpp
	$(com) $(flags) -c $< -o $@

clean:
	rm -rf $(obj)

fclean: clean
	rm -rf $(pro)

re: fclean all