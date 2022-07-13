# DEVS Network Distributed Embedded Systems


## Installing Dependancies

Clone this repo into an empty folder, ensure you download the MBed submodules:

`git submodule update --init --recursive`

Run this to install dependencies

`DAQ_DP_Networking/install.sh`


## Running the Simulation

```
cd DAQ_DP_Networking/top_model/
make all
```

This will run the standard Cadmium simulator.

Cadmium logs will be generated in `DAQ_DP_Networking/top_model/MessageOutputs.txt`

The outputs of the Publishers are stored in `DAQ_DP_Networking/top_model/outputs`

The Sensor inputs are stored in `DAQ_DP_Networking/top_model/inputs`

## Block Diagram

![block_diagram](Docs/BlockDiagram.png)

- Coupled Models :- 
  - DAQ
    - Sensor 1 - Sensor n
    - DAQ_Packetizer
    - Publisher
  - DP
    - Subscriber 1, Subscriber 2, Subscriber 3
    - Data_Parser 1, Data_Parser 2, Data_Parser 3
    - Fusion 1, Fusion 2, Fusion 3
    - DP_Packetizer
    - Publisher
  - TOP
    - DAQ
    - DP
    - NETWORK

## Formal Specification
#### DAQ
- Sensor Atomics
```
S = {}
X = {}
Y = {SensorData}
δint { //do nothing}
δext { //do nothing}
λ {
    SensorRead = readsensor()
    Send SensorRead to port SensorData.
}
ta = pollingRate
```
- DAQ_Packetizer
```
S = {active, passive}
X = {Sensor1in, Sensor2in..., SensorNin}
Y = {JSONDataOut}
δint () = {}
δext () {
    sprintf(Buffer, “{\”Temp\”:[%d, %d...], ”Hum\”:[%d, %d...], ”CO\”:[%d, %d...]}”, Sensor1in, ...);
    state = active
}
λ (active) {
    if Buffer is valid:
    send Buffer to the output port JSONDataOut
}
ta(passive) = infinity
ta(active) = ProcessingTime
```

- Publisher
```
S = {active, passive}
X = {DataIn}
Y = {}
δint (active) = passive
δext (CSVDataIn, passive) {
    client.publish(“<UID>/Raw/ALL”, (const char*) DataIn);
    state = active
}
δext () = active
λ (active) {}
ta(passive) = infinity
ta(active) = ProcessingTime
```

#### Data Processor
- Subscriber Atomics
```
S = {}
X = {}
Y = {Message}
δint () = {}
δext () = {}
λ {
    Recieve(A, B) //topic is discarded
    Message = B
}
ta() = pollingTime
```
- Data_Parser Atomics
```
S = {active, passive, values[3]}
X = {in}
Y = {out1, out2, out3}
δint (active) = passive
δext ((Topic, Message), passive){
    Parse string input from <in>
    store the parsed and converted calues in <values[]>
    state = active
}

λ (active, State){
    send <values[]> to the out ports iteratively (3 values go to 3 the 3 respective ports)
}

ta(passive) = infinity
ta(active) = Processing time
```
- Fusion Atomics
```
S = {active, passive, fused}
X = {in1, in2, in3}
Y = {out}
δint () = {}
δext (in, passive) { 
    fused = ALGORITHM(in1, in2, in3)
    state = active
}
λ (active) {
	out = fused
}
ta(passive) = infinity
ta(active) = ProcessingTime
```

- DP_Packetizer
```
S = {active, passive, buffer}
X = {T, H, C}
Y = {JSONDataOut}
δint () = {}
δext () {
    sprintf(buffer, “{\”Temp\”:%d, ”Hum\”:%d, ”CO\”:%d}”, T, H, C)
    state = active
}
λ (active) {
    if Buffer is valid:
    send Buffer to the output port JSONDataOut
}
ta(passive) = infinity
ta(active) = ProcessingTime
```
- Publisher
```
S = {}
X = {DataIn}
Y = {}
δint () = {}
δext (CSVDataIn, passive) {
    client.publish(“<UID>/fused”, (const char*) DataIn);
}
λ (active) {}
ta(passive) = infinity
ta(active) = ProcessingTime
```