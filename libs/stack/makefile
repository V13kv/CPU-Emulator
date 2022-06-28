CXXFLAGS = -g -std=c++17 -Wall -Wextra -Weffc++ -Wc++0x-compat -Wc++11-compat -Wc++14-compat -Waggressive-loop-optimizations \
-Walloc-zero -Walloca -Walloca-larger-than=8192 -Warray-bounds -Wcast-align -Wcast-qual -Wchar-subscripts \
-Wconditionally-supported -Wconversion -Wctor-dtor-privacy -Wdangling-else -Wduplicated-branches -Wempty-body -Wfloat-equal \
-Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Wformat-overflow=2 -Wformat-truncation=2 -Winline \
-Wlarger-than=8192 -Wvla-larger-than=8192 -Wlogical-op -Wmissing-declarations -Wnon-virtual-dtor -Wopenmp-simd \
-Woverloaded-virtual -Wpacked -Wpointer-arith -Wredundant-decls  -Wrestrict -Wshadow -Wsign-promo -Wstack-usage=8192 \
-Wstrict-null-sentinel -Wstrict-overflow=2 -Wstringop-overflow=4 -Wsuggest-attribute=noreturn -Wsuggest-final-types \
-Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wvariadic-macros \
-Wno-literal-suffix -Wno-missing-field-initializers -Wnarrowing -Wno-old-style-cast -Wvarargs -Waligned-new \
-Walloc-size-larger-than=1073741824 -Walloc-zero -Walloca -Walloca-larger-than=8192 -Wdangling-else \
-Wduplicated-branches -Wformat-overflow=2 -Wformat-truncation=2 -Wmissing-attributes -Wmultistatement-macros -Wrestrict \
-Wshadow=global -Wsuggest-attribute=malloc -fcheck-new -fsized-deallocation -fstack-check -fstrict-overflow \
-flto-odr-type-merging -fno-omit-frame-pointer -Wno-unknown-pragmas

SrcDir = src
IncDir = include
LibDir = ../
BuildDir = build

ColorsDir = $(LibDir)/colors
DebugDir = $(LibDir)/debug

HashBuildDir = $(LibDir)/hash/build
HashSrcDir = $(LibDir)/hash/src
HashIncDir = $(LibDir)/hash/include


all: $(BuildDir)/stack.o $(HashBuildDir)/hash.o

$(BuildDir)/stack.o: $(SrcDir)/stack.cpp $(IncDir)/stack.h $(DebugDir)/debug.h $(IncDir)/settings.h $(LibDir)/hash/include/hash.h
	g++ -I$(IncDir)/.. -I$(LibDir)/.. -c $(SrcDir)/stack.cpp $(CXXFLAGS) -o $(BuildDir)/stack.o

$(HashBuildDir)/hash.o: 
	"$(MAKE)" -C "$(LibDir)/hash/" makefile init all

.PHONY: init
init:
	mkdir -p $(BuildDir)
