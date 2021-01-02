Box,0,ground,409,622
Box,1,output 0.3,177,632
Box,2,input ./sample-44k.wav 0.2,233,164
Box,3,opamp,153,462
Box,4,resistor 2300,255,559
Box,5,resistor 1000,260,422
Box,6,capacitor 0.000001,10,340
Box,7,capacitor 0.000001,428,372
Box,8,resistor 1000,158,341
Box,9,resistor 1000,157,258
Inlet,0,174,570
Inlet,1,400,414
Connection,0,1,0,1,analog
Connection,3,5,1,0,analog
Connection,0,7,0,1,analog
Connection,3,8,0,1,analog
Connection,7,8,0,1,analog
Connection,2,9,0,0,analog
Connection,8,9,0,1,analog
Connection,6,9,0,1,analog
Connection,i,3,0,2,analog
Connection,i,1,0,0,analog
Connection,i,6,0,1,analog
Connection,i,4,0,1,analog
Connection,4,5,0,0,analog
Connection,i,5,1,1,analog
Connection,i,7,1,1,analog
Connection,i,2,1,1,analog
