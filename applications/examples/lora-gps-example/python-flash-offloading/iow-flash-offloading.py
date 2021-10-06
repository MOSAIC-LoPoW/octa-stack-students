#!/usr/bin/python

import sys
import serial
import pickle
import csv
import datetime
from Parser import Parser

################
# SERIAL SETUP #
################
if len(sys.argv) < 2:
    print("!No serial port specified!")
    print("You need to specifiy the serial port of the shimmer you wish to connect to")
    print("example (windows):")
    print("  python iow-flash-offloading.py COM1")
    print(" or (linux & mac OS):")
    print("  python iow-flash-offloading.py /dev/ttyUSB0")
    exit()
else:
    print("Opening serial port {} with baudrate 115200".format(str(sys.argv[1])))
    print("Press OCTA BTN2 two times in 5 seconds to start the offloading on the device")
    print("If the green light near the battery connector is on, the device is taking a measurement and will only respond to presses afterwards")
    ser = serial.Serial(sys.argv[1], 115200)
    ser.flushInput()

line = ser.readline()
while "*****OFFLOADING FLASH*****" not in str(line):
    line = ser.readline() # read a '\n' terminated line
ser.readline() #stopping measurement timer msg

# print amount of pages that will be offloaded
line = ser.readline() # offloading a total of x pages
amountofpages = int(''.join(filter(str.isdigit, str(line)))) # parse number out of this string
print("Starting the offloading of {} pages".format(amountofpages))
print("This will take a while")

##############
# RAW DATA #
##############
flash_pages = []
while "Flash offloaded" not in str(line):
    line = ser.readline()
    #add only valid lines to flash_pages
    if "[INF] Flash page" in str(line): 
        flash_pages.append(str(line))

filename = "flash_dump_raw-" + datetime.datetime.now().strftime("%Y%m%d-%H%M%S") + ".txt"
# raw data to file
with open(filename, 'wb') as pickle_out:
    pickle.dump(flash_pages, pickle_out)
pickle_out.close()
print("Raw flash offloaded to {}".format(filename))  

##############
# PARSE DATA #
##############
parser = Parser()
parsedlist = []

for page_string in flash_pages:
    # parse hex flash data from serial string (split on last space)
    page_hex_raw = page_string.rsplit(' ', 1)
    # remove \r\n from end to get hex string
    page_hex = page_hex_raw[1].replace('\\r\\n\'','')
    # create new result, will be appended to list
    parsed = {}
    try:
        parsed= parser.parse(page_hex)
        parsedlist.append(parsed)
    except:
        print("parsing failed for {}".format(str(page_string)))

filename = "iow-watersensor-parsed-" + datetime.datetime.now().strftime("%Y%m%d-%H%M%S") + ".csv"
with open(filename, 'w', encoding='utf8', newline='') as output_file:
    fc = csv.DictWriter(output_file, delimiter=',', quoting=csv.QUOTE_NONE, fieldnames=parsedlist[0].keys())
    fc.writeheader()
    fc.writerows(parsedlist)

print("Parsed data offloaded to {}".format(filename))