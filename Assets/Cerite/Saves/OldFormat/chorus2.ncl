Box,0,ground,251,296
Box,1,capacitor 4.7e-8,136,221
Box,2,resistor 4700,74,173
Box,3,input ./media/sample-44k.wav 0.2,72,120
Box,4,delay-,521,417
Box,5,adc,75,291
Box,6,sig- 20000,553,333
Box,7,delay-,208,477
Box,8,sig- 8000,207,389
Box,9,output- 0.3,291,579
Box,10,*- 0.4,357,416
Box,11,*- 0.4,352,495
Box,12,cycle- 2,383,238
Box,13,*- 80,382,296
Box,14,!-- 80,485,335
Box,15,comment Lowpass Filter,120,81
Box,16,comment Feedback,326,456
Box,17,*- 0.9,783,347
Box,18,sig- 500,457,234
Box,19,sig- 3,388,176
Box,20,sig- 0.3,779,261
Connection,1,2,0,1,analog
Connection,0,1,0,1,analog
Connection,2,3,0,0,analog
Connection,0,3,0,1,analog
Connection,5,1,1,1,analog
Connection,5,2,0,1,analog
Connection,4,5,0,2,digital
Connection,4,6,1,0,digital
Connection,7,5,0,2,digital
Connection,7,8,1,0,digital
Connection,10,7,0,2,digital
Connection,4,10,0,2,digital
Connection,7,11,0,2,digital
Connection,11,4,0,2,digital
Connection,13,12,0,1,digital
Connection,7,13,1,2,digital
Connection,14,13,0,2,digital
Connection,4,14,1,2,digital
Connection,7,9,2,0,digital
Connection,9,4,1,2,digital
Connection,10,17,1,2,digital
Connection,11,17,1,2,digital
Connection,13,18,1,0,digital
Connection,12,19,0,0,digital
Connection,17,20,0,0,digital
