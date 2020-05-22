# DCF77_NoInterrupt
This Arduino code decodes the DCF77-signal without the use of interrupts.<br>
The DCF77 signal is broadcasted at 77kHz and can be received with a simple cheap receiver that cost €10.<br>
The signal contains bits and in one minute the bits are coded in a signal.<br>
If the signal is high for 100 milliseconds it is a zero and 200 milli seconds represents a 1.<br>
Interference of the signal makes it harder or impossible to receive the bits without an error.<br>
Several libraries are available. The DCF77-library supported by Arduino uses interrupts.<br>
Interrupts (freezes) the program flow, does some coding and return the program to the point where it was interrupted.
An interrupt subroutine can be started when signals goes from low to high, high to low or changes.
When there is a lot of inteference in the signal the interrupt is entered very often.<br>
The subroutine has to deal with this "spikes" and filter these out.<br>
Interference of the signal can be caused by the weather, the sun, magnetrons, LEDs, displays, and other chips. 
But also a bad power supply and even a bad power suply in the same group of the power line!<br>
There are receivers that can receive other time transmitters on other wavelengths and some combine a few together. With serial communication the time can be read from these modules.<br>
I wanted to make a clock that display the DCF77-signal.<br>
This little program uses another approach. It loops and reads the signal continuously counting the positive and negative signals.
In this loop it can read 50,000 signal per second with an Arduino running at 16Mhz clock speed.
It simply divide the positive read by the total read and calculated percentages of 10% for a 0 and 20% for a 1 bit. Interference changes these percentages a little. The % is the length of the signal 105 = 100 msec and 20% is 200 msec.
I tried to optimize the program with several filters like the pulse must be longer than 50 msec or read every 10 msec 100 times the signal. These 'tricks' never resulted in more than 80% good time decoding.<br>
The trick to wait for a signal drop after 995 msec resulted in a 100% score in decoding the time from the signal when reception was optimal. The Arduino DCF77-library gives comparable results.<br>
When the signal is not optimal combining the DCF77-library and this routine improves the 'good decoding score' by 20-40%.





 
