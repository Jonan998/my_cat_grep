CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11
TEST_CFLAGS = -Wall -Wextra -std=c11 -D_DEFAULT_SOURCE
TEST_FLAGS = -lcheck -lsubunit -lm -lpthread

SRC = s21_cat.c
OBJ = $(SRC:.c=.o)
EXEC = s21_cat
TEST_EXEC = test_cat
TEST_SRC = test_cat.c

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TEST_EXEC)
	./$(TEST_EXEC)

$(TEST_EXEC): $(TEST_SRC) $(EXEC)
	$(CC) $(TEST_CFLAGS) -o $@ $(TEST_SRC) $(TEST_FLAGS)

clean:
	rm -f $(OBJ) $(EXEC) $(TEST_EXEC)

.PHONY: all test clean
