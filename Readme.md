# Welcome to FISBlocks!

[ENG]
[[ESP]](./ReadmeESP.md)

![Texto alternativo](media/main2.jpg "FIS-Blocks").


## Sumary:

The FISBlocks is an Arduino based device that allows us to consult in real time mechanical parameters of the vehicle to diagnose its state and predict faults. It implements part of the private protocol KWP1281 used in all vehicles of the group VAG (Audi, Seat, Volkswagen ...) since 1998 to carry out the diagnosis and obtain the information. In addition, this device also implements part of the private 3LB protocol that allows us to control the content that is displayed by the screen of the instruments that incorporate FIS-DIS technology.

Some of the interesting facts we can get:
* Liters of fuel
* Actual coolant temperature
* AFR
* Turbo pressure
* IAT
* EGT
* Delays / cylinder
* Misfires / cylinder
* MAF
* Voltage
* Engine load
* Securities demanded by the ECU
* ... And in general, any data that can be obtained through the Measurement Blocks of the unit.

![Texto alternativo](media/fis2.gif "FIS-Blocks").

This project has two main objectives:

* Extract information from the vehicle control units using the KWP1281 protocol and the Measurement Blocks.

* Implement 3LB protocol to display information in the instrument cluster.

### Update 10/08/2018!!!

![Texto alternativo](media/pcbFISBlocksFull.jpg "FIS-Blocks").
![Texto alternativo](media/pcbFISBlocksLow.jpg "FIS-Blocks").

FISBlocks PCB finished! As the assembly of the circuit using a KKL cable is not very practical I have developed this PCB for development.
Its function is exactly the same but the result is more integrated, there is no risk of making mistakes buying the cable and it is easier to integrate it in the car.

FISBlocks PCB is available in limited units, the prices are as follows:

* 1 unit -> 50€
* 5 units -> 200€

![Texto alternativo](media/pcbFISBlocksUnits.PNG "FIS-Blocks").

* Shipping costs not included, shipments are made to the whole world. Payment will be made by PayPal.
* Includes the PCB tested with all the components soldered, the source code with the most current version of the code and a custom logo.

![Texto alternativo](media/pcbFISBlocksSocial.png "FIS-Blocks").

If you are interested, please contact me by email: samueliban@gmail.com



### Update 22/12/2017!!!

If you are interested in the project and want to use the Premium version of FISBlocks, FISBlocks+ has been developed that includes among others:

* Various display modes: Top screen (UPPER FIS) or full screen (FULL FIS).
* Change of parameters and visualization with the handle of the cruise control.
* Higher refresh rate.
* Greater number of parameters per screen.
* Performance and usability improvements.

#### Click on the image to see the demonstration video:

[![IMAGE ALT TEXT HERE](/media/fisblocksplus.jpg)](https://www.youtube.com/watch?v=hIhcAbNPSvY)

Make a donation to the address samueliban@gmail.com through PayPal indicating your email address to receive the complete code and contribute to the development of new features.

### Update 16/01/2018!!!

Graphics happened!

#### Click on the image to see the demonstration video:

[![IMAGE ALT TEXT HERE](/media/fisblockspluscustom.jpg)](https://www.youtube.com/watch?v=ZmHX70X5kUM)

If you want the custom version of FISBlocks+ which includes logos at startup and have your own custom logo contribute to the project by making a donation of 25€ to the address samueliban@gmail.com indicating your email address and send me your logo in black and white in size 64x88 pixels, you will receive the code with your custom logo.

## Material needed to build the FISBlocks:

![Texto alternativo](media/components.jpg "Components")

* Arduino UNO or similar (ATmega328)
* USB ODB2 Diagnostic Cable 409.1
* Welder, tin cable and solder level "Little ninja" :)
* Optional: 12v to 5v converter

These components can be obtained in many places on the Internet, if you have problems getting them and I could help you get them, contact me through samueliban@gmail.com.


## Preparation of cable 409.1:

The hack designed by Andrew consists of taking advantage of the comparator LM that incorporates the cable OBD to use it with the Arduino through SofwareSerial.

To do this we must dismantle the housings and make 3 modifications on the PCB of the cable:

![Texto alternativo](media/hackobd.jpg "Hack").

* Cut communication with the FTDI chip that goes to TX and weld the new TX line for Arduino.
* Solder RX to Arduino.
* Solder line 5v to 5v Arduino (to be used as reference voltage, not for power).

The result is this:

![Texto alternativo](media/obd2.jpg "OBD").

Once the welds are made we can make the connections with the Arduino.


## Connections between Arduino and peripherals:

The first thing we have to do is look for a power supply for the Arduino at 5v. In this case it has been chosen to convert the 12v that reach the OBD connector using a transformer of small dimensions that fits inside the OBD connector, below the PCB:

![Texto alternativo](media/obdsupply.jpg "OBD & supply").

For the OBD part we will use 2 digital pins (2-Rx and 3-Tx) and for communication with the other 3 digital pins (4-Enable, 5- Clock, 6-Data).

![Texto alternativo](media/obdArdu.jpg "OBD, supply & Arduino").

In case of having a FIS-DIS instrument panel, we need to make the connection between the Arduino and the panel. To do this, we must locate the 3 signals of the connector that goes to the radio (ENA, CLOCK and ENABLE) and connect them to the pins of the Arduino assigned.

Note: Doing this will lose the functionality of displaying the radio information in the box.

If a FIS-DIS box is not available in an original way or have been retrofitted, connections must be made from the green connector T32 in the box to the Arduino.

Now we only have to load the program into the Arduino and connect the device to the OBD connector of the car. At this point we should be able to connect to the units and start extracting information from them, to do some testing we can see the results through the Arduino IDE serial monitor.


## Contribute to the project:

If this information has been useful to you or you have found it interesting you can contribute to the project to help in the development of improvements or new functionalities through making an economic donation through PayPal to the account samueliban@gmail.com.


## Future improvements:

* Possibility to change the menus using the cruise control handle, clutch or brake.
* Improve the usability of the instrument panel display.
* Script for generating specific labels for each unit.
* Needle movement at startup or Needle stagging / sweep through KWP1281.
* Creation of encapsulated PCB prototype.


## Acknowledgments & Contact:

This project would not have been possible without the help of:

* Alexander - http://grauonline.de/wordpress/?p=74
* Tomáš Kováčik - http://kovo-blog.blogspot.com.es/2013/11/audi-fis-3-line-protocol.html
* Nefmoto comunity & the awesome ME7Logger - http://www.nefmoto.com

You can get in touch for any query through the Issue List or send me an email to samueliban@gmail.com.
