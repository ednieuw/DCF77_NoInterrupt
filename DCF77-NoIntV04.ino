// =============================================================================================================================
/* 
The DCF77 module receives time signals from the German longwave DCF-77 transmitter.
The signals contains pulses of 100 or 200 msecs per second that represents respectively 0 and 1.
This routine exract the time and date from this binary code.
V01 Extracted from DCF_HC12TransmitterV48
V02 DCFtiny reports wrong time/dates. changed  if(DCF_signal>60 && abs((((Dyear*1  in if(DCFlocked && abs((((Dyear*12+Dmonth)*
V03 Updated coding in subroutine UpdateDCFclock(); case 0 
V04 Added in line 209 ||Dday>31

To be done  ---> Let the program choose if the DCF pulse is inverted or not inverted. See line 109 and 116

Ed Nieuwenhuys October 2020
 */
// ===============================================================================================================================

//--------------------------------------------
// ARDUINO Includes defines and initialysations
//--------------------------------------------
#include <RTClib.h>                       // Needed for RTC_Millis
#define MOD_DS3231                      // uncomment is a Ds3231 is connected to A4 and A5 pins (SDA & SCL)
byte DCF_PIN      = 2;                    // DCFPulse on interrupt pin
byte SecondsPin   = 9;                    // Connect a LED to ths port
byte DCF_LED_Pin  = 13;                   // Show DCF-signal on PCB LED pin 13

//--------------------------------------------
// CLOCK
//--------------------------------------------
        #ifdef MOD_DS3231
RTC_DS3231 Clock;    //RTC_DS1307 RTC; 
        #else if
RTC_Millis Clock;                          // If no RTC clock connected run on processor time
        #endif 
byte     DCF_signal         = 40;         // Is a proper time received? (0 - 100)
byte     CSummertime        = 40;         // If >50 times CSummertime is received Summertime = true
byte     CWintertime        = 40;         // If >50 times CWintertime is received Wintertime = true
//--------------------------------------------
// DCF77 DCFtiny MODULE
//--------------------------------------------

bool     DCFlocked          = false;      // Time received from DCF77
bool     PrintDebugInfo     = true;      // For showing debug info for DCFTINY
byte     StartOfEncodedTime =  0;         // Should be always one. Start if time signal
int      Dsecond, Dminute, Dhour;         // Variables of DCF decodings
int      Dday, Dmonth, Dyear, Dwday;      // Variables of DCF decodings 
int      Lastday;                         // Used to unlock DCFlock
uint32_t DCFmsTick          =  0;         // The number of millisecond ticks since we last incremented the second counter
uint32_t SumSecondSignal    =  0;         // Sum of digital signals ( 0 or 1) in 1 second
uint32_t SumSignalCounts    =  0;         // Noof of counted signals
uint32_t SignalFaults       =  0;         // Counter for SignalFaults per hour  
uint32_t ValidTimes         =  0;         // Counter of total recorded valid times
uint32_t MinutesSinceStart  =  0;         // Counter of recorded valid and not valid times
//byte     SignalBin[10];                 // For debugging to see in which time interval reception takes place

