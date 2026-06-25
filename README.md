# Noodle Padoodle 
An electric Guzheng (traditional Chinese instrument with 21 strings) that you can play in dorms with headphones, carry around easily and tune effortlessly.    
## My motivation  
I'm an international student from China studying in the United States. I've been playing Guzheng since five years old, but because it's too big, I couldn't bring it with me when I came to the US. Thus, I want to build my own Guzheng that is more portable and can be played in dorms :-) Also, it would be super cool to make an electric version of a traditional Chinese instrument!!
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
### Materials
|Name | Quanity| Link | Total price (USD) |
|-----|--------| -----|-------------------|
|ESP32 |2|amazon.com/gp/product/B0DNYR973V/ref=ox_sc_act_title_2?smid=A1VTL661FOEJB1&psc=1|15.99|
|HX711 |21|https://www.amazon.com/gp/product/B09K7G3477/ref=ox_sc_act_title_5?smid=A27MCP768Z76HQ&psc=1|109.89|
|Guitar Strings|6*4| https://www.amazon.com/gp/product/B093DND9V5/ref=ox_sc_act_title_3?smid=A3E4HJMK9QEY9N&psc=1 |39.96|
|F-F dupont wires| 40*3 | https://www.amazon.com/gp/product/B07GCY6CH7/ref=sw_img_1?smid=&th=1| 7.98|
|80cm T-slot Aluminum extrusion profile |2|
|40cm T-slot Aluminum extrusion profile |3|
|corner brakets|||
|U-shape pulley|21|
|eye bolts|42|
|screws
|sodering kit| 1| https://www.amazon.com/gp/product/B087767KNW/ref=ewc_pr_img_2?smid=A19YGYI63H9AEE&th=1| 12.74 |
## Acknowledgements
