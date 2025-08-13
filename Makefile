OUT_DIR = out
CC = gcc
SRC = src/main.c
TARGET = $(OUT_DIR)/main

main: $(TARGET)

$(TARGET): $(SRC) $(OUT_DIR)
	$(CC) $(SRC) -o $(TARGET)

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

clean:
	rm -r $(OUT_DIR)
