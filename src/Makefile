PROJECT_NAME=Algorithmic_trading

CC=g++ -std=c++17

SRCFILES=main.cc model/*.cc view/*.cc
HDRFILES=model/*.h view/*.h

OS = $(shell uname)

.PHONY: all install uninstall clean

all: install

build: uninstall
	@cmake -S . -B ./build && cmake --build ./build

clean:
	@rm -rf ./build

install: clean uninstall build
	@mkdir -p $(HOME)/$(PROJECT_NAME)
	@cp -rf ./build/$(PROJECT_NAME).app $(HOME)/$(PROJECT_NAME)
	@mkdir -p $(HOME)/$(PROJECT_NAME)/materials
	@cp -rf ../materials/*.csv $(HOME)/$(PROJECT_NAME)/materials
	make clean
ifeq ($(OS),Darwin)
	@open $(HOME)/$(PROJECT_NAME)/
endif

uninstall:
	rm -rf $(HOME)/$(PROJECT_NAME)/

