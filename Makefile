OUT_DIR = out
CC = gcc
# DEFINES = -DDEBUG
OPTIMIZE = -O2
OUT_FILE = togglemutedaemon 
INSTALL_DIR = ~/.local/bin

OUT_OBJECTS = $(OUT_DIR)/main.o $(OUT_DIR)/miniaudio.o $(OUT_DIR)/watch_key.o $(OUT_DIR)/server.o $(OUT_DIR)/client.o $(OUT_DIR)/helpers.o $(OUT_DIR)/arg_parser.o

$(OUT_DIR)/$(OUT_FILE): $(OUT_DIR) $(OUT_DIR)/sounds $(OUT_OBJECTS) 
	$(CC) $(OUT_OBJECTS) -o $(OUT_DIR)/$(OUT_FILE) -lm -latomic -lpthread $(OPTIMIZE)

$(OUT_DIR)/main.o: src/main.c
	$(CC) src/main.c -o $(OUT_DIR)/main.o -c $(DEFINES) $(OPTIMIZE)

$(OUT_DIR)/server.o: src/server.c
	$(CC) src/server.c -o $(OUT_DIR)/server.o -c $(DEFINES) $(OPTIMIZE)

$(OUT_DIR)/client.o: src/client.c
	$(CC) src/client.c -o $(OUT_DIR)/client.o -c $(DEFINES) $(OPTIMIZE)

$(OUT_DIR)/helpers.o: src/helpers.c
	$(CC) src/helpers.c -o $(OUT_DIR)/helpers.o -c $(DEFINES) $(OPTIMIZE)

$(OUT_DIR)/arg_parser.o: src/arg_parser.c
	$(CC) src/arg_parser.c -o $(OUT_DIR)/arg_parser.o -c $(DEFINES) $(OPTIMIZE)

$(OUT_DIR)/watch_key.o: src/watch_key.c
	$(CC) src/watch_key.c -o $(OUT_DIR)/watch_key.o -c $(DEFINES) $(OPTIMIZE)

$(OUT_DIR)/miniaudio.o: external/miniaudio.c
	$(CC) external/miniaudio.c -o $(OUT_DIR)/miniaudio.o -lm -c $(OPTIMIZE)

$(OUT_DIR)/sounds:
	cp -r sounds $(OUT_DIR)

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

clean:
	rm -r $(OUT_DIR)

install: $(OUT_DIR)/$(OUT_FILE)
	install -m 755 $(OUT_DIR)/$(OUT_FILE) $(INSTALL_DIR)
	install -d $(INSTALL_DIR)/sounds
	install -m 644 $(OUT_DIR)/sounds/* $(INSTALL_DIR)/sounds

uninstall:
	-rm $(INSTALL_DIR)/$(OUT_FILE)
	-rm -r $(INSTALL_DIR)/sounds

