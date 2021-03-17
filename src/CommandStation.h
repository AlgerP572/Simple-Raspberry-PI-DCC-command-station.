#pragma once

#include "../../APLPIe/Src/Headers/Delay.h"
#include "../../APLPIe/Src/Headers/PulseGenerator.h"

class CommandStation : public Device
{
private:
	Gpio& _gpio;
	PulseGenerator& _pulseGenerator;
	PulseTrain _pulseTrain;	
	int _dccPin;
	

	void AddIdlePacket(PulseTrain& pulseTrain);
	void AddPreamble(PulseTrain& pulseTrain);
	void AddByteEndBit(PulseTrain& pulseTrain);
	void AddPacketStartBit(PulseTrain& pulseTrain);
	void AddPacketEndBit(PulseTrain& pulseTrain);
	void AddOneByte(PulseTrain& pulseTrain);
	void AddZeroByte(PulseTrain& pulseTrain);

public:
	CommandStation(PulseGenerator& pulseGenerator, Gpio& gpio, int dccPin);

	PulseTrain&  Start();
	void Stop();
	void BitBangIdlePacket();

	void virtual SysInit(void);
	void virtual SysUninit(void);
};
