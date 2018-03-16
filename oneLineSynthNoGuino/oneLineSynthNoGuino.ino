// algorithminator
// author: jakeybob
// date created | modified : 1/11/11 | 4/11/11
// hardware: arduino Uno
//
// This code outputs algorithmic audio. There are 8 different algorithms to choose from,
// selected by the user via three toggle switches/LEDs in a binary display setup. Additionally,
// up to three parameters in each algorithm can be adjusted on the fly by the user, using
// three potentiometers. (rocker switches would probably be better, but I had pots ;)
//
// This program was inspired by this blogpost and related videos etc: 
// http://countercomplex.blogspot.com/2011/10/algorithmic-symphonies-from-one-line-of.html
//
// Code that has been reused is enclosed by "// <code>" tags, with a link or credit to the source.
//
// Hardware setup: 
// 3 toggle switches connected to pins 2, 7, 8; switching in either 5V or 10k-to-ground resistors
//     > these switches are used to choose the audio preset that's currently playing
// 3 LEDs connected to pins 3, 9, 11; with a 100R resistor in series to ground (I'm using blue-green
// LEDs http://uk.farnell.com/jsp/search/productdetail.jsp?SKU=1581173)
//    > these are used to show the current audio preset. Not strictly necessary as the position of the
//      toggles shows the same info
// 3 pots (10k) with wipers connected to pins 1, 2, 3 
//    > these are used to adjust values in the algorithms up and down
// 1 volume pot (10k) connected to audio output (pin 10)
//    > I've connected a 330R resistor from the input (i.e. pin 10) to the wiper to mess with  the volume
//      taper a bit. Easier to just use a smaller pot, but I only had 10k in this size.
// 1 output toggle switch connected to the volume pot output
// 1 stero socekt (3.5mm) connected to output toggle switch
// 1 micro dynamic speaker connected to output toggle switch (http://uk.farnell.com/jsp/search/productdetail.jsp?SKU=1502729)

const int buttonPin1 = 2;
const int buttonPin2 = 7;
const int buttonPin4 = 8;

const int ledPin1 = 3;
const int ledPin2 = 9;
const int ledPin4 = 11;

const int paramPin1 = 1;
const int paramPin2 = 2;
const int paramPin3 = 3;


const int brightness = 7; // LED brightness = quite low to avoid eye strain when looking at device

int ones; int twos; int fours; int number;
int param1; int param2; int param3;

void setup(){
  
  // <code> from arduino forum user "jmknapp"
  // http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1208715493/11
  //
  // defines for setting and clearing register bits
  #ifndef cbi
  #define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
  #endif
  #ifndef sbi
  #define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
  #endif
  // set prescale to 16
  sbi(ADCSRA,ADPS2);
  cbi(ADCSRA,ADPS1);
  cbi(ADCSRA,ADPS0);
  //
  // </code>
  
  
  // setup pins
  pinMode(buttonPin1, INPUT);
  pinMode(buttonPin2, INPUT);
  pinMode(buttonPin4, INPUT);
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin4, OUTPUT);
  
  // <code> from Arduino forum user "stimmer"
  // http://arduino.cc/forum/index.php?topic=74123.0
  TCCR1B = (TCCR1B & 0xf8) | 1;
  analogWrite(10,1);
  // </code>
  
}


void loop(){

  long v;
  for(long t=0;;t++)
  {
    // keep the speed up by only polling buttons / pots once every 500 loops
    if (t % 500 == 0){   
    number = getNumber();
    getParams();
    }

      // play appropriate audio depending on user selected preset
      switch (number){
        
        case 0:
        putb(
        (t + param1) * ((t>>12|t>>(8 + param2))& (63 + param3) & t>>4 )        // by viznut
        );
        break;
        
        case 1:
        putb(
        (t|(t>>(9 + param1)|t>>(7 + param2)))*t&(t>>(11 + param3)|t>>9)        // by red
        );
        break;
        
        case 2:
        putb(
        ((-t&4095)*(255&t*(t&t>>13))>>12)+(127&t*(234&t>>(8 + param1)&t>>3)>>((3 + param2)&t>>(14 + param3)))   // by tejeez
        );
        break;
        
        case 3:
        putb(
        t*(t>>(11 + param1)&t>>(8 + param2)&123&t>>(3 + param3))                                                // by tejeez
        );
        break;
        
        case 4:
        putb(
        t*((t>>(9 + param1)|t>>(13 + param2))&25&t>>(6 + param3))                                               // by visy
        );
        break;
        
        case 5:
        putb(
        ((t*(t>>(8 + param1)|t>>9)&46&t>>(8 + param2)))^(t&t>>(13 + param3)|t>>6)                               // by xpansive
        );
        break;
        
        case 6:
        putb(
        ((t&4096)?((t*(t^t%255)|(t>>(4 + param1)))>>1):(t>>(3 + param2))|((t&8192)?t<<(2 + param3):t))          // by skurk (raer's version)
        );
        break;
        
        case 7:
        putb(
        (t*((4 + param1)|7&t>>13)>>((~t>>11)&1)&128) + ((t)*(t>>(11 + param2)&t>>13)*((~t>>(9 + param3))&3)&127) // by stimmer
        );
        break;
        
        default:
        // play crappy error buzz if "number" input not integer between 0 and 7
        putb(30*t);
      
      }

  }
  
}



int getNumber(){
  
  ones = digitalRead(buttonPin1);
  twos = digitalRead(buttonPin2);
  fours = digitalRead(buttonPin4);
  
  // check ones toggle
  if (ones == HIGH){
    analogWrite(ledPin1, brightness);
  }
  else{
   digitalWrite(ledPin1, LOW); 
  }
  
  // check twos toggle
  if (twos == HIGH){
    analogWrite(ledPin2, brightness);
  }
  else{
   digitalWrite(ledPin2, LOW); 
  }
  
  // check fours toggle
  if (fours == HIGH){
    analogWrite(ledPin4, brightness);
  }
  else{
   digitalWrite(ledPin4, LOW); 
  } 
  
  number = ones + 2*twos + 4*fours;
}



void getParams(){
  // read in pot values and resolve to 5 integer values around zero
  param1 = (analogRead(paramPin1)/205) - 2; 
  param2 = (analogRead(paramPin2)/205) - 2; 
  param3 = (analogRead(paramPin3)/205) - 2; 
}



// <code> from Arduino forum user "stimmer"
// http://arduino.cc/forum/index.php?topic=74123.0
void putb(byte b)
{
  static long m;
  long t;
  while((t=micros())-m < 125);
  m=t;
  
  OCR1B=b;
}
// </code>
