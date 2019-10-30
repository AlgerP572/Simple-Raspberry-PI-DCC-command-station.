# Simple-Raspberry-PI-DCC-command-station.
The intent of this project is to explore two differnt ways of generating a national
model railroad association (NMRA) digital command control (DCC) signal using a 
raspberry pi model 3B.  The two methods explored here are a direct GPIO bit-bang
approach using CPU, and a second approach which uses the direct memory access (DMA),
pulse width modulation (PWM), and general purpose input output (GPIO) peripherals
on the raspberry pi to generate the DCC signal without using CPU resources.  An
oscilloscpe trace showing the two methods is shown below:

![Scope trace showing bit bang test timing vs DMA, PWM timings](Media/BitBangVsDmaTiming.gif)

The VNC viewer application is displaying the target Raspberry PI 3 which is running
the command station program.  In the right hand corner can be seen the CPU resources
required to run this application.  Most of the resources currently are associated with
the bit-bang thread which is generating one of the two DCC signals.  The command station
program is running in the open linux console with the incrementing digits.  These scrolling
digits represent the number of DCC Idle packets that the DMAPWM timer has generated.

