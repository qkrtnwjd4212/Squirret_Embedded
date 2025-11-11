import serial
import csv
import datetime

def log_fsr_to_csv(port, baudrate, output_file):
    try:
        ser = serial.Serial(port, baudrate, timeout=1)
        print(f"Connected to {port} at {baudrate} baud.")

        with open(output_file, mode='w', newline='') as csvfile:
            csv_writer = csv.writer(csvfile)
            csv_writer.writerow(["timestamp", "ratio1", "ratio2", "ratio3", "ratio4", "ratio5", "ratio6"])

            print("Logging FSR data to CSV (Ctrl + C to stop)...")

            while True:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if not line:
                    continue

                # ESP32 출력 중 "Ratio:" 문자열이 포함된 줄만 처리
                if "Ratio:" in line:
                    # 예시: "FSR 1 (GPIO36): 123 | Voltage: 0.45 V | Ratio: 12.34 %"
                    parts = line.split("Ratio:")[-1].strip().replace("%", "")
                    try:
                        ratio = float(parts)
                        ratios.append(ratio)
                    except:
                        continue

                    # ratio1~ratio6 모두 수집되면 한 줄 저장
                    if len(ratios) == 6:
                        timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                        csv_writer.writerow([timestamp] + ratios)
                        csvfile.flush()
                        print(f"{timestamp} | {ratios}")
                        ratios.clear()

    except serial.SerialException as e:
        print(f"Serial connection error: {e}")
    except KeyboardInterrupt:
        print("Logging stopped by user.")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()
            print("Serial port closed.")

# -------- 사용자 설정 --------
serial_port = 'COM4'   # Windows: COM3/COM4, Mac/Linux: /dev/ttyUSB0
baud_rate = 115200
output_csv_file = 'fsr_data_log.csv'

ratios = []
log_fsr_to_csv(serial_port, baud_rate, output_csv_file)
