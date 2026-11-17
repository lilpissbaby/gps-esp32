# Tests en el PC (no hace falta el ESP32):
#  - test_logic: batería, botones y diagnóstico del GPS
#  - test_sketch: el sketch entero con imitaciones de Arduino, SD, LovyanGFX y TinyGPS++
#  - check_sd_image: que sd/prueba.jpg existe y se puede dibujar
# Unity: https://github.com/ThrowTheSwitch/Unity (ruta configurable con UNITY=...)
UNITY ?= ../../vendor/Unity/src
SKETCH = ../gps_prueba
STUBS = sketch/stubs
CXXFLAGS = -std=gnu++17 -Wall -Wextra -Werror -I$(UNITY) -I$(SKETCH) -DUNITY_INCLUDE_DOUBLE
LOGIC = $(SKETCH)/battery.cpp $(SKETCH)/debouncer.cpp $(SKETCH)/gpsdiag.cpp

test: build/test_logic build/test_sketch
	./build/test_logic
	./build/test_sketch
	python3 -I check_sd_image.py ../sd/prueba.jpg

build/unity.o: $(UNITY)/unity.c
	mkdir -p build
	gcc -c -I$(UNITY) -DUNITY_INCLUDE_DOUBLE $< -o $@

build/test_logic: test_logic.cpp $(LOGIC) $(wildcard $(SKETCH)/*.h) build/unity.o
	g++ $(CXXFLAGS) test_logic.cpp $(LOGIC) build/unity.o -o $@

build/test_sketch: sketch/test_sketch.cpp $(SKETCH)/gps_prueba.ino $(LOGIC) $(wildcard $(SKETCH)/*.h) $(wildcard $(STUBS)/*) build/unity.o
	g++ $(CXXFLAGS) -I$(STUBS) -Wno-format-truncation sketch/test_sketch.cpp $(LOGIC) build/unity.o -o $@

clean:
	rm -rf build

.PHONY: test clean
