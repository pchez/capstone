from __future__ import print_function
import csv
import sys
import struct


# Converts raw bytes to floating point timestamp - TODO
def getTimestamp(timestamp_raw):
    return 0.0

# Converts raw bytes to floating point for each sensor's axes.
def getMotion(accel_raw, gyro_raw, mag_raw):
    accel, gyro, mag = {}, {}, {}
    MAX = 1024

    # Convert from 2-byte hex value to signed short, for each sensor
    for axis in accel_raw.keys():
        accel_string = accel_raw[axis][0] + accel_raw[axis][1]
        accel[axis] = struct.unpack('<h', accel_string.decode('hex'))[0] / MAX
    for axis in gyro_raw.keys():
        gyro_string = gyro_raw[axis][0] + gyro_raw[axis][1]
        gyro[axis] = struct.unpack('<h', gyro_string.decode('hex'))[0] / MAX
    for axis in mag_raw.keys():
        mag_string = mag_raw[axis][0] + mag_raw[axis][1]
        mag[axis] = struct.unpack('<h', mag_string.decode('hex'))[0] / MAX
    
    return accel, gyro, mag


if __name__ == "__main__":
    if len(sys.argv) < 4:
        print('Usage: python convert_data.py [inputfile] [outputfile] [label]')
        exit(-1)

    # Arguments:
    # python convert_data.py [inputfile] [label]
    #       - inputfile: A text file generated. Output file is edited automatically here
    #       - outputfile: A text file with processed data
    #       - label: Defined constant representing a position configuration
    #         This is passed in from the C program through the shell and is a predefined config

    input_file = sys.argv[1]
    output_file = sys.argv[2]
    label = int(sys.argv[3])

    # Open raw file
    input_data = []
    with open(input_file, 'rb') as input_raw:
        reader = csv.reader(input_raw, delimiter=' ')

        for row in reader:
            input_data.append(row)

    accel_raw, gyro_raw, mag_raw = {}, {}, {}
    with open(output_file, 'awb') as output:
        output_writer = csv.writer(output, delimiter=' ')
        
        for row in input_data:
                
            # Organize raw data
            timestamp_raw = row[0:2]
            accel_raw['x'] = row[2:4]
            accel_raw['y'] = row[4:6]
            accel_raw['z'] = row[6:8]
            gyro_raw['x'] = row[8:10]
            gyro_raw['y'] = row[10:12]
            gyro_raw['z'] = row[12:14]
            mag_raw['x'] = row[14:16]
            mag_raw['y'] = row[16:18]
            mag_raw['z'] = row[18:20]

            # Compute actual values
            timestamp = getTimestamp(timestamp_raw)
            accel, gyro, mag = getMotion(accel_raw, gyro_raw, mag_raw)

            # Write to training file, in the format as provided in xor.data example for fann
            # Training file will vary depending on what mode / what we're training for. TODO
            #
            # -- file format --
            # num_training_pairs num_inputs num_outputs (added in C program once all data collected)
            # inputs1, separated by space
            # outputs1, separated by spaces
            # inputs2, separated by spaces
            # outputs2, separated by spaces
            # etc
            output_writer.writerow([accel['x'], accel['y'], accel['z'], 
                                   gyro['x'], gyro['y'], gyro['z']])
            output_writer.writerow([label])




