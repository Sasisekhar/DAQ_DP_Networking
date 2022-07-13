### Sensor Fusion DEVS ###


### REAL TIME CADMIUM INSTALL ###

Clone this repo into an empty folder, ensure you download the MBed submodules:

> git submodule update --init --recursive

Run this to install dependencies

> SensorFusionDEVS-DataRecord/install.sh


### SIMULATE MODELS ###

> cd SensorFusionDEVS-DataRecord/top_model/

> make all

This will run the standard Cadmium simulator. Cadmium logs will be generated in SensorFusionDEVS-DataRecord/top_model/SensorFusion_Cadmium_output.txt
The outputs of the Publishers are stored in SensorFusionDEVS-DataRecord/outputs.
The pin's inputs are stored in SensorFusionDEVS-DataRecord/top_model/inputs.