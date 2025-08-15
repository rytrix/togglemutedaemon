OUT_DIR = out
CC = gcc
# DEFINES = -DDEBUG
OPTIMIZE = -O2
OUT_FILE = togglemutedaemon 
INSTALL_DIR = ~/.local/bin

$(OUT_DIR)/$(OUT_FILE): $(OUT_DIR) $(OUT_DIR)/sounds $(OUT_DIR)/main.o $(OUT_DIR)/miniaudio.o
	$(CC) $(OUT_DIR)/main.o $(OUT_DIR)/miniaudio.o -o $(OUT_DIR)/$(OUT_FILE) -lm $(OPTIMIZE)

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

install: $(OUT_DIR)/$(OUT_FILE)
	install -m 755 $(OUT_DIR)/$(OUT_FILE) $(INSTALL_DIR)
	install -d $(INSTALL_DIR)/sounds
	install -m 644 $(OUT_DIR)/sounds/* $(INSTALL_DIR)/sounds

uninstall:
	-rm $(INSTALL_DIR)/$(OUT_FILE)
	-rm -r $(INSTALL_DIR)/sounds

