Box,0,ground,608,643,0
Box,1,output 0.3,369,718,0
    Parameter,Amplitude,0.3
Box,2,input ./sample-44k.wav 0.2,427,211,0
    Parameter,Amplitude,0.2
    Parameter,Path,./sample-44k.wav
Box,3,opamp,351,480,0
    Parameter,G,2000
    Parameter,UMax,0.05
Box,4,resistor 2300,465,609,0
    Parameter,Resistance,2300
Box,5,resistor 1000,465,466,1
    Parameter,Resistance,1000
Box,6,capacitor 0.000001,201,475,0
    Parameter,Capacitance,0.000001
Box,7,capacitor 0.000001,431,413,1
    Parameter,Capacitance,0.000001
Box,8,varres 1000,363,357,0
Box,9,varres 1000,363,278,0
Box,10,saw- 3,673,179,0
Box,11,*- 600,677,252,0
Box,12,+- 1,672,212,0
Box,13,slider-,781,138,0
    Parameter,slider,2.107901
Box,14,*- 200,767,222,0
Box,15,slider-,671,50,0
    Parameter,slider,4.921506
Box,16,*- 5,669,137,0
Inlet,0,372,675
Inlet,1,595,473
Inlet,2,192,292
Inlet,3,201,676
Connection,0,1,0,1,analog
Connection,3,5,1,0,analog
Connection,3,8,0,2,analog
Connection,7,8,0,2,analog
Connection,2,9,0,0,analog
Connection,8,9,0,2,analog
Connection,i,3,0,2,analog
Connection,i,1,0,0,analog
Connection,i,4,0,1,analog
Connection,4,5,0,0,analog
Connection,i,5,1,1,analog
Connection,i,7,1,1,analog
Connection,i,2,1,1,analog
Connection,i,0,1,0,analog
Connection,i,9,2,2,analog
Connection,i,6,2,0,analog
Connection,i,6,3,1,analog
Connection,i,i,0,3,analog
Connection,9,11,1,2,digital
Connection,8,11,1,2,digital
Connection,10,12,1,0,digital
Connection,11,12,0,2,digital
Connection,13,14,0,0,digital
Connection,11,14,1,2,digital
Connection,15,16,0,0,digital
Connection,10,16,0,2,digital
