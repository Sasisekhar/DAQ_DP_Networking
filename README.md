# DEVS Network Distributed Embedded Systems


## Installing Dependancies

Clone this repo into an empty folder, ensure you download the MBed submodules:

`git submodule update --init --recursive`

Run this to install dependencies

`DAQ_DP_Networking/install.sh`


## Running the Simulation

```
cd DAQ_DP_Networking/top_model/

# To build absolutely everything including the bin files for the 2 models and the executables for the simulation, run:-
make all

# To build all of the individual components, run (respectively):-
make all_DAQ
make all_DP
make embedded_DAQ
make embedded_DP

# To run the simulations, run (respectively):-
cd bin
./DP_SIM
./DAQ_SIM

# To flash the boards run (respectively):-
make flash_DAQ
make flash_DP

# To see the UART logged data from the MCUs, run:-
mbed sterm

# To cleanup, run
make clean; make eclean
```


Cadmium logs will be generated in `DAQ_DP_Networking/top_model/MessageOutputs.txt` (Contains the logs of the simulation you last ran)

The outputs of the Publishers are stored in `DAQ_DP_Networking/top_model/outputs`

The Sensor inputs and the subscriber are stored in `DAQ_DP_Networking/top_model/inputs`
