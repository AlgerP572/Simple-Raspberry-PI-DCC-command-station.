# Simple-Raspberry-PI-DCC-command-station.
The intent of this project is to explore two differnt ways of generating a national
model railroad association (NMRA) digital command control (DCC) signal using a 
raspberry pi model 3B.  The two methods explored here are a direct GPIO bit bang
approach using CPU, and a second approach which uses the DMA, PWM, and GPIO
peripherals on the raspberry pi to generati the DCC signal without using CPU
resources.  An oscilloscpe trace showing the two methods is shown below:

![Scope trace showing bit bang test timing vs DMA, PWM timings](Media/BitBangVsDmaTiming.gif)