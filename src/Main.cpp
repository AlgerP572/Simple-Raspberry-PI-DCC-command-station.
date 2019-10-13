#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <unistd.h>

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

#define   BitBangDCCPin 21
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

	gpio.Export(BitBangDCCPin);
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

	gpio.Unexport(BitBangDCCPin);
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

void BitBangIdlePacket()
{
	// using 98 탎ec instead of 100 to force
	// zero packets to use Microsecond spin and
	// avoid the overhead with calling the system
	// time APIs. See Delay::Microseconds for details
	// Ans also accomodating for overhead in MicroSecondSpin
	// so specifying 2 탎ec less than target.

	for (int i = 0; i < 12; i++)
	{
		gpio.WritePin(BitBangDCCPin, PinState::Low);
		Delay::Microseconds(56);
		gpio.WritePin(BitBangDCCPin, PinState::High);
		Delay::Microseconds(56);
	}

	// Byte end bit
	gpio.WritePin(BitBangDCCPin, PinState::Low);
	Delay::Microseconds(98);
	gpio.WritePin(BitBangDCCPin, PinState::High);
	Delay::Microseconds(98);

	// Byte: Value = 0xFF (255)
	for (int i = 0; i < 8; i++)
	{
		gpio.WritePin(BitBangDCCPin, PinState::Low);
		Delay::Microseconds(56);
		gpio.WritePin(BitBangDCCPin, PinState::High);
		Delay::Microseconds(56);
	}

	// Byte end bit
	gpio.WritePin(BitBangDCCPin, PinState::Low);
	Delay::Microseconds(98);
	gpio.WritePin(BitBangDCCPin, PinState::High);
	Delay::Microseconds(98);

	// Byte: Value = 0x00 (0)
	for (int i = 0; i < 8; i++)
	{
		gpio.WritePin(BitBangDCCPin, PinState::Low);
		Delay::Microseconds(98);
		gpio.WritePin(BitBangDCCPin, PinState::High);
		Delay::Microseconds(98);
	}

	// Byte end bit
	gpio.WritePin(BitBangDCCPin, PinState::Low);
	Delay::Microseconds(98);
	gpio.WritePin(BitBangDCCPin, PinState::High);
	Delay::Microseconds(98);

	// Checksum Byte: Value = 0xFF (255)
	for (int i = 0; i < 8; i++)
	{
		gpio.WritePin(BitBangDCCPin, PinState::Low);
		Delay::Microseconds(56);
		gpio.WritePin(BitBangDCCPin, PinState::High);
		Delay::Microseconds(56);
	}

	// Packet end bit.
	gpio.WritePin(BitBangDCCPin, PinState::Low);
	Delay::Microseconds(56);
	gpio.WritePin(BitBangDCCPin, PinState::High);
	Delay::Microseconds(56);
}

//static bool _mainDmaLoopRunning = true;

// Or also use this to turn bit bang off
static bool _mainDmaLoopRunning = false;

void* BitBangThread(void* ptr)
{
	gpio.SetPinMode(BitBangDCCPin, PinMode::Output);

	while(_mainDmaLoopRunning)
	{
		// This is to demonstrate pitfalls of using bit bang timing.  Since
		// linux will pre-empt you at any time there will be pulses that 
		// do not conform to NMRA timing...
		//
		// Timing of this same packet can vary by as much as 350 탎...
		// This may work with some decoders but would not pass NMRA
		// conformance testing.
		BitBangIdlePacket();

		// This for debugging a streched zero to appear between
		// packets.  Set your scope to window trigger on a 500 탎ec
		// wide pulse.
		gpio.WritePin(BitBangDCCPin, PinState::Low);
		Delay::Microseconds(500);
		display.Display();
		gpio.WritePin(BitBangDCCPin, PinState::High);
		Delay::Microseconds(500);
		display.Display();
	}

	return NULL;
}

int main(void)
{
	pthread_t bitBangThread;

	// Register signals 
	signal(SIGQUIT, sig_handler);
	signal(SIGABRT, sig_handler);

	// Now that library is intialized the individual devices
	// can be intialized.
	sysInit();

	pthread_create(&bitBangThread,
		NULL,
		BitBangThread,
		(void*)NULL);	

	PulseTrain& pulseTrain = commandStation.Start();
	
	long long unsigned int runCount = 1000000;
	do
	{		
		if (pulseTrain.OuputCount >= runCount)
		{
			break;
		}

		int displayCount = (int) pulseTrain.OuputCount % 10000;
		display.SetDisplayValue(displayCount);
		display.Display();
		printf("%d\n", displayCount);
		nanosleep((const struct timespec[]) { {0, 100000L} }, NULL);

	} while (pulseTrain.Valid);

	// Stop the pulse train.
	pulseTrain.Repeat = false;
	_mainDmaLoopRunning = false;

	// Wait for completion.
	do {} while (pulseGenerator.IsRunning());
	pulseTrain.OuputCount = 0;
	
	pthread_join(bitBangThread, NULL);

 	sysUninit();
	return 0;
}


