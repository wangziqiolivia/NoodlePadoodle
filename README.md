# Noodle Padoodle 
An electric Guzheng (traditional Chinese instrument with 21 strings) that you can play in dorms with headphones, carry around easily and tune effortlessly.    
## My motivation  
I'm an international student from China studying in the United States. I've been playing Guzheng since five years old, but because it's too big, I couldn't bring it with me when I came to the US. Thus, I want to build my own Guzheng that is more portable and can be played in dorms :-) Also, it would be super cool to make an electric version of a traditional Chinese instrument!!
## Pictures
### My Guzheng
#### Demo video  
https://github.com/user-attachments/assets/ef05cc21-928e-400d-a834-1ba05163664a
#### Some pictures
<img width="1280" height="1707" alt="afc1a4ef0554323734a6b1be0fc4f1ca" src="https://github.com/user-attachments/assets/3ad95d43-76ed-458b-892f-898131cb6be8" />
<img width="1707" height="1280" alt="becd36a59b36f32def657f0ffad531c0" src="https://github.com/user-attachments/assets/c99d6d76-0a61-494c-8721-79032cfb4e47" />  

Wiring
<img width="1405" height="1337" alt="HX711toESP32" src="https://github.com/user-attachments/assets/be6ed44a-123b-4604-aa4a-f5ecb7c2f762" />
<img width="4694" height="1873" alt="wiringCompleteGuide" src="https://github.com/user-attachments/assets/788fc026-27f0-4213-a5ec-9c423f0c71fe" />


### The traditional Guzheng 
<img width="1239" height="804" alt="image" src="https://github.com/user-attachments/assets/f012b191-0185-4fce-a4dc-c530b6d32f5b" /> 

## Features  
- Electric!! Super cool right?
- Small size compared with traditional Guzheng 40cm * 163cm * 9cm
- Does not disturb others: the sound is generated in the laptop and users can use a headphone
- Easy to tune and change the key: all can be done on the laptop
- Durable, robust, less likely to be influenced by humidity
- Great for music production: can record the notes played easily in the future
  
## How it works
Every string is connected to a 5kg load cell that senses the change in pull force of the string. The sensor is then connected to a HX711 to amplify the signal. Then an ESP32 board connects the HX711 to a laptop.  
Then codes in my laptop will analyze the signals and produce sounds that matches the operation of the player, considering the strength and techniques applied. The program can differentiate between left hand operations and right hand operations with only ONE FORCE SENSOR!
  

## Materials
|Name | Quanity| Link | Total price (USD) |
|-----|--------| -----|-------------------|
|ESP32 |2| https://amazon.com/gp/product/B0DNYR973V/ref=ox_sc_act_title_2?smid=A1VTL661FOEJB1&psc=1|15.99|
|HX711 +load cell|2*1|https://amazon.com/dp/B09K7G3477?ref=ppx_yo2ov_dt_b_fed_asin_title|9.99|
|HX711 + load cell | 4*5 | https://amazon.com/gp/product/B09VYSHW16/ref=ewc_pr_img_3?smid=A27MCP768Z76HQ&psc=1| 63.96|
|Guzheng strings|21*1| [https://www.amazon.com/gp/product/B093DND9V5/ref=ox_sc_act_title_3?smid=A3E4HJMK9QEY9N&psc=1 ](https://www.amazon.com/dp/B09BVVC9LP?ref=ppx_yo2ov_dt_b_fed_asin_title)|15.55|
|F-F dupont wires| 40*3 | https://www.amazon.com/gp/product/B07GCY6CH7/ref=sw_img_1?smid=&th=1| 7.98|
|F-M dupont wires| 40*1 | https://www.amazon.com/gp/product/B0BRTHR2RL/ref=ewc_pr_img_1?smid=A2XLLJ8HYD6SMA&th=1|3.99|
|breadboard kit | 1| https://www.amazon.com/gp/product/B0GVD11LSS/ref=ewc_pr_img_2?smid=AKJTXD7UD2AFM&psc=1| 7.29|
|700 mm V-slot Aluminum extrusion profile |8*1|https://www.amazon.com/gp/product/B0DY7G17BK/ref=ewc_pr_img_4?smid=A2DGLVBBDAJTOF&psc=1|39.99|
|sodering kit| 1| https://www.amazon.com/gp/product/B087767KNW/ref=ewc_pr_img_2?smid=A19YGYI63H9AEE&th=1| 12.74 |

### Notes
I later found out that one ESP32 was enough and also I only used 17 strings because 4 load cells were lost unfortunately.

## Guide for my repository
Check the AllStringsAdaptiveFinished.py for the final, adaptive code. It's an eight-string verion because I haven't resolved the bugs for the other nine strings yet, but it's very easy to modify the code into a 21-string version.  
Guzheng sound samples are in the 21StringsSample folder, and the program uses wav format audio. The M_E folder has sounds of many different techiniques played on the Guzheng.  

## Acknowledgements
A big thank you to my mom in China for recording all the Guzheng sounds!!!  
### Wiring & interfacing tutorials for esp32  
- https://randomnerdtutorials.com/esp32-pinout-reference-gpios/  
- https://randomnerdtutorials.com/esp32-load-cell-hx711/
- https://lastminuteengineers.com/esp32-pinout-reference/#esp32-pinout
