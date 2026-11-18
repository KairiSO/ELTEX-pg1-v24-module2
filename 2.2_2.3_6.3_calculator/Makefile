CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -fPIC
LDFLAGS = -ldl

APP = calculator
PLUGINS = plugins/add.so plugins/subtract.so plugins/multiply.so plugins/divide.so

all: $(APP) $(PLUGINS)

$(APP): src/main.c src/loader.c
	$(CC) $(CFLAGS) src/main.c src/loader.c -o $(APP) $(LDFLAGS)

plugins/%.so: plugins/%.c
	$(CC) $(CFLAGS) -shared $< -o $@

clean:
	rm -f $(APP) plugins/*.so