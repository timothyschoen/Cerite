Box,0,ground,833,466
Box,1,output 0.3,31,503
Box,2,input ./media/Solo.wav 0.2,204,188
Box,3,capacitor 2.2e-6,189,236.5625
Box,4,resistor 100e3,221,330.5625
Box,5,voltage 9,35,68
Box,6,resistor 330,30,318.5625
Box,7,resistor 33e3,42,266.5625
Box,8,resistor 8200,43,359.5625
Box,9,capacitor 10e-9,30,415.5625
Box,10,bjt 0,133,325.5625
Box,11,bjt 0,148,283.5625
Box,12,resistor 250e3,30,446
Box,13,resistor 250e3,133,445
Box,14,resistor 500,233,372
Box,15,resistor 500,243,420
Box,16,capacitor 20e-6,426,413
Connection,1,12,0,1,analog
Connection,0,1,0,1,analog
Connection,12,13,1,0,analog
Connection,0,13,0,1,analog
Connection,12,9,0,1,analog
Connection,9,10,0,1,analog
Connection,9,6,0,1,analog
Connection,6,5,0,0,analog
Connection,6,8,1,0,analog
Connection,10,8,1,1,analog
Connection,10,11,0,1,analog
Connection,5,7,0,0,analog
Connection,11,7,1,1,analog
Connection,0,11,0,2,analog
Connection,2,3,0,0,analog
Connection,3,4,1,0,analog
Connection,11,3,0,1,analog
Connection,10,4,2,1,analog
Connection,4,14,1,0,analog
Connection,14,15,1,0,analog
Connection,0,15,0,1,analog
Connection,14,16,1,0,analog
Connection,0,16,0,1,analog
Connection,2,0,1,0,analog
Connection,5,0,1,0,analog
