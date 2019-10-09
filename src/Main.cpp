#include <stdio.h>
#include <stdint.h>
#include <signal.h>

#include "../../../APLPIe/Src/Headers/Clock.h"
#include "../../../APLPIe/Src/Headers/Dma.h"
#include "../../../APLPIe/Src/Headers/Gpio.h"
#include "../../../APLPIe/Src/Headers/Pwm.h"
#include "../../../APLPIe/Src/Headers/Timer.h"
#include "../../../APLPIe/Src/Headers/Display.h"
#include "../../../APLPIe/Src/Headers/PulseGenerator.h"
#include "../../../APLPIe/Src/Headers/Delay.h"

#include "CommandStation.h"


// Display (These select the digit to display.)
#define   DisplayPin0    2
#define   DisplayPin1    3
#define   DisplayPin2    8
#define   DisplayPin3    7

#define   DisplayCharPin0 17
#define   DisplayCharPin1 18
#define   DisplayCharPin2 27
#define   DisplayCharPin3 22
#define   DisplayCharPin4 23
#define   DisplayCharPin5 24
#define   DisplayCharPin6 25
#define   DisplayCharPin7 4

#define   DmaDCCSignalPin 5
#define   DmaSyncPin 6

static CharacterDisplayPins characterPins = CharacterDisplayPins(DisplayCharPin0,
	DisplayCharPin1,
	DisplayCharPin2,
	DisplayCharPin3,
	DisplayCharPin4,
	DisplayCharPin5,
	DisplayCharPin6,
	DisplayCharPin7);

// Output pulses for testing
//#define LeftRailOutput 21
//#define LeftRailOutput 22

// AxleCounter
#define LeftRailInput    16
#define RightRailInput   12

#define LeftRailOutput 19
#define RightRailOutput 26

static Dma dma("Dma Controller");
static Gpio gpio("Gpio Controller");
static Pwm pwm("Pwm Controller");
static Clock clock1("Clock Controller");

const int numPeripherals = 4;
static Peripheral** peripherals = new Peripheral*[numPeripherals];


static FourDigitSevenSegmentDisplay display(gpio,
	DisplayPin0,
	DisplayPin1,
	DisplayPin2,
	DisplayPin3,
	characterPins);

static PulseGenerator pulseGenerator(gpio,
	dma,
	pwm,
	clock1,
	DmaSyncPin,
	1);

static CommandStation commandStation(pulseGenerator,
	gpio,
	DmaDCCSignalPin);

const int numDevices = 3;
static Device** devices = new Device * [numDevices];

void sysInit(void)
{
	peripherals[0] = &dma;
	peripherals[1] = &gpio;
	peripherals[2] = &pwm;
	peripherals[3] = &clock1;

	devices[0] = &display;
	devices[1] = &pulseGenerator;
	devices[2] = &commandStation;
	
	// Devices depend on peripherals so init them first.
	for (int i = 0; i < numPeripherals; i++)
	{
		peripherals[i]->SysInit();
	}

	gpio.Export(LeftRailInput);
	gpio.Export(RightRailInput);

	gpio.Export(LeftRailOutput);
	gpio.Export(RightRailOutput);

	gpio.Export(DisplayPin0);
	gpio.Export(DisplayPin1);
	gpio.Export(DisplayPin2);
	gpio.Export(DisplayPin3);

	gpio.Export(DisplayCharPin0);
	gpio.Export(DisplayCharPin1);
	gpio.Export(DisplayCharPin2);
	gpio.Export(DisplayCharPin3);
	gpio.Export(DisplayCharPin4);
	gpio.Export(DisplayCharPin5);
	gpio.Export(DisplayCharPin6);
	gpio.Export(DisplayCharPin7);

	gpio.Export(DmaDCCSignalPin);
	gpio.Export(DmaSyncPin);

	// Devices depend on peripherals so init peripherals first.
	for (int i = 0; i < numDevices; i++)
	{
		devices[i]->SysInit();
	}
}

void sysUninit(void)
{
	// Devices depend on peripherals so Uninit devices first.
	for (int i = 0; i < numDevices; i++)
	{
		devices[i]->SysUninit();
	}

	gpio.Unexport(LeftRailInput);
	gpio.Unexport(RightRailInput);

	gpio.Unexport(LeftRailOutput);
	gpio.Unexport(RightRailOutput);

	gpio.Unexport(DisplayPin0);
	gpio.Unexport(DisplayPin1);
	gpio.Unexport(DisplayPin2);
	gpio.Unexport(DisplayPin3);

	gpio.Unexport(DisplayCharPin0);
	gpio.Unexport(DisplayCharPin1);
	gpio.Unexport(DisplayCharPin2);
	gpio.Unexport(DisplayCharPin3);
	gpio.Unexport(DisplayCharPin4);
	gpio.Unexport(DisplayCharPin5);
	gpio.Unexport(DisplayCharPin6);
	gpio.Unexport(DisplayCharPin7);

	gpio.Unexport(DmaDCCSignalPin);
	gpio.Unexport(DmaSyncPin);

	for (int i = 0; i < numPeripherals; i++)
	{
		peripherals[i]->SysUninit();
	}
}

void sig_handler(int sig)
{
	sysUninit();
}

