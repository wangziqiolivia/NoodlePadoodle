# Noodle Padoodle 
An electric Guzheng (traditional Chinese instrument) that you can play in dorms with headphones, carry around easily and tune effortlessly.    
## Picture  
## Demo link
## Features
- Small size: 40cm * 90cm * 2cm compared with traditional Guzheng 40cm * 163cm * 9cm
- Does not disturb others: the sound is generated in the laptop, and users can use a headphone
- Easy to tune and change the key: all can be done on the laptop
- Durable, robust, less likely to be influenced by humidity
- Great for music production: can record the notes played easily in the future
## How it works
Every string is connected to a 5kg load cell that senses the change in pull force of the string. The sensor is then connected to a HX711 to amplify the signal. Then an ESP32 board connects the HX711 to a laptop.  
Then codes in my laptop will analyze the signals and produce sounds that matches the operation of the player, considering the strength and techniques applied.
## Acknowledgements
