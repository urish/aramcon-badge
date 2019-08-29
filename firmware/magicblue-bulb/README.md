# MagicBlue Bulb Driver

## Installation

Copy all the files and [badgeio.py](../lib/badgeio.py) to your badge's USB Drive

## Usage

The code will automatically look for a light bulb when the badge is powered on.
Once the badge has connected to the bulb, use the keys to control the light:

* Left key → Red
* Middle key → Green
* Right key → Blue

You can also use any combination of these keys to create different colors.

The demo code uses the neopixels to indicate the status:
* Blue: Searching / connecting to the bulb.
* No light: Connected to the bulb, press any key to set its color.
* Red: Bulb not found / connection error.

Enjoy!

## Bulb protocol

You can find information about the bulb protocol in the following blog post:

https://medium.com/@urish/reverse-engineering-a-bluetooth-lightbulb-56580fcb7546

