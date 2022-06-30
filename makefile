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

IncDir = include
LibDir = libs
BuildDir = build

TextBuildDir = $(LibDir)/text/build
TextSrcDir = $(LibDir)/text/src
TextIncDir = $(LibDir)/text/include

#------------------------------------------------ASSEMBLER COMPILATION BLOCK----------------------------------------------
AsmSrcDir = src/asm
AsmBuildDir = $(BuildDir)/asm

ASM_OBJECTS =	$(AsmBuildDir)/main.o $(AsmBuildDir)/labels.o $(AsmBuildDir)/assembler.o \
				$(TextBuildDir)/text.o $(TextBuildDir)/file.o

asm: $(ASM_OBJECTS)
	g++ $(ASM_OBJECTS) -o asm.exe

$(AsmBuildDir)/main.o: $(AsmSrcDir)/main.cpp $(TextIncDir)/text.h $(LibDir)/colors/colors.h $(IncDir)/asm/assembler.h
	g++ -I$(LibDir)/.. -I$(IncDir)/.. -c $(AsmSrcDir)/main.cpp $(CXXFLAGS) -o $(AsmBuildDir)/main.o

$(AsmBuildDir)/labels.o: $(AsmSrcDir)/labels.cpp $(IncDir)/asm/labels.h
	g++ -I$(IncDir)/.. -c $(AsmSrcDir)/labels.cpp $(CXXFLAGS) -o $(AsmBuildDir)/labels.o

$(AsmBuildDir)/assembler.o:	$(AsmSrcDir)/assembler.cpp $(IncDir)/asm/assembler.h $(TextIncDir)/text.h $(IncDir)/asm/labels.h \
							$(IncDir)/constants.h $(IncDir)/asm/settings.h $(LibDir)/debug/debug.h $(IncDir)/opdefs.h $(IncDir)/regdefs.h
	g++ -I$(LibDir)/.. -I$(IncDir)/.. -c $(AsmSrcDir)/assembler.cpp $(CXXFLAGS) -o $(AsmBuildDir)/assembler.o
#-------------------------------------------------------------------------------------------------------------------------


#------------------------------------------------PROCESSOR COMPILATION BLOCK----------------------------------------------
ProcBuildDir = build/processor
ProcSrcDir = src/processor

StackBuildDir	= $(LibDir)/stack/build
StackSrcDir		= $(LibDir)/stack/src
HashBuildDir	= $(LibDir)/hash/build

PROC_OBJS = $(ProcBuildDir)/main.o $(ProcBuildDir)/processor.o	\
			$(TextBuildDir)/text.o $(TextBuildDir)/file.o 		\
			$(StackBuildDir)/stack.o $(HashBuildDir)/hash.o

proc: $(PROC_OBJS)
	g++ $(PROC_OBJS) -o proc.exe

$(ProcBuildDir)/main.o: $(ProcSrcDir)/main.cpp $(IncDir)/processor/processor.h $(LibDir)/stack/include/stack.h $(TextIncDir)/text.h $(LibDir)/colors/colors.h $(LibDir)/debug/debug.h
	g++ -I$(LibDir)/.. -I$(IncDir)/.. -c $(ProcSrcDir)/main.cpp $(CXXFLAGS) -o $(ProcBuildDir)/main.o

$(ProcBuildDir)/processor.o: $(ProcSrcDir)/processor.cpp $(IncDir)/processor/processor.h $(IncDir)/processor/settings.h $(LibDir)/stack/include/stack.h $(TextIncDir)/text.h $(LibDir)/colors/colors.h $(LibDir)/debug/debug.h
	g++ -I$(LibDir)/.. -I$(IncDir)/.. -c $(ProcSrcDir)/processor.cpp $(CXXFLAGS) -o $(ProcBuildDir)/processor.o
#--------------------------------------------------------------------------------------------------------------------------


#-------------------------------------------------LIBRARY COMPILATION------------------------------------------------------
$(StackBuildDir)/stack.o: $(StackSrcDir)/stack.cpp
	"$(MAKE)" -C "$(LibDir)/stack" makefile init all

$(HashBuildDir)/hash.o:
	"$(MAKE)" -C "$(LibDir)/hash" makefile init all

$(TextBuildDir)/text.o $(TextBuildDir)/file.o: $(TextSrcDir)/text.cpp $(TextSrcDir)/file.cpp
	"$(MAKE)" -C "$(LibDir)/text" makefile init all
#--------------------------------------------------------------------------------------------------------------------------


.PHONY: init
init:
	mkdir -p $(BuildDir) $(AsmBuildDir) $(ProcBuildDir)
