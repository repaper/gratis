#!/usr/bin/python3
from time import sleep
import RPi.GPIO as GPIO # Use the RPi.GPIO python module - need to run as root
GPIO.setmode(GPIO.BCM) # Use Broadcom GPIO pin designations
GPIO.setwarnings(False) # Disable warning messages, because they annoy me!

gpio_in = [16, 19, 20, 26] # Buttons on GPIO16, 19, 20, 26 left to right
gpio_out = [6, 12, 5] # Led driven Red, Green, Blue on GPIO 6, 12, 5 respectively

for x in gpio_in: # Set up four button channels as inputs with internal pull-up
    GPIO.setup(x, GPIO.IN, pull_up_down=GPIO.PUD_UP)

for x in gpio_out: # Set up three led RGB channels as outputs
    GPIO.setup(x, GPIO.OUT)

def my_callback(channel):
    print('This is an edge event callback function!') # These are all for information only
    print('Edge detected on channel %s'%channel)    # Normally put your code here for what 
    print('This is running in a different thread to your main program') # you want to happen when a button is pressed

GPIO.add_event_detect(16, GPIO.FALLING, callback=my_callback, bouncetime=500) # Set up button pressed
GPIO.add_event_detect(19, GPIO.FALLING, callback=my_callback, bouncetime=500) # event detection for
GPIO.add_event_detect(20, GPIO.FALLING, callback=my_callback, bouncetime=500) # all four buttons
GPIO.add_event_detect(26, GPIO.FALLING, callback=my_callback, bouncetime=500) #

try:
    while True: # Loop
        for x in gpio_out: #Rotate through Red, Green, Blue
            p = GPIO.PWM(x, 100) #Set up pwm @ 100Hz
            p.start(0)
            for dc in range(0, 101, 5): # Ramp up 0 to 100 in steps of 5  
                p.ChangeDutyCycle(dc) # Change duty cycle
                sleep(0.05)     # Wait 50 ms
            for dc in range(100, -1, -5): # Ramp down 100 to 0 in steps of 5
                p.ChangeDutyCycle(dc)  # Change duty cycle
                sleep(0.05)  # Wait 50ms
            p.stop(0)   

except KeyboardInterrupt: # Exit by typing CTRL-C
    print ("You hit CTRL-C")
    GPIO.cleanup()
    
    
