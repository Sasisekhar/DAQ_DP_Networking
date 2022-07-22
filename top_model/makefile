CC=g++
CFLAGS=-std=c++17
COMPILE_TARGET=B_L4S5I_IOT01A
FLASH_TARGET=DIS_L4S5VI
EXECUTABLE_NAME=build/Testing_Algorithm_TOP

INCLUDRT_ARM_MBED=-I ../../cadmium/include
INCLUDEDESTIMES=-I ../../cadmium/DESTimes/include
INCLUDEBOOST=-I ../../boost_1_70_0


embedded:
	mbed compile --target $(COMPILE_TARGET) --toolchain GCC_ARM --profile ../cadmium.json

flash:
	sudo cp ../BUILD/$(COMPILE_TARGET)/GCC_ARM-CADMIUM/*.bin /media/$(USER)/$(FLASH_TARGET)/
	$(info *** FLASH MAKE TAKE ~10 Seconds! DO NOT RESET WHILE COM PORT LED IS FLASHING! ***)


all: main fusion
	$(CC) -g -o $(EXECUTABLE_NAME) build/main.o build/Algorithm.o
	$(info *** run ./$(EXECUTABLE_NAME) to run the simulation ***)

main: main.cpp
	make clean
	mkdir build
	$(CC) -g -c $(CFLAGS) $(INCLUDRT_ARM_MBED) $(INCLUDEDESTIMES) $(INCLUDEBOOST) main.cpp -o build/main.o

fusion: ../drivers/Algorithm.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDRT_ARM_MBED) $(INCLUDEDESTIMES) $(INCLUDEBOOST) ../drivers/Algorithm.cpp -o build/Algorithm.o

clean:
	rm -rf build

eclean:
	rm -rf ../BUILD
