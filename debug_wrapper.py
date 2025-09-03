#!/usr/bin/env python3
"""
Quick HIL Wrapper Debug Tool
"""

import serial
import time

def debug_wrapper():
    port = "/dev/cu.usbmodem2101"
    
    try:
        ser = serial.Serial(port, 115200, timeout=2)
        time.sleep(2)
        
        # Clear startup messages
        while ser.in_waiting > 0:
            line = ser.readline().decode().strip()
            print(f"Startup: {line}")
        
        print("Sending HELP command...")
        ser.write(b"HELP\n")
        time.sleep(1)
        
        # Read all help output
        while ser.in_waiting > 0:
            line = ser.readline().decode().strip()
            print(line)
            
        ser.close()
        
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    debug_wrapper()