//----------------------------------------
// Common
//----------------------------------------
char sptext[80];                          // For common print use in sprintf(sptext, ... )     
//--------------------------------------------
// ARDUINO Loop
//--------------------------------------------
void loop(void)
{ 
 EverySecondCheck();                      // Place as much of your progrtam in this subroutine
 DCF77Check();                            // Should be checked as much as possible.
                                          // Do not use delays in your program! 
                                          // DCF signal readings can be up to 50000 times/sec without EverySecondCheck() 
} 
//--------------------------------------------
// CLOCK Update routine done every second
//--------------------------------------------
void EverySecondCheck(void)
{
static  uint32_t msTick;                                                       // The number of millisecond ticks since we last incremented the second counter

 if( millis() - msTick == 50 )     digitalWrite(SecondsPin,LOW);               // Turn OFF the second on pin secondsPin
 if( millis() - msTick > 999)                                                  // Flash the onboard Pin 13 Led so we know something is happening
   {    
    msTick = millis();
    digitalWrite(SecondsPin,HIGH);                                             // Turn ON the second on pin 13
    if (Lastday != Dday) EveryDayCheck();
   }
}
void EveryDayCheck(void)
{
 DCFlocked = false;                                                            // allow the DCF to set time once a day
 Lastday = Dday;
}
//--------------------------------------------
// ARDUINO Setup
//--------------------------------------------
void setup()
{                                        
 Serial.begin(9600);                                                           // Setup the serial port to 9600 baud  
 Clock.begin(); //DateTime(__DATE__, __TIME__));                                     // Start the RTC-module  
 while (!Serial);                                                              // Wait for serial port to connect. Needed for native USB port only
 Serial.println("\nSerial started \nDCF77-NoIntV03"); 
                                                                               // Define your pins                      
 pinMode(DCF_LED_Pin, OUTPUT);                                                 // For showing DCF-pulse or other purposes
 pinMode(DCF_PIN,     INPUT_PULLUP);
 DCFmsTick = millis();                                                         // Start of DCF 1 second loop
 } 

//--------------------------------------------
// CLOCK check for DCF input
//--------------------------------------------
void DCF77Check(void)
{
 digitalWrite(DCF_LED_Pin, !digitalRead(DCF_PIN));                             // This line can be removed is a LED signal is not needed
                                                                               // ********** Choose one of the following two lines
 SumSecondSignal += !digitalRead(DCF_PIN);                                     // Invert 0 to 1 and 1 to 0 // Counter of the positive signals
//SumSecondSignal += digitalRead(DCF_PIN);                                     // Do not invert the signal // Counter of the negative signals

 SumSignalCounts++;                                                            // Counter the measured loops per second
 if((millis() - DCFmsTick) >995)                                               // Compute every second the received DCF-bit to a time 
   {                                                                           // Wait after 995 msecs until the signal drops

    while (!digitalRead(DCF_PIN))                                              // ********* Use one of the two folowing line
// or   while (digitalRead(DCF_PIN))                                           // Do not invert the signal 
       {
        SumSignalCounts++;
        SumSecondSignal++;
       }
    DCFmsTick = millis();                                                      // Set one second counter
//    SumSignalCounts *= 1.05;                                                 // Possible orrection factor to correct pulses to an average of 100 and 200 msecs (10% and 20%)
    if(byte OKstatus = UpdateDCFclock())                                       // If after 60 sec a valid time is calculated, sent it to the HC-12 module
      {
      DateTime now = Clock.now();                                              // Get the recent time
      if(OKstatus == 2)                                                        // If time flag was OK and date flag was NOK
        {
        sprintf(sptext,"       TIME OK --> %0.2d:%0.2d  ",Dhour, Dminute); Serial.println(sptext);           
     //  Clock.adjust(DateTime(now.year(), now.month(), now.day(), Dhour, Dminute, 0));  // Your choice
        }
      if(OKstatus == 1)                                                        // If time flag was OK and date flag also OK  (OKstatus == 1)
        {
         sprintf(sptext,"TIME & DATE OK -1> %0.2d:%0.2d %0.2d-%0.2d-%0.4d/%0.1d ",Dhour, Dminute, Dday, Dmonth, 2000+Dyear, Dwday );
         Serial.println(sptext);
         DateTime DCFtijd (Dyear, Dmonth, Dday, Dhour, Dminute, 0);
         int TimeDiff = (int)(abs(now.unixtime() - DCFtijd.unixtime()));
         if(TimeDiff > 2)                                                      // > in Seconds. If difference between DCF and RTC time update the RTC
           {   
            sprintf(sptext,"Updating %ds different! Clock:%0.2d:%0.2d:%0.2d DCF:%0.2d:%0.2d:%0.2d ",
                               TimeDiff,now.hour(), now.minute(),now.second(),Dhour,Dminute,0); 
            Serial.println(sptext); 
            Clock.adjust(DateTime(Dyear, Dmonth, Dday, Dhour, Dminute, 0));
//            setTime(Dhour, Dminute, 0, Dday, Dmonth, Dyear );
            DCFlocked = true;                                                  // DCF time stored (for one hour before unlock)
            Print_RTCtime();
           }
         }
       ValidTimes++;                                                           // Count the valid date/times
       }         
    }
} 

