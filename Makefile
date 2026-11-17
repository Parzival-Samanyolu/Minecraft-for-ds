# BlocksDS build (https://blocksds.skylyrac.net). The shared ARM9-only ROM
# makefile that ships with the SDK does the real work; this just says where
# this project's sources, headers and binary data live.

BLOCKSDS ?= /opt/blocksds/core

NAME          := minecraft-for-ds
GAME_TITLE    := Minecraft for DS
GAME_SUBTITLE := voxel sandbox homebrew

SOURCEDIRS  := source
INCLUDEDIRS := include
BINDIRS     := data

include $(BLOCKSDS)/sys/default_makefiles/rom_arm9/Makefile
