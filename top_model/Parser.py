file1 = open('SensorFusion_Cadmium_output.txt', 'r')
file2 = open('Output.csv', 'w')
Lines = file1.readlines()
 
count = 0
oldTimeStamp = ''
file2.write("TIMESTAMP,T1, T2, T3, T4, T5, T6, T7, T8, FusedT\n")
for line in Lines:
    if line:
        string1 = line.split('{')

        if len(string1) == 1:
            if len(string1[0])==13:
                TimeStamp = string1[0].strip()
                if oldTimeStamp != TimeStamp:
                    file2.write("\n")
                    file2.write(TimeStamp)
                    file2.write(",")
                oldTimeStamp = TimeStamp
        else: 
            string2 = string1[1]
            string2 = string2.split('}')
            values = string2[0].strip()
            if values:
                file2.write(values)
                file2.write(",")
file1.close()
file2.close()
