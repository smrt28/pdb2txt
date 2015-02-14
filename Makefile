GCC=g++ -g

all: pdb2txt vlna_pipe

pdb2txt: main.o PDBFile.o RFile.o
	$(GCC) main.o PDBFile.o RFile.o -o pdb2txt

vlna_pipe: v.o
	$(GCC) v.o -o vlna_pipe     

main.o:
	$(GCC) -c main.cpp -o main.o

v.o:
	$(GCC) -c v.cpp -o v.o

PDBFile.o:
	$(GCC) -c PDBFile.cpp -o PDBFile.o

RFile.o:
	$(GCC) -c RFile.cpp -o RFile.o

clean:
	rm -f *.o pdb2txt* vlna_pipe*

main.o: main.cpp ptr.h RFile.h PDBFile.h
PDBFile.o: PDBFile.cpp ptr.h PDBFile.h RFile.h types.h
RFile.o: RFile.cpp ptr.h RFile.h
v.o: v.cpp ptr.h


