Box,0,ground,437,578
Box,1,output 0.3,127,582
Box,2,input ./Media/sample-44k.wav 0.2,91,147
Box,3,opamp,128,474
Box,4,resistor 2300,212,475
Box,5,resistor 1000,279,421
Box,6,capacitor 0.000001,215,373
Box,7,capacitor 0.000001,396,326
Box,8,varres 1000,187,213
Box,9,varres 1000,116,271
Box,10,cycle- 2,436,136
Box,11,+- 1,442,172
Box,12,*- 800,554,222
Connection,0,2,0,1,analog
Connection,1,3,0,2,analog
Connection,0,1,0,1,analog
Connection,4,3,0,1,analog
Connection,3,4,2,1,analog
Connection,5,3,0,1,analog
Connection,0,5,0,1,analog
Connection,0,7,0,1,analog
Connection,3,6,2,1,analog
Connection,8,2,0,0,analog
Connection,9,8,0,2,analog
Connection,6,8,0,2,analog
Connection,9,7,2,0,analog
Connection,9,3,2,0,analog
Connection,11,10,0,1,digital
Connection,8,12,1,2,digital
Connection,9,12,1,2,digital
Connection,12,11,0,2,digital
