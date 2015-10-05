#!/usr/bin/python3

#Code adapted with thanks to Alex Eames at raspi.tv

import time # Want to sleep 
import RPi.GPIO as GPIO # Use the python GPIO handler for RPi
GPIO.setmode(GPIO.BCM) # Use the Broadcom GPIO pin designations

def my_callback(channel): # When a button is pressed, report which channel and flash led
    print("Falling edge detected on port %s" % channel)
    flash_led(channel)

def flash_led(channel):
    if (channel == 16):
        GPIO.output(6, 1) # flash red on GPIO6
        time.sleep(0.2)
        GPIO.output(6, 0)
    elif (channel == 19):
        GPIO.output(12, 1) # flash green on GPIO12
        time.sleep(0.2)
        GPIO.output(12, 0)
    elif (channel == 20):
        GPIO.output(5, 1)  # flash blue on GPIO5
        time.sleep(0.2)
        GPIO.output(5, 0)
    elif (channel == 26):
        GPIO.output(12, 1)  # flash them all
        GPIO.output(6, 1)
        GPIO.output(5, 1)
        time.sleep(0.2)
        GPIO.output(12, 0)
        GPIO.output(6, 0)
        GPIO.output(5, 0)

for channel in [5, 6, 12]:
    GPIO.setup(channel, GPIO.OUT)
    GPIO.output(channel, 0)

for channel in [16, 19, 20, 26]: # Buttons are on GPIO16, 19, 20 & 26 left to right
    GPIO.setup(channel, GPIO.IN, pull_up_down=GPIO.PUD_UP) # Set buttons as input with internal pull-up
    GPIO.add_event_detect(channel, GPIO.FALLING, callback=my_callback, bouncetime=500) # Event detect a button press, falling edge - switched to ground
                                                                                       # On detect, do actions in my_callback function, switch debounce of 500ms


try:
    while True:   # loop forever
        print("Waiting for something to happen.....")
        time.sleep(1)

except KeyboardInterrupt:
    GPIO.cleanup()   # Clean up when exiting with Ctrl-C
    print("Cleaning up!")

        
