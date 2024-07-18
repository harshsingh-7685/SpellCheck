CC=gcc

CFLAGS = -Wall -std=c99

TARGET = spchk

DICT_PATH = ./words

TEST_PATH = ./tests/

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET) $(DICT_PATH) $(TEST_PATH)