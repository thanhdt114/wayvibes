TARGET = wayvibes
SRC = main.cpp

all: $(TARGET)

$(TARGET): $(SRC) miniaudio.h
	g++ -o $(TARGET) $(SRC) -levdev --verbose

install: $(TARGET)
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp $(TARGET) $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/$(TARGET)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all install clean uninstall
