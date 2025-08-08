#!/usr/bin/env python3
"""
Simple script to send a string over serial to Arduino at 115200 baud.
Usage: python3 serial-simple.py "your text here"
"""

import serial
import serial.tools.list_ports
import time
import sys

def find_arduino_port():
    """Find the Arduino USB serial port."""
    ports = serial.tools.list_ports.comports()

    print("Available ports:")
    for port in ports:
        print(f"  {port.device}: {port.description}")

    # Look for common Arduino identifiers
    for port in ports:
        # Look for common Arduino identifiers
        if any(identifier in port.description.lower() for identifier in ['arduino', 'usb serial', 'usb-serial', 'ch340', 'cp210', 'ftdi']):
            print(f"Found Arduino on port: {port.device}")
            return port.device

    # On Raspberry Pi, also check common port names directly
    common_pi_ports = ['/dev/ttyACM0', '/dev/ttyACM1', '/dev/ttyUSB0', '/dev/ttyUSB1']
    for port_name in common_pi_ports:
        try:
            # Try to open the port to see if it's available
            test_ser = serial.Serial(port_name, 115200, timeout=0.1)
            test_ser.close()
            print(f"Found available port: {port_name}")
            return port_name
        except:
            continue

    print("No Arduino found. Please check USB connection.")
    return None

def send_string(text_to_send):
    """Send a string to the Arduino."""
    try:
        # Find Arduino port
        arduino_port = find_arduino_port()
        if not arduino_port:
            print("No Arduino found. Please check USB connection.")
            return

        print(f"Attempting to connect to {arduino_port}...")

        # Open serial connection
        ser = serial.Serial()
        ser.baudrate = 115200
        ser.port = arduino_port
        ser.writeTimeout = 0.2
        ser.timeout = 1  # Add timeout for reading

        # Wait for connection to stabilize
        time.sleep(1)

        try:
            ser.open()
            print("Serial connection opened successfully")
        except Exception as e:
            print(f"Failed to open serial connection: {e}")
            return


        time.sleep(2)
        # Flush existing serial buffers
        ser.reset_input_buffer()
        ser.reset_output_buffer()
        print("Serial buffers flushed")

        # Send the string - match Arduino IDE Serial Monitor exactly
        message = text_to_send + '\r\n'  # Add carriage return and newline
        print(f"Sending: '{message.strip()}'")

        try:
            ser.write(message.encode('ascii'))
            ser.flush()  # Re-enable flush to ensure transmission
            print("Message written to serial port")
        except Exception as e:
            print(f"Failed to write message: {e}")
            return

        print("Message sent successfully!")

        # Wait for Arduino response
        print("Waiting for Arduino response...")
        time.sleep(2)  # Give Arduino time to process

        # Try to read response
        try:
            if ser.in_waiting > 0:
                response = ser.readline().decode('ascii').strip()
                if response:
                    print(f"Arduino response: {response}")
                else:
                    print("Empty response received from Arduino")
            else:
                print("No response received from Arduino (no data in buffer)")
        except Exception as e:
            print(f"Error reading response: {e}")

        # Close connection
        ser.close()
        print("Serial connection closed")

    except serial.SerialException as e:
        print(f"Serial error: {e}")
    except Exception as e:
        print(f"Error: {e}")
        import traceback
        traceback.print_exc()

def main():
    """Main function to handle command line arguments."""
    if len(sys.argv) != 2:
        print("Usage: python3 serial-simple.py \"your text here\"")
        print("Example: python3 serial-simple.py \"Hello Arduino!\"")
        sys.exit(1)

    # Check if running on Raspberry Pi
    try:
        with open('/proc/cpuinfo', 'r') as f:
            if 'Raspberry Pi' in f.read():
                print("Running on Raspberry Pi")
                print("Note: If you get permission errors, run: sudo usermod -a -G dialout $USER")
    except:
        pass

    text_to_send = sys.argv[1]
    send_string(text_to_send)

if __name__ == "__main__":
    main()
