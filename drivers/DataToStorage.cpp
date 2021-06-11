#include "mbed.h"
#include<iostream>


void StoreData(double sensorinputs[], int number_of_sensors, double fusedValue) {
            int i;
            time_t seconds = time(NULL);
            char buf[40];
            strftime(buf,40, "%Y-%m-%dT%H:%M:%SZ,", localtime(&seconds));
            printf("%s", buf);
            for(i=0;i<number_of_sensors;i++){
                cout<<sensorinputs[i]<<",";
            }
            cout<<fusedValue<<endl;
}