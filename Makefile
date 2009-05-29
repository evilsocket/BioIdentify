CFLAGS= -Wunused -O3 -funroll-loops -fno-rtti -fomit-frame-pointer -ffast-math -fno-stack-protector -ffunction-sections
LFLAGS= -lpthread -lX11 

all:
	g++ -c biofinger.cpp -o obj/biofinger.o $(CFLAGS)
	g++ -c biofloatmap.cpp -o obj/biofloatmap.o $(CFLAGS)
	g++ -c biohistogram.cpp -o obj/biohistogram.o $(CFLAGS)
	g++ -c bioidrecord.cpp -o obj/bioidrecord.o $(CFLAGS)
	g++ -c bioimage.cpp -o obj/bioimage.o $(CFLAGS)
	g++ -c bioerror.cpp -o obj/bioerror.o $(CFLAGS)
	g++ main.cpp -o bioid obj/*.o $(CFLAGS) $(LFLAGS)
	
debug:
	g++ *.cpp -o bioid -g3 $(LFLAGS)
	
profile:
	g++ *.cpp -o bioid -pg -g3 $(LFLAGS)

clean:
	rm obj/*.o bioid
