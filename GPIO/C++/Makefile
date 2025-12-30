CC = g++
CFLAGS = -Wall -Wextra
LDFLAGS = -llgpio

TARGET = gpio_control
SRC = gpio_control.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)
