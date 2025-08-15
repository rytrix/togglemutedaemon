OUT_DIR = out
CC = gcc
# DEFINES = -DDEBUG
OPTIMIZE = -O2
OUTFILE = tmd 

$(OUT_DIR)/$(OUTFILE): $(OUT_DIR) $(OUT_DIR)/sounds $(OUT_DIR)/main.o $(OUT_DIR)/miniaudio.o
	$(CC) $(OUT_DIR)/main.o $(OUT_DIR)/miniaudio.o -o $(OUT_DIR)/$(OUTFILE) -lm $(OPTIMIZE)

$(OUT_DIR)/main.o: src/main.c
	$(CC) src/main.c -o $(OUT_DIR)/main.o -c $(DEFINES) $(OPTIMIZE)

$(OUT_DIR)/miniaudio.o: external/miniaudio.c
	$(CC) external/miniaudio.c -o $(OUT_DIR)/miniaudio.o -lm -c $(OPTIMIZE)

$(OUT_DIR)/sounds:
	cp -r sounds $(OUT_DIR)

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

clean:
	rm -r $(OUT_DIR)
