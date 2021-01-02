CXX = clang++
CC = clang


CXXFLAGS = -Wall -std=c++17 -Iinclude -Iinclude/KLU

CCFLAGS = -Iinclude/KLU

RETURNCODE =
AUFLAGS =
MAINFLAGS = -llo

OBJECTS = obj/junctions.o obj/klu_solver.o obj/AMD.o obj/BTF.o obj/COLAMD.o obj/KLU.o obj/SuiteSparse_config.o


UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	MAINFLAGS += -lasound
	AUFLAGS += -D__LINUX_ALSA__
endif
ifeq ($(UNAME_S),Darwin)
	MAINFLAGS += -framework CoreMIDI -framework CoreAudio -framework CoreFoundation
	AUFLAGS += -D__MACOSX_CORE__
endif


all: Circuit

# Main


Patch: patch.cpp
	@-rm -f obj/patch.o
	@$(CXX) -c -o obj/patch.o -fpic -shared -rdynamic $(CXXFLAGS) patch.cpp -ldl >> log.txt 2>&1 && (echo "Compilation: OK" && make patch.dylib) || (mv log.txt errlog.txt && echo "Compilation: ERROR")

patch.dylib: $(OBJECTS)
	@find ./ -iname \*.o -exec mv {} ./obj/ \; >> log.txt 2>&1
	@$(CXX) -dynamiclib -Wall -rdynamic -o patcher.dylib $(CXXFLAGS) obj/*.o -ldl >> log.txt 2>&1 && echo "Linking: OK" || (mv log.txt errlog.txt && echo "Linking: ERROR")

Circuit : main.cpp obj/patch.o
	find ./ -iname \*.o -exec mv {} ./obj/ \;
	$(CXX) -o bin/$@ $(CXXFLAGS) main.cpp obj/*.o $(MAINFLAGS) $(AUFLAGS)

obj/patch.o: patch.cpp
	@$(CXX) -c -o $@ $(CXXFLAGS) patch.cpp >> log.txt 2>&1

obj/klu_solver.o : src/klu_solver.cpp
	@$(CXX) -c -o $@ $(CXXFLAGS) src/klu_solver.cpp >> log.txt 2>&1

obj/junctions.o: src/junctions.cpp
	@$(CXX) -c -o $@ $(CXXFLAGS) src/junctions.cpp >> log.txt 2>&1

obj/AMD.o: lib/AMD
	@$(CC) -c $(CCFLAGS) $(AUFLAGS) lib/AMD/Source/*.c >> log.txt 2>&1

obj/BTF.o: lib/BTF
	@$(CC) -c $(CCFLAGS) $(AUFLAGS) lib/BTF/Source/*.c >> log.txt 2>&1

obj/COLAMD.o: lib/COLAMD
	@$(CC) -c $(CCFLAGS) $(AUFLAGS) lib/COLAMD/Source/*.c >> log.txt 2>&1

obj/KLU.o: lib/KLU
	@$(CC) -c $(CCFLAGS) $(AUFLAGS) lib/KLU/Source/*.c >> log.txt 2>&1

obj/SuiteSparse_config.o: lib/SuiteSparse_config
	@$(CC) -c $(CCFLAGS) $(AUFLAGS) lib/SuiteSparse_config/SuiteSparse_config.c >> log.txt 2>&1

patch:
	@ echo "" > log.txt
	@-rm -f *.dylib
	@make Patch

run:
	make
	./bin/Circuit

clean:
	-rm -f *.dylib
	-rm -f obj/*.o
	-rm -f obj/solvers/*.o
	-rm Circuit
	make
