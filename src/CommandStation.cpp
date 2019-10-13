#include "CommandStation.h"

CommandStation::CommandStation(PulseGenerator& pulseGenerator, Gpio& gpio, int dccPin) :
	Device("Simple Command Station"),
	_gpio(gpio),
	_pulseGenerator(pulseGenerator),	
	_pulseTrain(1 << dccPin),
	_dccPin(dccPin)
{	
}

PulseTrain& CommandStation::Start()
{		

	AddIdlePacket(_pulseTrain);

	// A strectched zero for debugging. Real command station
	// may excluded this if improved packet throughput is desired.
	_pulseTrain.Add(PinState::Low, MICROSEC_TO_RNG1(500));
	_pulseTrain.Add(PinState::High, MICROSEC_TO_RNG1(500));

	_pulseGenerator.Add(_pulseTrain);
	_pulseTrain.Repeat = true;
	
	// This use case requires the sync pin to be high at the
	// end of the first buffer. It is being pulsed here so that
	// a sync pulse can be viewed on the scope.
	_pulseGenerator.WriteSyncPinState(PinState::Low);
	_pulseGenerator.WriteSyncPinState(PinState::High);

	// Wait for the interrupt handlers to clear.
	Delay::Milliseconds(10);
	_pulseGenerator.Start();

	return _pulseTrain;
}

void CommandStation::AddIdlePacket(PulseTrain& pulseTrain)
{
	AddPreamble(pulseTrain);
	AddPacketStartBit(pulseTrain);

	AddOneByte(pulseTrain);
	AddByteEndBit(pulseTrain);

	AddZeroByte(pulseTrain);
	AddByteEndBit(pulseTrain);

	AddOneByte(pulseTrain);
	AddPacketEndBit(pulseTrain);
}

void CommandStation::AddPreamble(PulseTrain& pulseTrain)
{
	for (int i = 0; i < 12; i++)
	{
		pulseTrain.Add(PinState::Low, MICROSEC_TO_RNG1(58));
		pulseTrain.Add(PinState::High, MICROSEC_TO_RNG1(58));
	}
}

void CommandStation::AddPacketStartBit(PulseTrain& pulseTrain)
{
	pulseTrain.Add(PinState::Low, MICROSEC_TO_RNG1(100));
	pulseTrain.Add(PinState::High, MICROSEC_TO_RNG1(100));
}

void CommandStation::AddPacketEndBit(PulseTrain& pulseTrain)
{
	pulseTrain.Add(PinState::Low, MICROSEC_TO_RNG1(58));
	pulseTrain.Add(PinState::High, MICROSEC_TO_RNG1(58));
}

void CommandStation::AddOneByte(PulseTrain& pulseTrain)
{
	for (int i = 0; i < 8; i++)
	{
		pulseTrain.Add(PinState::Low, MICROSEC_TO_RNG1(58));
		pulseTrain.Add(PinState::High, MICROSEC_TO_RNG1(58));
	}
}

void CommandStation::AddZeroByte(PulseTrain& pulseTrain)
{
	for (int i = 0; i < 8; i++)
	{
		pulseTrain.Add(PinState::Low, MICROSEC_TO_RNG1(100));
		pulseTrain.Add(PinState::High, MICROSEC_TO_RNG1(100));
	}
}

void CommandStation::AddByteEndBit(PulseTrain& pulseTrain)
{
	pulseTrain.Add(PinState::Low, MICROSEC_TO_RNG1(100));
	pulseTrain.Add(PinState::High, MICROSEC_TO_RNG1(100));
}

void CommandStation::SysInit(void)
{
	_gpio.SetPinMode(_dccPin, PinMode::Output);

	// Here for debugging purpose can look on scope for this.
	_gpio.WritePin(_dccPin, PinState::High);
	_gpio.WritePin(_dccPin, PinState::Low);
	_gpio.WritePin(_dccPin, PinState::High);
}

void CommandStation::SysUninit(void)
{
	_gpio.SetPinMode(_dccPin, PinMode::Input);
}