#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <unistd.h>
#include <semaphore.h>

#include "../../APLPIe/Src/Headers/Clock.h"
#include "../../APLPIe/Src/Headers/Dma.h"
#include "../../APLPIe/Src/Headers/Gpio.h"
#include "../../APLPIe/Src/Headers/Pwm.h"
#include "../../APLPIe/Src/Headers/Timer.h"
#include "../../APLPIe/Src/Headers/Display.h"
#include "../../APLPIe/Src/Headers/PulseGenerator.h"
#include "../../APLPIe/Src/Headers/Delay.h"

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
#define   DmaDCCPacketStart 16
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
	4);

static CommandStation commandStation(pulseGenerator,
	gpio,
	DmaDCCSignalPin);

const int numDevices = 3;
static Device** devices = new Device * [numDevices];
static sem_t bitBangSem;
static bool mainDmaLoopRunning = true;

void BitBangIdlePacket()
{
	// using 98 �sec instead of 100 to force
	// zero packets to use Microsecond spin and
	// avoid the overhead with calling the system
	// time APIs. See Delay::Microseconds for details
	// Ans also accomodating for overhead in MicroSecondSpin
	// so specifying 2 �sec less than target.

	for (int i = 0; i < 12; i++)
	{
		gpio.WritePin(BitBangDCCPin, PinState::Low);
		Delay::Microseconds(58);
		gpio.WritePin(BitBangDCCPin, PinState::High);
		Delay::Microseconds(58);
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
		Delay::Microseconds(58);
		gpio.WritePin(BitBangDCCPin, PinState::High);
		Delay::Microseconds(58);
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
		Delay::Microseconds(58);
		gpio.WritePin(BitBangDCCPin, PinState::High);
		Delay::Microseconds(58);
	}

	// Packet end bit.
	gpio.WritePin(BitBangDCCPin, PinState::Low);
	Delay::Microseconds(58);
	gpio.WritePin(BitBangDCCPin, PinState::High);
	Delay::Microseconds(58);
}


void BitBangISR(void* ptr)
{

	// Generally a bit bang thread would run at tha same priority as
	// other threads in the application.  To emulate that and to synchronize
	// the bit bang version of the DCC signal with the DMA version. A GPIO
	// pin interrupt handler is being triggered each time a new DMA
	// DCC packet starts.  Since this thread is at APLPIe ISR FIFO priority
	// it is transferred to another thread of regular application priority
	// This subjects the bit bang DCC thread to the same timing conditions
	// it would see in a typical bit bang based applicaton.
	sem_post(&bitBangSem);
}

void* BitBangThread(void* ptr)
{
	while (mainDmaLoopRunning)
	{
		sem_wait(&bitBangSem);

		gpio.WritePin(BitBangDCCPin, PinState::High);
		Delay::Microseconds(500);

		// This is to demonstrate pitfalls of using bit bang timing.  Since
		// linux will pre-empt you at any time there will be pulses that 
		// do not conform to NMRA timing...
		//
		// Timing of this same packet can vary by as much as 350 �s...
		// This may work with some decoders but would not pass NMRA
		// conformance testing.
		BitBangIdlePacket();

		// This for debugging a streched zero to appear between
		// packets.  Set your scope to window trigger on a 500 �sec
		// wide pulse.
		gpio.WritePin(BitBangDCCPin, PinState::Low);
	}
}

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
	gpio.Export(DmaDCCPacketStart);
	gpio.Export(DmaDCCSignalPin);
	gpio.Export(DmaSyncPin);

	// Devices depend on peripherals so init peripherals first.
	for (int i = 0; i < numDevices; i++)
	{
		devices[i]->SysInit();
	}

	// Any specialize init specific to the application
	gpio.SetPinMode(BitBangDCCPin, PinMode::Output);
	gpio.SetPinMode(DmaDCCPacketStart, PinMode::Output);

	gpio.SetIsr(DmaDCCPacketStart,
		IntTrigger::Rising,
		BitBangISR,
		NULL);
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
	gpio.Unexport(DmaDCCPacketStart);
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

int main(void)
{
	pthread_t bitBangThread;

	// Register signals 
	signal(SIGQUIT, sig_handler);
	signal(SIGABRT, sig_handler);

	sem_init(&bitBangSem, 0, 0);

	// Now that library is intialized the individual devices
	// can be intialized.
	sysInit();

	// To use the bit-bang method uncomment this line of code.
	/*pthread_create(&bitBangThread,
		NULL,
		BitBangThread,
		(void*)NULL);*/

	PulseTrain& pulseTrain = commandStation.Start();
	
	long long unsigned int runCount = 20000000;
	int displayDelay = 0;
	do
	{
		/*if (pulseTrain.OuputCount >= runCount)
		{
			break;
		}*/

		int displayCount = (int)pulseTrain.OuputCount;
		int ledCount = displayCount % 10000;

		/*display.SetDisplayValue(ledCount);
		display.Display();*/

		if (displayDelay > 5000)
		{
			printf("%d\n", displayCount);
			displayDelay = 0;
		}	
		displayDelay++;
	
		nanosleep((const struct timespec[]) { {0, 100000L} }, NULL);

	} while (pulseTrain.Valid);

	// Stop the pulse train.
	pulseTrain.Repeat = false;

	// Wait for completion.
	do {} while (pulseGenerator.IsRunning());
	pulseTrain.OuputCount = 0;
	
	// Just in case its sleeping wake up the thread.
	sem_post(&bitBangSem);
	mainDmaLoopRunning = false;

	pthread_join(bitBangThread, NULL);
	sem_close(&bitBangSem);

	commandStation.Stop();

 	sysUninit();
	return 0;
}


