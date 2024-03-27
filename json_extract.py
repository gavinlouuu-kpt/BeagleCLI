import serial
import serial.tools.list_ports
import time  # Import the time module for delays

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

if __name__ == "__main__":
    target_com = "/dev/cu.wchusbserial220"
    path_0 = "/E4:65:B8:80:22:A4/t_0"
    manager = SerialManager(target_com)
    manager.open_com()

    # Read the directory list
    data = manager.read_data_list()
    if "Command executed" in data:
        print("Ready to open file.")

    # Assuming you want to proceed to open a file only if the directory listing was successful
    data = manager.open_file(path_0)
    if "Command executed" in data:
        print("File opened successfully.")

    manager.close_com()
