#include "mbed.h"
#include<iostream>


void StoreData(double sensorinputs[], double fusedValue) {
            int i;
            time_t seconds = time(NULL);
            char buf[40];
            strftime(buf,40, "%Y-%m-%dT%H:%M:%SZ,", localtime(&seconds));
            printf("%s", buf);
            for(i=0;i<(sizeof(sensorinputs)/sizeof(sensorinputs[0]));i++){
                cout<<sensorinputs[i]<<",";
            }
            cout<<fusedValue<<endl;
}