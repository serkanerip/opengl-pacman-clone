build:
	g++ -o build/main src/main.cc src/glad.c -lglfw -lGL -lm -ldl -Iinclude

clean:
	rm -f main

.PHONY: build clean