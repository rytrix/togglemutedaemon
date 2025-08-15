OUT_DIR = out
CC = gcc

$(OUT_DIR)/main: $(OUT_DIR) $(OUT_DIR)/main.o $(OUT_DIR)/miniaudio.o
	$(CC) $(OUT_DIR)/main.o $(OUT_DIR)/miniaudio.o -o $(OUT_DIR)/main -lm

$(OUT_DIR)/main.o: src/main.c
	$(CC) src/main.c -o $(OUT_DIR)/main.o -c

$(OUT_DIR)/miniaudio.o: external/miniaudio.c
	$(CC) external/miniaudio.c -o $(OUT_DIR)/miniaudio.o -lm -c

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

clean:
	rm -r $(OUT_DIR)
