WORK_DIR:= $(shell pwd)
.PHONY: clean

all: clean prepare xdg_shell surfaces graph_2d
	gcc -Iinclude/wayland -lwayland-client $(WORK_DIR)/obj/* $(WORK_DIR)/main.c -o $(WORK_DIR)/main

prepare:
	mkdir -p $(WORK_DIR)/obj

clean:
	rm -f $(WORK_DIR)/main
	rm -f -r $(WORK_DIR)/obj
# 	rm -f $(WORK_DIR)/tmp/*

graph_2d:
	gcc -c -Iinclude/graph_2d $(WORK_DIR)/src/graph_2d/graph_2d.c -o $(WORK_DIR)/obj/graph_2d.o

surfaces:
	gcc -c -Iinclude/graph_2d -Iinclude/wayland $(WORK_DIR)/src/wayland/surfaces.c -o $(WORK_DIR)/obj/surfaces.o

xdg_shell:
	gcc -c $(WORK_DIR)/src/wayland/xdg_shell.c -o $(WORK_DIR)/obj/xdg_shell.o

