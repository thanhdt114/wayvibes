TARGET = wayvibes
SRC = main.cpp

all: $(TARGET)

$(TARGET): $(SRC) miniaudio.h
	g++ -o $(TARGET) $(SRC) -levdev --verbose

install: $(TARGET)
	install -Dm755 $(TARGET) -t /usr/local/bin

uninstall:
	rm -f /usr/local/bin/$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all install clean uninstall
