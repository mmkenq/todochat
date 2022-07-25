WORK_DIR:= $(shell pwd)
.PHONY: clean

all: clean prepare xdg_shell graph_2d gui_core init_wayland
	gcc -Iinclude/wayland -lwayland-client $(WORK_DIR)/obj/* $(WORK_DIR)/main.c -o $(WORK_DIR)/main

prepare:
	mkdir -p $(WORK_DIR)/obj
# 	mkdir -p $(WORK_DIR)/tmp

clean:
	rm -f $(WORK_DIR)/main
	rm -f $(WORK_DIR)/log
	rm -f -r $(WORK_DIR)/obj
# 	rm -f -r $(WORK_DIR)/tmp

graph_2d:
	gcc -c -Iinclude/graph_2d $(WORK_DIR)/src/graph_2d/graph_2d.c -o $(WORK_DIR)/obj/graph_2d.o

gui_core:
	gcc -c -Iinclude/graph_2d -Iinclude/wayland $(WORK_DIR)/src/wayland/gui_core.c -o $(WORK_DIR)/obj/gui_core.o

init_wayland:
	gcc -c -Iinclude/graph_2d -Iinclude/wayland -Iinclude/DEFAULT_DEFINES $(WORK_DIR)/src/wayland/init_wayland.c -o $(WORK_DIR)/obj/init_wayland.o

xdg_shell:
	gcc -c $(WORK_DIR)/src/wayland/xdg_shell.c -o $(WORK_DIR)/obj/xdg_shell.o

