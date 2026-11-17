# Compila y ejecuta los tests de la lógica en el PC (no hace falta el ESP32).
# Unity: https://github.com/ThrowTheSwitch/Unity (ruta configurable con UNITY=...)
UNITY ?= ../../vendor/Unity/src
SKETCH = ../gps_prueba
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -I$(UNITY) -I$(SKETCH) -DUNITY_INCLUDE_DOUBLE

SRCS = test_logic.cpp $(SKETCH)/battery.cpp $(SKETCH)/debouncer.cpp $(SKETCH)/gpsdiag.cpp

test: build/test_logic
	./build/test_logic

build/test_logic: $(SRCS) $(wildcard $(SKETCH)/*.h)
	mkdir -p build
	gcc -c -I$(UNITY) -DUNITY_INCLUDE_DOUBLE $(UNITY)/unity.c -o build/unity.o
	g++ $(CXXFLAGS) $(SRCS) build/unity.o -o $@

clean:
	rm -rf build

.PHONY: test clean
