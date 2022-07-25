CC=g++
CFLAGS=-std=c++17

COMPILE_TARGET_DP=NUCLEO_H743ZI2
FLASH_TARGET_DP=NOD_H743ZI2

COMPILE_TARGET_DAQ=B_L4S5I_IOT01A
FLASH_TARGET_DAQ=DIS_L4S5VI

EXECUTABLE_NAME=build_sim/DP_SIM

INCLUDRT_ARM_MBED=-I ../../cadmium/include
INCLUDEDESTIMES=-I ../../cadmium/DESTimes/include
INCLUDEBOOST=-I ../../boost_1_70_0


embedded_DAQ:
	mbed compile --target $(COMPILE_TARGET) --toolchain GCC_ARM --profile ../DAQ_profile.json

embedded_DP:
	mbed compile --target $(COMPILE_TARGET_DP) --toolchain GCC_ARM --profile ../DP_profile.json

flash_DAQ:
	sudo cp ../BUILD/$(COMPILE_TARGET)/GCC_ARM-CADMIUM/*.bin /media/$(USER)/$(FLASH_TARGET)/
	$(info *** FLASH MAKE TAKE ~10 Seconds! DO NOT RESET WHILE COM PORT LED IS FLASHING! ***)

flash_DP:
	sudo cp ../BUILD/$(COMPILE_TARGET_DP)/GCC_ARM-CADMIUM/*.bin /media/$(USER)/$(FLASH_TARGET_DP)/
	$(info *** FLASH MAKE TAKE ~10 Seconds! DO NOT RESET WHILE COM PORT LED IS FLASHING! ***)


all: main fusion
	$(CC) -g -o $(EXECUTABLE_NAME) build_sim/main.o build_sim/Algorithm.o
	@echo "*** run ./$(EXECUTABLE_NAME) to run the simulation ***"

main: main.cpp
	rm -rf build_sim
	mkdir build_sim
	$(CC) -g -c $(CFLAGS) $(INCLUDRT_ARM_MBED) $(INCLUDEDESTIMES) $(INCLUDEBOOST) main.cpp -o build_sim/main.o

fusion: ../drivers/Algorithm.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDRT_ARM_MBED) $(INCLUDEDESTIMES) $(INCLUDEBOOST) ../drivers/Algorithm.cpp -o build_sim/Algorithm.o

clean:
	rm -rf build_sim

eclean:
	rm -rf ../BUILD
