import csv
import matplotlib.pyplot as plt
import math
import sys
import os

def calculate_distance(point1, point2):
    x1, y1 = point1
    x2, y2 = point2
    return math.sqrt((x2 - x1)**2 + (y2 - y1)**2)

def calculate_total_distance(positions):
    total_distance = 0.0
    for i in range(len(positions) - 1):
        distance = calculate_distance(positions[i], positions[i+1])
        total_distance += distance
    return total_distance

def plot_coordinates_log(file_path):
    pos = []
    with open(file_path, 'r') as file:        
        reader = csv.reader(file)
        for row in reader:
            pos.append((float(row[1]),float(row[2])))
    return pos

def plot_auv_coordinates_log(file_path):
    pos = []
    auv_1 = []
    auv_2 = []
    auv_3 = []
    auv_4 = []
    with open(file_path, 'r') as file:        
        reader = csv.reader(file)
        for row in reader:
            if (float(row[1]) < 0 and float(row[2]) > 0): #1Q -> auv_1
                auv_1.append((float(row[1]),float(row[2])))
            elif (float(row[1]) > 0 and float(row[2]) > 0): #2Q -> auv_2
                auv_2.append((float(row[1]),float(row[2])))
            elif (float(row[1]) < 0 and float(row[2]) < 0): #3Q -> auv_3
                auv_3.append((float(row[1]),float(row[2])))
            else:                                           #4Q -> auv_3
                auv_4.append((float(row[1]),float(row[2])))

    return auv_1,auv_2,auv_3,auv_4

def tx_log(filename):
    with open(filename, 'r') as file:
        for line in file:
            line = line.split(" ")
        
    return line

def separate_rows(filename):
    error_g_on = []
    error_g_off = []
    error_r_on = []
    error_r_off = []
    error_w = []

    with open(filename, 'r') as file:
        for line in file:
            line = line.strip().split(',')
            line[1] = float(line[1])
            line[2] = float(line[2])
            line[3] = float(line[3])

            if line[0].strip() == 'G':
                if line[4].strip() == 'ON':
                    error_g_on.append(line[1:])
                else:
                    error_g_off.append(line[1:])

            elif line[0].strip() =='R':
                if line[4].strip() == 'ON':
                    error_r_on.append(line[1:])
                else:
                    error_r_off.append(line[1:])
            else:
                error_w.append(line[1:])
            

    return error_g_on,error_g_off, error_r_on,error_r_off, error_w


# Retrieve command line arguments
arguments = sys.argv[1:] 

log_error = "log/error_log_t.csv"
log_position = "log/position_log.csv"
log_position_a = "log/position_log_a.csv"
log_terror = "log/true_error_log.csv"

difference = []
error_g_on = []
error_g_off = []
error_r_on = []
error_r_off = []
error_w = []
# Create a dictionary to store the positions from r_on as keys
time_w = {}
time_g = {}

if os.path.exists(log_error):
    error_g_on, error_g_off, error_r_on, error_r_off, error_w = separate_rows(log_error)

    # Iterate over r_on and store the time values for each position
    for row in error_w:
        time_w[(row[1], row[2])] = row[0]   

    #for row in error_g_off:
    #    time_g[(row[2], row[3])] = row[1]

    # Iterate over r_off and compute the time difference for each position
    for row in error_g_on:
        position = (row[1], row[2])
        if position in time_w:
            difference.append(time_w[position] - row[0])
else:
    difference.append(0)


tp=0
tn=0
e=0
fp=0
fn=0
big_dict={}

if os.path.exists(log_terror):
    with open(log_terror, 'r') as file:
        for line in file:
            line = line.strip()  # Remove leading/trailing whitespaces
            values = line.split(',')  # Split the line by comma
            
            # Extract the values
            if len(values) == 4:
                value1 = float(values[0])
                value2 = float(values[1])
                value3 = float(values[2])
                value4 = values[3]

                if value4 == 'e':
                    e += 1
                elif (value2,value3) not in big_dict:
                    big_dict[value2,value3] = value4
                
                
            # Check if the second and third columns are equal

        for val in big_dict.values():
            if val == 'tp':
                tp += 1
            elif val == 'fn':
                fn+=1
            elif val == 'fp':
                fp+=1
            elif val == 'tn':
                tn+=1

total_distance = 0
if os.path.exists(log_position):
    pos_sv = plot_coordinates_log(log_position)
    # Calculate the total distance traveled
    total_distance = calculate_total_distance(pos_sv)

if os.path.exists(log_position_a):
    auv_1,auv_2,auv_3,auv_4 = plot_auv_coordinates_log(log_position_a)
    x_1 = [a1[0] for a1 in auv_1]
    y_1 = [a1[1] for a1 in auv_1]
    x_2 = [a2[0] for a2 in auv_2]
    y_2 = [a2[1] for a2 in auv_2]
    x_3 = [a3[0] for a3 in auv_3]
    y_3 = [a3[1] for a3 in auv_3]
    x_4 = [a4[0] for a4 in auv_4]
    y_4 = [a4[1] for a4 in auv_4]

    # Calculate the total distance traveled
    total_distance_1 = calculate_total_distance(auv_1)
    total_distance_2 = calculate_total_distance(auv_2)
    total_distance_3 = calculate_total_distance(auv_3)
    total_distance_4 = calculate_total_distance(auv_4)

tx_data = tx_log('log.out')
print (arguments[0],';',arguments[1],';',arguments[2],';',arguments[3],';',sum(difference)/len(difference),';',total_distance,';',tp,';',fn,';',tn,';',fp,';',e,';',total_distance_1,';',total_distance_2,';',total_distance_3,';',total_distance_4,';',tx_data[0],';',tx_data[1],';',tx_data[2],';',tx_data[3],';',tx_data[4],';',tx_data[5],';',tx_data[6],';',tx_data[7],'\n') # time spent in error distance