//--------------------------------------------
// DCFtiny Make the time from the received DCF-bits
//--------------------------------------------
byte UpdateDCFclock(void)
{
 byte        TimeOK     , Receivebit     , Receivebitinfo;                     // Return flag is proper time is computed and received bit value
 byte        BMinutemark, BReserveantenna, BOnehourchange;
 byte        BSummertime, BWintertime    , BLeapsecond;
 bool        TimeSignaldoubtfull = false;                                      // If a bit length signal is not between 5% and 30%
 static byte Bitpos,  Paritybit;                                               // Postion of the received second signal in the outer ring  
 static byte MinOK=2, HourOK=2, YearOK=2;                                      // 2 means not set. static because the content must be remembered
 static int  TimeMinutes,     LastTimeMinutes;
 static int  TimeMinutesDiff, DateDaysDiff;
 static uint32_t  DateDays,   LastDateDays;

 if (Bitpos >= 60) 
     {
      Bitpos = 0;
      MinOK = HourOK = YearOK = 2;                                             // 2 means not set. static because the content must be remembered
      MinutesSinceStart++;
      } 
 int msec  = (int)(1000 * SumSecondSignal / SumSignalCounts);                  // This are roughly the milliseconds times 10 of the signal length
 switch(msec)
  {
   case   0 ...  10: if(SumSignalCounts > 500)                                 // Check if there were enough signals
                       {Bitpos = 59; Receivebitinfo = 2; Receivebit = 0; }     // If enough signals and one second no signal found. This is position zero
                              else { Receivebitinfo = 9; Receivebit = 0; }     // If not it is a bad signal probably a zero bit
                                                                        break; // If signal is less than 0.05 sec long than it is the start of minute signal
   case  11 ...  50: Receivebitinfo = 8; Receivebit = 0;                break; // In all other cases it is an error
   case  51 ... 160: Receivebitinfo = 0; Receivebit = 0;                break; // If signal is 0.1 sec long than it is a 0 
   case 161 ... 320: Receivebitinfo = 1; Receivebit = 1;                break; // If signal is 0.2 sec long than it is a 1 
   default:          Receivebitinfo = 9; Receivebit = 1;                break; // In all other cases it is an error probably a one bit
  } 
 if(Receivebitinfo > 2)  SignalFaults++;                                       // Count the number of faults per hour
 TimeOK = 0;                                                                   // Set Time return code to false
 DateTime now = Clock.now();
 switch (Bitpos)                                                               // Decode the 59 bits to a time and date.
  {                                                                            // It is not binary but "Binary-coded decimal". 
   case 0: BMinutemark = Receivebit;                                           // Blocks are checked with an even parity bit. 
           TimeMinutes    = (Dhour*60 + Dminute);
           DateDays       = (uint32_t)(((Dyear*12 + Dmonth)*31) + Dday);
           TimeMinutesDiff =      abs(TimeMinutes - LastTimeMinutes);
           DateDaysDiff    = (int)abs(DateDays - LastDateDays);
           if(PrintDebugInfo)
             { 
               sprintf(sptext,"Timediff %d min DCF:%0.2d:%0.2d:%0.2d TotMin:%d Last TotMin:%d ",
                     TimeMinutesDiff, Dhour, Dminute, 0, TimeMinutes, LastTimeMinutes);  
               Serial.println(sptext);
              }  
            if((MinOK && HourOK && YearOK) && 
              (TimeMinutesDiff < 2) && (DateDaysDiff < 2 ) )        TimeOK = 1;  // All OK          
            if(MinOK && HourOK && !YearOK && (TimeMinutesDiff<2))   TimeOK = 2;  // If a date difference time remains usable
            if(TimeSignaldoubtfull)                                 TimeOK = 5;  // If a signal lenght was too short or long this last minute time is not OK
            if(Dmonth==0||Dmonth>12||Dday>31||Dhour>23||Dminute>59) TimeOK = 0;  // If time is definitely wrong return Time is not OK
                                                                        break;   // Bit must always be 0
   case 1: TimeSignaldoubtfull = 0;    
           LastTimeMinutes = TimeMinutes; 
           LastDateDays    = DateDays;                                      
            Dminute = Dhour = Dday = Dwday = Dmonth = Dyear = 0;  
   case 2 ... 14:                                                       break;   // Reserved for weather info we will not use
   case 15: BReserveantenna = Receivebit;                               break;   // 1 if reserve antenna is in use
   case 16: BOnehourchange  = Receivebit;                               break;   // Has value of 1 one hour before change summer/winter time
   case 17: BSummertime = Receivebit;
             if(Receivebit) CSummertime++; 
             else           CSummertime--;
             CSummertime = constrain(CSummertime,5,60);                 break;                               // 1 if summer time 
   case 18: BWintertime = Receivebit; 
             if(Receivebit) CWintertime++;
             else           CWintertime--;
             CWintertime = constrain(CWintertime,5,60);                 break;   // 1 if winter time
   case 19: BLeapsecond = Receivebit;                                   break;   // Announcement of leap second in time setting is coming up
   case 20: StartOfEncodedTime=Receivebit; Paritybit = 0;               break;   // Bit must always be 1 
   case 21: if(Receivebit) {Dminute  = 1;  Paritybit = 1 - Paritybit;}  break;
   case 22: if(Receivebit) {Dminute += 2 ; Paritybit = 1 - Paritybit;}  break;
   case 23: if(Receivebit) {Dminute += 4 ; Paritybit = 1 - Paritybit;}  break;
   case 24: if(Receivebit) {Dminute += 8 ; Paritybit = 1 - Paritybit;}  break;
   case 25: if(Receivebit) {Dminute += 10; Paritybit = 1 - Paritybit;}  break;
   case 26: if(Receivebit) {Dminute += 20; Paritybit = 1 - Paritybit;}  break;
   case 27: if(Receivebit) {Dminute += 40; Paritybit = 1 - Paritybit;}  break;
   case 28: MinOK = (Receivebit==Paritybit);  Paritybit = 0;            break;   // The minute parity is OK or NOK               
   case 29: if(Receivebit) {Dhour   =  1; Paritybit = 1 - Paritybit;}   break;
   case 30: if(Receivebit) {Dhour  +=  2; Paritybit = 1 - Paritybit;}   break;
   case 31: if(Receivebit) {Dhour  +=  4; Paritybit = 1 - Paritybit;}   break;
   case 32: if(Receivebit) {Dhour  +=  8; Paritybit = 1 - Paritybit;}   break;
   case 33: if(Receivebit) {Dhour  += 10; Paritybit = 1 - Paritybit;}   break;
   case 34: if(Receivebit) {Dhour  += 20; Paritybit = 1 - Paritybit;}   break;
   case 35: HourOK = (Receivebit==Paritybit); Paritybit = 0;            break; // The hour parity is OK or NOK                       
   case 36: if(Receivebit) {Dday    =  1; Paritybit = 1 - Paritybit;}   break;
   case 37: if(Receivebit) {Dday   +=  2; Paritybit = 1 - Paritybit;}   break;
   case 38: if(Receivebit) {Dday   +=  4; Paritybit = 1 - Paritybit;}   break;
   case 39: if(Receivebit) {Dday   +=  8; Paritybit = 1 - Paritybit;}   break;
   case 40: if(Receivebit) {Dday   += 10; Paritybit = 1 - Paritybit;}   break;
   case 41: if(Receivebit) {Dday   += 20; Paritybit = 1 - Paritybit;}   break;
   case 42: if(Receivebit) {Dwday   =  1; Paritybit = 1 - Paritybit;}   break;
   case 43: if(Receivebit) {Dwday  +=  2; Paritybit = 1 - Paritybit;}   break;
   case 44: if(Receivebit) {Dwday  +=  4; Paritybit = 1 - Paritybit;}   break;
   case 45: if(Receivebit) {Dmonth  =  1; Paritybit = 1 - Paritybit;}   break;
   case 46: if(Receivebit) {Dmonth +=  2; Paritybit = 1 - Paritybit;}   break;
   case 47: if(Receivebit) {Dmonth +=  4; Paritybit = 1 - Paritybit;}   break;
   case 48: if(Receivebit) {Dmonth +=  8; Paritybit = 1 - Paritybit;}   break;
   case 49: if(Receivebit) {Dmonth += 10; Paritybit = 1 - Paritybit;}   break;
   case 50: if(Receivebit) {Dyear   =  1; Paritybit = 1 - Paritybit;}   break;
   case 51: if(Receivebit) {Dyear  +=  2; Paritybit = 1 - Paritybit;}   break;
   case 52: if(Receivebit) {Dyear  +=  4; Paritybit = 1 - Paritybit;}   break;
   case 53: if(Receivebit) {Dyear  +=  8; Paritybit = 1 - Paritybit;}   break;
   case 54: if(Receivebit) {Dyear  += 10; Paritybit = 1 - Paritybit;}   break;
   case 55: if(Receivebit) {Dyear  += 20; Paritybit = 1 - Paritybit;}   break;
   case 56: if(Receivebit) {Dyear  += 40; Paritybit = 1 - Paritybit;}   break;
   case 57: if(Receivebit) {Dyear  += 80; Paritybit = 1 - Paritybit;}   break;
   case 58: YearOK = (Receivebit==Paritybit);  Paritybit = 0;           break; // The year month day parity is OK or NOK               
   case 59: //Serial.println("silence");                               
                                                                        break;
    default:  Serial.println(F("Default in BitPos loop. Impossible"));  break;
   }
  if((Bitpos>19 && Bitpos<36) && Receivebitinfo>2) TimeSignaldoubtfull = true; // If a date time bit is received and the received bit is not 0 or 1 then time becomes doubtfull
  if(PrintDebugInfo)
    {
     sprintf(sptext,"@%s%s%s", (MinOK<2?(MinOK?"M":"m"):"."),(HourOK<2?(HourOK?"H":"h"):"."),(YearOK<2?(YearOK?"Y":"y"):"."));  Serial.print(sptext);
     sprintf(sptext,"%6ld %5ld %2ld%% (%d) %0.2d:%0.2d:%0.2d %0.2d-%0.2d-%0.4d/%0.1d F%d",
                     SumSecondSignal, SumSignalCounts, 100*SumSecondSignal/SumSignalCounts, Receivebitinfo, 
                     Dhour, Dminute, Bitpos, Dday, Dmonth, 2000+Dyear, Dwday, SignalFaults); Serial.print(sptext); 
     sprintf(sptext," Valid:%ld MinTot:%ld OK:%d Lock:%s ",ValidTimes, MinutesSinceStart,TimeOK,DCFlocked?"Y":"N"); Serial.println(sptext); 
    }     
// sprintf(sptext,"\%d%d%d%d%d%d%d%d%d%d/", SignalBin[0],SignalBin[1],SignalBin[2],SignalBin[3],SignalBin[4], SignalBin[5],SignalBin[6],SignalBin[7],SignalBin[8],SignalBin[9]);  Serial.println(sptext);
 SumSecondSignal = SumSignalCounts = 0;
// for (int x=0; x<10; x++) SignalBin[x] = 0;
 Dsecond = Bitpos;
 Bitpos++; 
return(TimeOK);
}        
 
//--------------------------------------------
// CLOCK utility function prints time to serial
//--------------------------------------------
void Print_RTCtime(void)
{
 DateTime now = Clock.now();
 sprintf(sptext,"RTC %0.2d:%0.2d:%0.2d %0.2d-%0.2d-%0.4d",now.hour(),now.minute(),now.second(),now.day(),now.month(),now.year());
 Serial.println(sptext);
}
