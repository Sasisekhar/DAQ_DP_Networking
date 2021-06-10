### Sensor Fusion DEVS ###


### REAL TIME CADMIUM INSTALL ###

Clone this repo into an empty folder, ensure you download the MBed submodules:

> git submodule update --init --recursive

Run this to install dependencies

> SensorFusionDEVS-DataRecord/install.sh


### SIMULATE MODELS ###

> cd SensorFusionDEVS-DataRecord/top_model/

> make clean; make all

> ./Testing_Algorithm_TOP

This will run the standard Cadmium simulator. Cadmium logs will be generated in SensorFusionDEVS-DataRecord/top_model/SensorFusion_Cadmium_output.txt
The pin's inputs are stored in SensorFusionDEVS-DataRecord/top_model/inputs. The value of the output pins will be in DEVS_BCA_Cadmium/top_model/inputs.

Now, to parse the data, just run the Parser.py file in the SensorFusionDEVS-DataRecord/top_model folder and it will create an Output.csv file in the same folder. This output can be used to see what inputs were given and what Fused output is obtained.
