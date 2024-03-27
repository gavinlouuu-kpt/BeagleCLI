import serial
import serial.tools.list_ports
import time  # Import the time module for delays
import os
import re
import json

class JSONDataManager:
    @staticmethod
    def extract_json_content(file_data):
        content_pattern = r"Contents of the file:\n(.*?)Command executed"
        content_match = re.search(content_pattern, file_data, re.DOTALL)
        if content_match:
            return content_match.group(1).strip()
        return None

    @staticmethod
    def save_json_to_file(json_data, path):
        save_directory = "data_extract/"
        if not os.path.exists(save_directory):
            os.makedirs(save_directory)
        filename = path.replace(":", "").replace("/", "_") + ".json"
        full_path = os.path.join(save_directory, filename)
        with open(full_path, 'w') as json_file:
            json.dump(json_data, json_file, indent=4)
        return full_path

class SerialManager:
    def __init__(self, target_com, baudrate=115200):
        self.target_com = target_com
        self.baudrate = baudrate
        self.ser = None

    def list_serial_ports(self):
        return [port.device for port in serial.tools.list_ports.comports()]

    def open_com(self):
        if self.target_com in self.list_serial_ports():
            try:
                self.ser = serial.Serial(self.target_com, self.baudrate, timeout=1)
                print(f"{self.target_com} has been opened successfully.")
            except serial.SerialException as e:
                print(f"Failed to open {self.target_com}: {e}")
        else:
            print(f"{self.target_com} is not available.")

    def close_com(self):
        if self.ser:
            self.ser.close()
            print(f"{self.target_com} has been closed successfully.")

    def read_data_list(self):
        self.ser.flushInput()
        self.ser.write(b"ls\n")
        time.sleep(1)  # Delay to allow data to be fully sent and responses to be prepared
        data = ""
        while True:
            line = self.ser.readline().decode("utf-8").strip()
            if line:
                data += line + "\n"
            else:
                break
        print(data)
        return data

    def open_file(self, path):
        self.ser.flushInput()
        self.ser.write(b"open\n")
        time.sleep(1)  # Delay after sending the command
        response = ""
        while True:
            line = self.ser.readline().decode('utf-8').strip()
            response += line + "\n"
            if "Enter the file name to open:" in line:
                self.ser.flushInput()  # Optional: Clear buffer before sending the next command
                self.ser.write(path.encode() + b"\n")
                time.sleep(1)  # Delay after sending the file path
            elif "Command executed" in line:
                break

        print(response)
        return response
    
class DataProcessor:
    @staticmethod
    def extract_mac_address(message):
        """
        Extracts and returns the MAC address from a given message string.
    
        Parameters:
        - message (str): The message string containing the MAC address.
    
        Returns:
        - str: The extracted MAC address or a message indicating that no MAC address was found.
        """
        # Define the regex pattern to find the MAC address
        mac_address_pattern = r"DIR : /([0-9A-Fa-f]{2}:[0-9A-Fa-f]{2}:[0-9A-Fa-f]{2}:[0-9A-Fa-f]{2}:[0-9A-Fa-f]{2}:[0-9A-Fa-f]{2})"
    
        # Search for the pattern in the message
        match = re.search(mac_address_pattern, message)
    
        if match:
            return match.group(1)  # The first capturing group contains the MAC address
        else:
            return "No MAC Address found."    # Implementation remains the same

    @staticmethod
    def path_constructor(data):
        # use the list from read_data_list() to construct file paths to open
        # only construct path that contains "t_"
        # i.e., /E4:65:B8:80:22:A4/t_0 and /E4:65:B8:80:22:A4/t_1 ...
        mac_addr = DataProcessor.extract_mac_address(data)
        if mac_addr == "No MAC Address found.":
            return []
        
        paths = []
        for line in data.split("\n"):
            if "FILE: /t_" in line: # Ensure it only matches file lines with 't_'
                file_name = line.split(": /")[1] # Extract the file name
                paths.append(f"/{mac_addr}/{file_name}") # Construct the full path
                
        return paths

if __name__ == "__main__":
    target_com = "/dev/cu.wchusbserial220"
    path_0 = "/E4:65:B8:80:22:A4/t_0"
    manager = SerialManager(target_com)
    manager.open_com()

    # Read the directory list
    data = manager.read_data_list()
    if "Command executed" in data:
        print("Ready to open file.")

    paths = DataProcessor.path_constructor(data)
    print("Paths to open:", paths)

    # Loop through the paths and open the files
    for path in paths:
        file_data = manager.open_file(path)
        if "Command executed" in file_data:
            print(f"File opened successfully: {path}")

            # Extracting the JSON content
            json_content = JSONDataManager.extract_json_content(file_data)
            if json_content:
                try:
                    # Attempt to parse the JSON content
                    json_data = json.loads(json_content)
                    # Save the parsed JSON data to a file
                    saved_path = JSONDataManager.save_json_to_file(json_data, path)
                    print(f"File saved successfully: {saved_path}")        
                except json.JSONDecodeError:
                    print(f"Failed to parse JSON content from the file: {path}")
            else:
                print(f"No content found to save for file: {path}")
        else:
            print(f"Failed to open file: {path}")

    # data = manager.open_file(path_0)
    # if "Command executed" in data:
    #     print("File opened successfully.")

    manager.close_com()
