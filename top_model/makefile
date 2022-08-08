CC=g++
CFLAGS=-std=c++17

COMPILE_TARGET_DP=NUCLEO_H743ZI2
FLASH_TARGET_DP=NOD_H743ZI2

COMPILE_TARGET_DAQ=B_L4S5I_IOT01A
FLASH_TARGET_DAQ=DIS_L4S5VI

EXECUTABLE_NAME_DP=bin/DP_SIM
EXECUTABLE_NAME_DAQ=bin/DAQ_SIM
EXECUTABLE_NAME_DFC=bin/DFC_SIM

INCLUDRT_ARM_MBED=-I ../../cadmium/include
INCLUDEDESTIMES=-I ../../cadmium/DESTimes/include
INCLUDEBOOST=-I ../../boost_1_79_0
INCLUDE_JSON=-I ../../json/include


embedded_DAQ:
	mbed compile --target $(COMPILE_TARGET_DAQ) --toolchain GCC_ARM --profile ../DAQ_profile.json

embedded_DAQ_1:
	mbed compile --target $(COMPILE_TARGET_DP) --toolchain GCC_ARM --profile ../DAQ_profile_eth.json

embedded_DP:
	mbed compile --target $(COMPILE_TARGET_DP) --toolchain GCC_ARM --profile ../DP_profile.json

flash_DAQ:
	sudo cp ../BUILD/$(COMPILE_TARGET_DAQ)/GCC_ARM-DAQ_PROFILE/*.bin /media/$(USER)/$(FLASH_TARGET_DAQ)/
	$(info *** FLASH MAKE TAKE ~10 Seconds! DO NOT RESET WHILE COM PORT LED IS FLASHING! ***)

flash_DAQ_1:
	sudo cp ../BUILD/$(COMPILE_TARGET_DP)/GCC_ARM-DAQ_PROFILE_ETH/*.bin /media/$(USER)/$(FLASH_TARGET_DP)/
	$(info *** FLASH MAKE TAKE ~10 Seconds! DO NOT RESET WHILE COM PORT LED IS FLASHING! ***)

flash_DP:
	sudo cp ../BUILD/$(COMPILE_TARGET_DP)/GCC_ARM-DP_PROFILE/*.bin /media/$(USER)/$(FLASH_TARGET_DP)/
	$(info *** FLASH MAKE TAKE ~10 Seconds! DO NOT RESET WHILE COM PORT LED IS FLASHING! ***)

#*************************** DP ***************************#

DP_sim: main_DP fusion
ifeq (, $(wildcard bin))
	mkdir bin
endif
	$(CC) -DDP_COUPLED -g -o $(EXECUTABLE_NAME_DP) build_sim_DP/main.o build_sim_DP/Algorithm.o
	@echo "*** run ./$(EXECUTABLE_NAME_DP) to run the simulation ***"

main_DP: main.cpp
ifeq (, $(wildcard build_sim_DP))
	mkdir build_sim_DP
endif
	$(CC) -DDP_COUPLED -g -c $(CFLAGS) $(INCLUDRT_ARM_MBED) $(INCLUDEDESTIMES) $(INCLUDEBOOST) main.cpp -o build_sim_DP/main.o

fusion: ../drivers/Algorithm.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDRT_ARM_MBED) $(INCLUDEDESTIMES) $(INCLUDEBOOST) ../drivers/Algorithm.cpp -o build_sim_DP/Algorithm.o


#**********************************************************#

#*************************** DAQ **************************#

DAQ_sim: main_DAQ
ifeq (, $(wildcard bin))
	mkdir bin
endif
	$(CC) -DDAQ_COUPLED -g -o $(EXECUTABLE_NAME_DAQ) build_sim_DAQ/main.o
	@echo "*** run ./$(EXECUTABLE_NAME_DAQ) to run the simulation ***"

main_DAQ: main.cpp
ifeq (, $(wildcard build_sim_DAQ))
	mkdir build_sim_DAQ
endif
	$(CC) -DDAQ_COUPLED -g -c $(CFLAGS) $(INCLUDRT_ARM_MBED) $(INCLUDEDESTIMES) $(INCLUDEBOOST) main.cpp -o build_sim_DAQ/main.o

#**********************************************************#

#*************************** DFC **************************#

network_sim: main_DFC
ifeq (, $(wildcard bin))
	mkdir bin
endif
	$(CC) -DNETWORK_COUPLED -g -o $(EXECUTABLE_NAME_DFC) build_sim_DFC/main.o
	@echo "*** run ./$(EXECUTABLE_NAME_DFC) to run the simulation ***"

main_DFC: main.cpp
ifeq (, $(wildcard build_sim_DFC))
	mkdir build_sim_DFC
endif
	$(CC) -DNETWORK_COUPLED -g -c $(CFLAGS) $(INCLUDRT_ARM_MBED) $(INCLUDEDESTIMES) $(INCLUDEBOOST) $(INCLUDE_JSON) main.cpp -o build_sim_DFC/main.o

#**********************************************************#

#*************************** all_simulation **************************#

all:
	make DAQ_sim
	make network_sim
	make DP_sim

run:
	./bin/DAQ_SIM
	./bin/DFC_SIM
	./bin/DP_SIM

#**********************************************************#


clean:
	rm -rf build_sim_DAQ
	rm -rf build_sim_DP
	rm -rf build_sim_DFC

eclean:
	rm -rf ../BUILD
