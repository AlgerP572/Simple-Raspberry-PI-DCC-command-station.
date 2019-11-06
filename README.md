# Simple-Raspberry-PI-DCC-command-station.
The intent of this project is to explore two different ways of generating a national model railroad association (NMRA) digital command control (DCC) signal using a raspberry pi model 3B.  The two methods explored here are a direct GPIO bit-bang approach using CPU, and a second approach which uses the direct memory access (DMA), pulse width modulation (PWM), and general purpose  input output (GPIO) peripherals on the raspberry pi to generate the DCC signal without using CPU resources.  An oscilloscpe trace showing the two methods is shown below:

![Scope trace showing bit bang test timing vs DMA, PWM timings](Media/BitBangVsDmaTiming.gif)

In the image above, the VNC viewer application is displaying the target Raspberry PI 3, which is running the command station program.  In the upper right-hand corner can be seen the CPU resources required to run this application.  Most of the resources currently are associated with the bit-bang thread which is generating one of the two DCC signals. The command station program is running in the open linux console with the incrementing digits.  These scrolling digits represent the number of DCC Idle packets that the DMAPWM timer has generated. (Currently this simple command station only generates idle
packets...)

Outputs from the two DCC signal generation methods are each directed to its own GPIO pin on the raspberry PI.  These pins are both configured as outputs.  The bit-bang DCC signal is forwarded to GPIO pin 21 and the DMAPWM DCC signal is forwarded to GPIO pin 5. For debugging and oscilloscpe triggering purposes, a streched 500 µsec zero bit is placed in between each idle DCC packet that is generated.  This is useful to be able to see the start and end of each DCC idle packet.

Each GPIO pin (21 and 5), has been connected to a channel of an oscilloscope for timing comparison.  The scope is triggered on a 400 µsec window after a rising edge. This allows synching of the DCC signals by using the streched zero bit mentioned 
previously.  Each channel is labelled.  Pin 21 which is the bit-bang GPIO DCC signal is displayed in channel A.  Pin 5 which is the DMA PWM timed signal is displayed in channel B.  Relative timing differences between the two methods can clearly be seen.

For peripeheral access and control this program uses the APIPIe peripheral libray written in C++ specifically for this purpose.  This library handles all of the timing, GPIO access, DMA control and PWM function required to make these techniques function.
More information on this libray can be found at the link below:

https://github.com/AlgerP572/APLPIe

### DCC signal via GPIO bit-bang

The first approach being explored to generate a DCC idle packet is a simple bit bang approach that uses the ARM/CPU core to handle all of the timing and GPIO pin control.  This appraoch has the advantage of being very simple to program in C/C++ code and can be handled by inserting delays with the required pulse widths in between the GPIO pin write instructions.  C code that generates a DCC idle packet is shown below:

```C++

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

```

Although as with most pre-emptive task operating systems such as Windows 10, and OS/2, linux should on average be very capable and hopefully fast enough to generate an one single DCC idle packet. Questions arise however to sustained accuracy of this timing when the command station application becomes more complete and is asked to take on other tasks at the same time such as display feedback, multiple throttles, and other advanced command station functions.  Will linux be able to sustain NMRA timing requirements while meeting these other application functions?

To answer this question a simple test app is provided that creates two basic threads.  One thread simply creates the idle packet and repeats the the packet on an I/O pin to create a stream of DCC idle packets.  A second thread prints to the screen the number of DCC packets that have so far been generated.  This simulates additional status functionality that a more advanced command station package might provide and gives the bit-bang thread a second thread which will also compete for CPU resources. A screen shot of the application is shown below:

![Screen shot of simple command station showing bit-bang status](Media/SimpleCommandStation.jpg)


### DCC signal via PWM gated DMA transfers from memory to GPIO

TODO...




