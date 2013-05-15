/**

    @file       main.cpp
    @brief      HiResTimer Example

    This example demonstates how to use the functions provided by the HiResTimer
    utility. This utility offers an easier way to configure and use the hardware
    timers provided by the various processors.

    The High Resolution Timer class is designed to facilitate delays and triggers
    in the submillisecond range. The class is based on the hardware DMA timers
    and operates using interrupts to achieve such levels of accuracy. All timers
    use a level 6 interrupt.

    Note:
        The MCF5272 processor is not compatible with the HiResTimer utility.
**/

#include "predef.h"
#include <stdio.h>
#include <ctype.h>
#include <system.h>
#include <basictypes.h>

#ifdef MOD5213
#include <basictypes.h>
#include <serialirq.h>
#include <system.h>
#include <constants.h>
#include <ucos.h>
#include <SerialUpdate.h>
#include <smarttrap.h>
#include <gdbstub.h>

#else
#include <init.h>
#endif

#if( ( defined MOD5270 ) || ( defined MOD5282 ) || (defined MOD5234) || (defined MOD5213) || (defined MOD5441X)  )
#include <pins.h>
#endif

#include <utils.h>
#include <stdlib.h>
#include <HiResTimer.h>

const char * AppName = "HiResTimer Example";

DWORD LEDValue = 0;
double TotalTime = 0;
char Buffer[1000];

void IncrementLEDs( );
void SetGPIOtoDMATimerOut();  // Timer pin set to timer out function for driver callback

extern "C" void UserMain( void * pd )
{

#ifdef MOD5213
    SimpleUart(0,115200);
    assign_stdio(0);
    EnableSerialUpdate();
#else
    init();
#endif

    iprintf("Application: %s\r\nNNDK Revision: %s\r\n",AppName,GetReleaseTag());

    HiResTimer *timer;

    while( 1 )
    {
        iprintf( "\r\n1) Use DMA Timer to time an operation\r\n" );
        iprintf( "2) Use DMA Timer to repeatedly trigger an interrupt routine\r\n" );
        iprintf( "3) Use DMA Timer to generate a precise OSTimeDly()\r\n" );
        iprintf( "4) Use DMA Timer to generate a precise polling delay\r\n" );
        iprintf( "5) Use DMA Timer to generate a Clock output\r\n" );
        iprintf( "Select the operation to test :" );
        gets( Buffer );
        iprintf( "\r\n" );
        switch( Buffer[0] )
        {
            // Using the timer as a stopwatch
        case '1':
        {
            // Obtain a pointer to timer 0
            timer = HiResTimer::getHiResTimer(0);
            // Initialize the timer
            timer->init();
            iprintf( "Press enter to start HiResTimer... \r\n" );
            gets( Buffer );
            // Start the timer
            timer->start();
            iprintf( "Press enter to stop HiResTimer... \r\n" );
            gets( Buffer );
            // Read the time elapsed
            TotalTime = timer->readTime();
            printf( "The total time elapsed was %g seconds\r\n", TotalTime );
            // Stop and clear the timer
            //iprintf(timer->toString());
            timer->stopClear();
            break;
        }
        // Using the timer to control a periodic event
        case '2':
        {
            timer = HiResTimer::getHiResTimer(0);
            // Set the function to be called when
            timer->setInterruptFunction(IncrementLEDs); // Point timer interrupt call-back to LED function
            iprintf( "Input number of seconds (floating point) between LED increments:" );
            gets( Buffer );
            timer->init(strtod( Buffer, NULL ) );
            timer->start();
            iprintf( "\r\nPress enter to disable incrementing LEDs\r\n" );
            gets( Buffer );
            timer->stopClear();
            timer->clearInterruptFunction();
            LEDValue = 0;
            putleds( 0 );
            break;
        }
        // Demonstrating delay function
        case '3':
        {
            iprintf( "Input number of seconds (floating point) for precise OSTimeDly():" );
            gets( Buffer );
            DWORD StartingTick = TimeTick;
            timer = HiResTimer::getHiResTimer();
            timer->delay(strtod( Buffer, NULL ));
            iprintf( "\r\nDelay finished in %d system ticks\r\n", ( TimeTick - StartingTick ) );
            break;
        }
        // Demonstrating the polling delay function
        case '4':
        {
            iprintf( "Input number of seconds (floating point) for precise polling time delay:" );
            gets( Buffer );
            double DelayTime = strtod( Buffer, NULL );
            DWORD StartingTick = TimeTick;
            timer = HiResTimer::getHiResTimer();
            timer->pollingDelay( DelayTime );
            iprintf( "\r\nDelay finished in %d system ticks\r\n", ( TimeTick - StartingTick ) );
            break;
        }
        case '5':
        {
            //--------------------------------------------------
            // NOTE - You must manually set the correct timer output pin for the
            //        proper timer output function for the clock generator to work
            //--------------------------------------------------
#if( ( defined MOD5270 ) || ( defined MOD5282 ) )
            J2[34].function( PINJ2_34_DTOUT1 );
#elif(defined MOD5234)
            J2[34].function( PINJ2_34_DT1OUT );
#elif(defined MOD5213)
            Pins[23].function( PIN23_DTOUT1 );
#endif

            iprintf( "Input the Frequency (floating point) to generate a clock on timer 1 out:" );
            gets( Buffer );
            timer = HiResTimer::getHiResTimer();
            timer->clockGenerator( strtod( Buffer, NULL) );
            iprintf( "\r\n" );
            break;
        }

        default:
            break;
        }
    }
}

// A simple funtion that increments the LEDs displayed on the dev-board
void IncrementLEDs()
{
    // The Timer parameter is not used here since we do not need to determine which timer interrupted
    LEDValue++;
#ifndef MCF52234
    putleds( LEDValue );
#endif
}



