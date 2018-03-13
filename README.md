# Senior-Design
-- Project Breach --

-- Connor Grehlinger, Keith Doggett, Matthew Schlogel, John Oribioye, Matthew Leung --

Repository for senior design course. Source code for microcontroller (Arduino R2)

To collect card data from a common RFID proximity card using the antenna and demodulation circuit in Project Breach's prototype, Demodulation_Circuit_Read_Store.cpp must first be compiled and ran on an Arduino capable of interfacing with a microSD shield (Arduino R2 and HanRun ethernet/microSD shield used for this prototype). The demodulation circuit must also be processing the encoded signal at program start on the Ardunio. 

The data from the demodulation circuit output is collected, decoded and written to a MicroSD card on the Ardunio.

To replay the previosuly collected proximity card data, RFID_Replay_Encoder.cpp must be compiled and ran on the Arduino. The microSD card used to save the decoded output of the demodulation circuit must remain in the Arduino. A binary sequence is constructed from this data and used to control an output signal of varying frequency used for the modulation circuit of Breach's prototype. 
