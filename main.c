#include <msp430.h> 
/*

int main(void) {

    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

    P1DIR&=~BIT3; //Set P1.3 as input
    P1REN|=BIT3; //Enable pullup/pulldown resistors for P1.3
    P1OUT|=BIT3; //Set P1.3 to have pull up resistors

    P1IE|=BIT3; //Enable interrupt on P1.3
    P1IES|=BIT3; //Set interrupt flag on the falling edge of logic level on P1.3

    __enable_interrupt(); //Enable maskable interrupts

    P1DIR|=BIT0; //Set P1.0 as output
    P1OUT&=~BIT0; //Initially turn off the LED


    __low_power_mode_0(); //Go to low power mode 0
     return 0;
}

#pragma vector=PORT1_VECTOR
__interrupt void P1_Function()
{
    int i; //Declare counter variable
    P1OUT^=BIT0; //Toggle the LED
    i=0; //Start a counter variable
    while(i<500) //Wait till the switch is not pressed continusouly for 500 loop cycles
    {
        if((P1IN&BIT3))
            i++;  //If the switch is not pressed, increment the counter variable
        else
            i=0; //If the switch is pressed, reset the counter variable
    }
    P1IFG&=~BIT3; // Reset interrupt flag
}
*/



#include "PCD8544.h"
#include <msp430.h>

#define IN BIT4  // Button connected to P1.3
#define LED BIT6     // Green LED connected to P1.6
#define LED2 BIT0     // Green LED connected to P1.6

#define BUZZER  BIT5
#define P2REDLED BIT4
#define P2GreenLED BIT3

unsigned long buttonPressedTime = 0;
int morsePos = 0;  // Morse code buffer position
int dotTime = 250; // Time in milliseconds for a dot
int dashTime = 500; // Time in milliseconds for a dash
int resetTime = 1000; // Time in milliseconds to reset
int OFCount = 0;
volatile int indexCounter = 0;
int check = 0;

const char *morseAlphabet[] = {
        ".-",   // A
        "-...", // B
        "-.-.", // C
        "-..",  // D
        ".",    // E
        "..-.", // F
        "--.",  // G
        "....", // H
        "..",   // I
        ".---", // J
        "-.-",  // K
        ".-..", // L
        "--",   // M
        "-.",   // N
        "---",  // O
        ".--.", // P
        "--.-", // Q
        ".-.",  // R
        "...",  // S
        "-",    // T
        "..-",  // U
        "...-", // V
        ".--",  // W
        "-..-", // X
        "-.--", // Y
        "--.."  // Z
};


void buzzerOn() {
    P2OUT |= BUZZER;

}

void buzzerOff() {
    P2OUT &= ~BUZZER;
}

void redLedOn()
{
    P2OUT |= P2REDLED;
}


void redLedOff()
{
    P2OUT &= ~ P2REDLED;
}


void greenLedOn()
{
    P2OUT |= P2GreenLED;
}


void greenLedOff()
{
    P2OUT &= ~P2GreenLED;
}


char findMorseIndex(char *morse) {
    volatile int i;

    for (i = 0; i < sizeof(morseAlphabet) / sizeof(morseAlphabet[0]); i++) {
        if (strcmp(morseAlphabet[i], morse) == 0) {
            // Convert index to corresponding uppercase letter (ASCII: 'A' = 65)
            return 'A' + i;
        }
    }

    return '?'; // Return a placeholder character if not found
}


void delayForOn(unsigned int n)
{
    unsigned int i;
    for (i = 0; i < n; i++)
    {
        __delay_cycles(1000); // 1ms delay at 1MHz clock speed
        if ((P1IN & IN ) != 0) break;
    }
}
void delayForOff(unsigned int n)
{
    unsigned int i;
    for (i = 0; i < n; i++)
    {
        __delay_cycles(1000); // 1ms delay at 1MHz clock speed
        if((P1IN & IN ) == 0)
            break;
    }
}

void main(void)
{

    volatile int indexCounter = 0;
    char adder;
    volatile char morseCode[6]; // Morse code buffer to store dots and dashes


    WDTCTL = WDTPW | WDTHOLD; // Watchdog Timer'ı durdur

    P1DIR &= ~IN; // P1.3'ü bir giriş olarak ayarla (Düğme için)
    P1REN |= IN ;  // P1.3 için pull-up/pull-down direncini etkinleştir
    P1IE |= IN;
    P1IES |= IN;

    __enable_interrupt(); //Enable maskable interrupts


    P2DIR |= BUZZER;
    P2OUT &= ~BUZZER; // Buzzer'ı başlangıçta kapalı tut

    P2DIR |= P2REDLED;
    P2OUT &= ~P2REDLED;

    P2DIR |= P2GreenLED;
    P2OUT &= ~P2GreenLED;

    initLCD();

    clearLCD();

    volatile int x = 1;
    volatile int y = 0;


    __low_power_mode_0(); //Go to low power mode 0



}



#pragma vector=PORT1_VECTOR
__interrupt void P1_Function()
{


    while (1)
    {

        if ((P1IN & IN ) == 0)
        {
            greenLedOn();
            buzzerOn();

            adder = '.';

            delayForOn(250); // 0.25 saniye gecikme

            if ((P1IN & IN ) == 0)
            {

                greenLedOff();
                redLedOn();


                adder = '-';
                while((P1IN & IN ) == 0);
            }
            buzzerOff();
            morseCode[indexCounter] = adder;
            indexCounter++;

        }
        else if(morseCode[0] != '\0')
        {

            greenLedOff();
            redLedOff();

            delayForOff(500);    // 1 saniye bekle

            if ((P1IN & IN ) != 0)
            {

                morseCode[indexCounter] = '\0';



                const char result = findMorseIndex(morseCode);


                if(result != '?')
                {
                    setAddr(x,y);

                    greenLedOn();
                    delayForOff(500);
                    greenLedOff();

                    const char unit[2] = {result, '\0'};

                    writeStringToLCD(unit);

                    greenLedOn();
                    __delay_cycles(100000);
                    greenLedOff();
                    if(x > (14*6-1))
                    {

                        if(y > 5)
                        {
                            y = 0;
                        }
                        else
                        {
                            y = y + 1;
                        }

                        x = 1;

                    }

                    else
                    {
                        x = x + 6;
                    }


                }
                else
                {
                    redLedOn();

                     delayForOff(500);
                     redLedOff();

                }

                indexCounter = 0;

                memset(morseCode, '\0', sizeof(morseCode));

            }

        }


    }

    P1IFG&=~IN; // Reset interrupt flag
}


