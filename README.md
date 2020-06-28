# DCF77_NoInterrupt
This Arduino code decodes the DCF77-signal without the use of interrupts.<br><br>
The DCF77 signal is broadcasted at 77kHz and can be received with a simple cheap receiver that cost €10.<br>
A one second signal contains a bits.<br>
If the one second signal is high for 100 milliseconds it is a zero and 200 milli seconds represents a 1.<br>
After one minute the date and time can be calculated from these sixty bits
Interference of the signal makes it harder or impossible to determine if the signal contained a 0 or a 1.<br>
Several libraries are available. The DCF77-library supported by Arduino uses interrupts.<br>
Interrupts (freezes) the program flow, does some coding and return the program to the point where it was interrupted.<br>
An interrupt subroutine can be started when signals goes from low to high, high to low or changes.<br>
When there is interference in the signal the interrupt is entered more then two times.<br>
The subroutine has to deal with this "spikes" and filter these out.<br>
Interference of the signal can be caused by the weather, the sun, magnetrons, LEDs, displays, and other chips.<br> 
But also a bad power supply and even a bad power suply in the same group of the power line!<br><br>
There are receivers that receive other time transmitters on other wavelengths and some combine a few together.<br> 
Time and date can be read by serial communication from these modules.<br>
I wanted to make a clock that displays the DCF77-signal. https://github.com/ednieuw/DCFtiny-clock<br>
This little program presented does not use an interupt routine but another approach. It loops and reads the signal continuously counting the positive and negative signals.
In this loop it can read 50,000 signal per second with an Arduino running at 16Mhz clock speed.<br>
It simply divides the positive reads by the total reads and calculates percentages.<br>
Signal lengths between 5% and 15% is a 0 and between 15% and 30% is a 1 bit.<br>
The % is the length of the signal 10% = 100 msec and 20% is 200 msec.
I tried to optimize the program with several filters like the pulse must be longer than 50 msec or read every 10 msec 100 times the signal. These 'tricks' never resulted in more than 80% good time decoding.<br>
This was probably caused by the drift of the internal Arduino clock, that is used in the millisec() function, of around 1 second every 5 minutes.<br>
The solution was to wait for a signal drop after 995 msec. This resulted in a 100% score in decoding the time from the signal when reception was optimal. The Arduino DCF77-library gives comparable results.<br>
When the signal is not optimal combining the DCF77-library and this routine improves the 'good decoding score' by 20-40%.
<br>
A disadvantage of this method that you can not use delay() and routines that consumes a lot of time.<br>
<br>
Techniques like below can improve the loop speed dramatically.<br> 
<br>
 if( millis() - msTick == 50 )  digitalWrite(SecondsPin,LOW);   // Turn OFF the second on pin secondsPin<br>
 if( millis() - msTick > 999)                                   // Flash the onboard Pin 13 Led so we know something is happening<br>
   { <br>   
    msTick = millis();<br>
    digitalWrite(SecondsPin,HIGH);                               // Turn ON the second on pin 13<br>
    if (Lastminue != Dminute) EverMinuteCheck();<br>
   }<br>
}<br>
void EveryMinuteCheck(void)<br>
{<br>
}<br>




 