//void AddLongPulseTrain(PulseTrain& pulseTrain)
//{
//	for (int i = 0; i < 12; i++)
//	{
//		pulseTrain.Add(PinState::Low, MICROSEC_TO_RNG1(58));
//		pulseTrain.Add(PinState::High, MICROSEC_TO_RNG1(58));
//	}
//
//	pulseTrain.Add(PinState::Low, MICROSEC_TO_RNG1(100));
//	pulseTrain.Add(PinState::High, MICROSEC_TO_RNG1(100));
//
//	for (int i = 0; i < 8; i++)
//	{
//		pulseTrain.Add(PinState::Low, MICROSEC_TO_RNG1(58));
//		pulseTrain.Add(PinState::High, MICROSEC_TO_RNG1(58));
//	}
//
//	pulseTrain.Add(PinState::Low, MICROSEC_TO_RNG1(100));
//	pulseTrain.Add(PinState::High, MICROSEC_TO_RNG1(100));
//
//	for (int i = 0; i < 8; i++)
//	{
//		pulseTrain.Add(PinState::Low, MICROSEC_TO_RNG1(100));
//		pulseTrain.Add(PinState::High, MICROSEC_TO_RNG1(100));
//	}
//
//	pulseTrain.Add(PinState::Low, MICROSEC_TO_RNG1(100));
//	pulseTrain.Add(PinState::High, MICROSEC_TO_RNG1(100));
//
//	for (int i = 0; i < 8; i++)
//	{
//		pulseTrain.Add(PinState::Low, MICROSEC_TO_RNG1(58));
//		pulseTrain.Add(PinState::High, MICROSEC_TO_RNG1(58));
//	}
//
//	pulseTrain.Add(PinState::Low, MICROSEC_TO_RNG1(58));
//	pulseTrain.Add(PinState::High, MICROSEC_TO_RNG1(58));
//}

//void GeneratePulseTrainWithRepeat(PulseGenerator& pulseGenerator)
//{
//	//now set our pin as an output:
//	/*gpio.SetPinMode(DmaDCCSignal, PinMode::Output);
//	gpio.WritePin(DmaDCCSignal, PinState::High);
//	gpio.WritePin(DmaDCCSignal, PinState::Low);
//	gpio.WritePin(DmaDCCSignal, PinState::High);*/
//		
//	for (int j = 0; j < 5; j++)
//	{
//		PulseTrain pulseTrain(1 << DmaDCCSignalPin);
//
//		for (int i = 0; i < j; i++)
//		{
//
//			AddLongPulseTrain(pulseTrain);
//
//			pulseTrain.Add(PinState::Low, MICROSEC_TO_RNG1(500));
//			pulseTrain.Add(PinState::High, MICROSEC_TO_RNG1(500));
//		}
//
//		pulseTrain.Repeat = true;
//
//		pulseGenerator.Add(pulseTrain);
//
//		// start sync pin high so we see the end on
//		// scope.
//		pulseGenerator.WriteSyncPinState(PinState::Low);
//		pulseGenerator.WriteSyncPinState(PinState::High);
//		pulseGenerator.WriteSyncPinState(PinState::Low);
//		pulseGenerator.WriteSyncPinState(PinState::High);
//
//		// Give wait a brief time to ensure above interrupts
//		// are processed as non-running interrupts...
//		Delay::Milliseconds(10); // Also nice place for breakpoint :o
//
//		pulseGenerator.Start();
//
//		// A pulse train of zero length can never start.
//		// This pulse train will also be marked as
//		// invalid.  Only pulse trains that actually start
//		// DMA hardware are valid.
//		do
//		{
//			if (pulseTrain.OuputCount >= 10000)
//			{
//				break;
//			}
//
//			int displayCount = pulseTrain.OuputCount;
//			display.SetDisplayValue(displayCount);
//			display.Display();
//
//		} while (pulseTrain.Valid);
//
//		// Stop the pulse train.
//		pulseTrain.Repeat = false;
//
//		// Wait for completion.
//		do {} while (pulseGenerator.IsRunning());
//		pulseTrain.OuputCount = 0;
//	}
//}

int main(void)
{

	// Register signals 
	signal(SIGQUIT, sig_handler);
	signal(SIGABRT, sig_handler);

	// Now that library is intialized the individual devices
	// can be intialized.
	sysInit();

	//	GeneratePulseTrainWithRepeat(pulseGenerator);

	for (int i = 0; i < 1000; i++)
	{
		display.SetDisplayValue(i);

		// This is to demonstrate pitfalls of using bit bang timing.  Since
		// linux will pre-empt you at any time there will be pulses that 
		// do not conform to NMRA timing...
		//
		// Timing of this same packet can vary by as much as 350 µs...
		// This may work with some decoders but would not pass NMRA
		// conformance testing.
		commandStation.BitBangIdlePacket();
		display.SetDisplayValue(i);

		// This for debugging a streched zero to appear between
		// packets.  Set your scope to window trigger on a 500 µsec
		// wide pulse.
		gpio.WritePin(DmaDCCSignalPin, PinState::Low);
		Delay::Microseconds(500);
		display.Display();
		gpio.WritePin(DmaDCCSignalPin, PinState::High);
		Delay::Microseconds(500);
		display.Display();	
	}

	PulseTrain& pulseTrain = commandStation.Start();
	
	long long unsigned int runCount = 100000;
	do
	{		
		if (pulseTrain.OuputCount >= runCount)
		{
			break;
		}

		int displayCount = (int) pulseTrain.OuputCount % 10000;
		display.SetDisplayValue(displayCount);
		display.Display();

	} while (pulseTrain.Valid);

	// Stop the pulse train.
	pulseTrain.Repeat = false;

	// Wait for completion.
	do {} while (pulseGenerator.IsRunning());
	pulseTrain.OuputCount = 0;

	sysUninit();
	return 0;
}

