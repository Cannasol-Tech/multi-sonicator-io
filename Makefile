# Multi-Sonicator I/O Controller Makefile


#  Make Targets 

## Firmware Related Make Targets 

build: 
	# TODO: Implement make target to build the ATMEGA32A Firmware for loading on the device or emulator 

clean: 
	# TODO: Implement make target to clean the ATMEGA32A Firmware build - This should wipe all previous builds and generated files so we ensure a clean new build 

upload: 
	# TODO: Implement make target to upload the ATMEGA32A Firmware to the device or emulator 
		# - This make target should evoke a cli that will asks the user if they want to upload the firmware to the device or the emulator 

build-emulator: 
	# TODO: Implement make target to build the emulator docker image 
	    # - IMPORTANT:  This make target should result with a running simulavr docker container with the latest ATMEGA32A firmware loaded 
		# - This make target should output the docker container information 

monitor-emulator: 
	# TODO:  This make target should evoke a cli (python script) that interacts with the emulator the same way we interact for hil-sandbox mode

monitor-device: 
	# TODO:  This make target should evoke a cli (python script) that monitors the serial output coming from the Arduino Test Wrapper / Harness	

upload-to-device: 
   # TODO: Upload the ATMEGA32A Firmware to the device using the arduino as an ISP, This should first check to see if the Arduino as ISP script is running on the arduino and if it is - it should tell the user and ask if the user wants to continue to upload again or not 

upload-to-emulator: 
   # TODO: Upload the ATMEGA32A Firmware to the emulator using the emulators

upload-harness: 
	# TODO: This make target should upload the ardunio test harness to the Arduino that is connected to the computer acting as a middle man for the ATMEGA32A chip

hardware-sandboxz: 
    # TODO:  This make target should evoke a cli (python script) that interacts with the user until they reach a point to being connected to the hardware in a "sandbox" mode 
	    # - The sandbox mode should consist of the following: 
		#     1. The user should be able to send commands to the Arduino Test Wrapper / Harness
		# 	  2. The python cli should be in a continuous loop with the user able to send "monitor_on" or "monitor_off" commands 
		#     3. The "monitor_on" and "monitor_off" commands should be able to turn on and off the monitoring of the AtMega32A chip .
		#     4. When Monitoring is on - the python cli should automatically print the state of all of the monitored pins to the CLI every second 
		#     5. The python cli should have a "quit" command that will exit the sandbox mode and return the user to the terminal 
		#     6. The python cli should have a "help" command that will print the list of available commands to the user 
		#     7. The python cli should have a "status" command that will print the current state of the monitored pins to the CLI 
		#     8. The python cli should have a "set" command that will allow the user to set the state of the output pins on the AtMega32A chip 
		#     9. The python cli should have a "reset" command that will reset the AtMega32A chip 
		#    10. The python cli should have a "upload" command that will upload the ATMEGA32A firmware to the device or emulator 
		#    11. The python cli should have a "build" command that will build the ATMEGA32A firmware for loading on the device or emulator 
		#    12. The python cli should have a "clean" command that will clean the ATMEGA32A firmware build - This should wipe all previous builds and generated files so we ensure a clean new build 
		#    13. When monitoring is turned off -- the CLI should notify the user and suggest turning it on every 15 seconds ( implemented with a count in the main infinite loop )


## Testing Make Targets 
test: 
   # TODO: Run all of the tests for this project

test-unit: 
   # TODO: Run all of the unit tests for this project

test-emulation:
   # TODO: Run all of the emulation tests for this project
       # NOTE:  This should be the bdd tests that are written to te st the project requirements ( These are supposed to be able to be ran against the emulator or the device hardware )

test-hardware:
	# TODO: Run all of the hardware-in-the-loop tests for this project
	#  NOTE:  This should be the bdd tests that are written to test the project requirements ( These are supposed to be able to be ran against the emulator or the device hardware )

