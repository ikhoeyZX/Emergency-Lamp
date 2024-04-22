# Undens Lampu Emergency

This repository contains the source code for build a Emergency lamp based STM8 or ESP8266.

* Note: ESP8266 version are __not recommended__ but you can build it, but beware it sometimes crash and heating up when using stepup ic at low battery level (below 3V)


# How to use

There is 2 build exist (ESP8266 and STM8) and for now only 1 layer since it cheaper and easy to build than 2 layes or more.

each folder contains how to build it. You need Android IDE to program it (in STM8 case, use linux because win8 or newer doesn't really work sduino for now)

* Note : For stepup ic you can find by self, depend of budget, because MOSFET out only act as "switch" so you need stepup to make sure it can out high voltage to light a leds (5v are common)
* MOSFET should be withstand around ~6A continues with heatsink, so if you using stepup and output 5v it should be around ~4A Max in input side
* In my case only use 1A at MOSFET output and 4400mA battery. it can run without issue around 4H with 20 leds (~40mA/leds) in 12 places at home (No need too bright, bright as candle are enough or around 1W/room)

## Finished device:

|                      **Main device**                          |                                      **A Step up ic MT3608**                                   |
| :-----------------------------------------------------------: | :--------------------------------------------------------------------------------------------: |
| ![Main device](./IMG_Preview/PREV_1.jpg)                      | ![Step up ic](./IMG_Preview/PREV_2.jpg)                                                        |

|               **Main device when running**                    |                               **A led that used, smd led 3v with 5v input**                    |
| :-----------------------------------------------------------: | :--------------------------------------------------------------------------------------------: |
| ![Main device when running](./IMG_Preview/PREV_3.jpg)         | ![A led that used for emergency](./IMG_Preview/PREV_4.jpg) |

