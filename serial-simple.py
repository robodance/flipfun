#!/usr/bin/env python3
"""
Simple script to send a string over serial to Arduino at 115200 baud.
"""

import serial
import serial.tools.list_ports
import time

def find_arduino_port():
    """Find the Arduino USB serial port."""
    ports = serial.tools.list_ports.comports()

    for port in ports:
        # Look for common Arduino identifiers
        if any(identifier in port.description.lower() for identifier in ['arduino', 'usb serial', 'usb-serial', 'ch340', 'cp210']):
            print(f"Found Arduino on port: {port.device}")
            return port.device

    # If no Arduino found, list all available ports
    print("Arduino not found. Available ports:")
    for port in ports:
        print(f"  {port.device}: {port.description}")

    return None

def send_string():
    """Send a string to the Arduino."""
    try:
        # Find Arduino port
        arduino_port = find_arduino_port()
        if not arduino_port:
            print("No Arduino found. Please check USB connection.")
            return

        # Open serial connection
        ser = serial.Serial()
        ser.baudrate = 115200
        ser.port = arduino_port
        ser.writeTimeout = 0.2
        ser.timeout = 1  # Add timeout for reading

        # Wait for connection to stabilize
        time.sleep(1)

        ser.open()

        # Flush existing serial buffers
        ser.reset_input_buffer()
        ser.reset_output_buffer()

        # Send the string - match Arduino IDE Serial Monitor exactly
        message = 'foo\r\n'  # Hardcoded message
        print(f"Sending: {message.strip()}")

        ser.write(message.encode('ascii'))
        ser.flush()  # Re-enable flush to ensure transmission

        print("Message sent successfully!")

        # Wait for Arduino response
        print("Waiting for Arduino response...")
        time.sleep(2)  # Give Arduino time to process

        # Try to read response
        try:
            response = ser.readline().decode('ascii').strip()
            if response:
                print(f"Arduino response: {response}")
            else:
                print("No response received from Arduino")
        except:
            print("No response received from Arduino")

        # Close connection
        ser.close()

    except serial.SerialException as e:
        print(f"Serial error: {e}")
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    send_string()
