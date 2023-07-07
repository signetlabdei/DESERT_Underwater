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

def plot_auv_coordinates_log(file_path):
    pos = []
    auv_1 = []
    auv_2 = []
    with open(file_path, 'r') as file:        
        reader = csv.reader(file)
        for row in reader:
            if (float(row[1]) < 0):
                auv_1.append((float(row[1]),float(row[2])))
            else:
                auv_2.append((float(row[1]),float(row[2])))

    return auv_1,auv_2

def plot_coordinates_log(file_path):

    pos = []

    with open(file_path, 'r') as file:        
        reader = csv.reader(file)
        for row in reader:
            pos.append((float(row[1]),float(row[2])))
    return pos

def separate_rows(filename):
    error_ON = []
    error_OFF = []

    with open(filename, 'r') as file:
        for line in file:
            line = line.strip()
            if line.startswith('ON'):
                line_l = line.split(',')
                line_l[1] = float(line_l[1])
                line_l[2] = float(line_l[2])
                line_l[3] = float(line_l[3])

                error_ON.append(line_l[1:])
            else:
                line_l = line.split(',')
                line_l[1] = float(line_l[1])
                line_l[2] = float(line_l[2])
                line_l[3] = float(line_l[3])
                
                error_OFF.append(line_l[1:])
            
    return error_ON,error_OFF


# Retrieve command line arguments
arguments = sys.argv[1:] 

log_error = "log/error_log.csv"
log_position = "log/position_log.csv"
log_position_a = "log/position_log_a.csv"
log_terror = "log/true_error_log.csv"

difference = []
# Create a dictionary to store the positions from r_on as keys
time_e = {}
time_ne = {}

if os.path.exists(log_error):
    error_on, error_off = separate_rows(log_error)

    # Iterate over r_on and store the time values for each position
    for row in error_on:
        time_e[(row[1], row[2])] = row[0]

    for row in error_off:
        time_ne[(row[1], row[2])] = row[0]

    # Iterate over r_off and compute the time difference for each position
    for row in error_on:
        position = (row[1], row[2])
        if position in time_ne:
            difference.append(time_ne[position] -time_e[position])
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
    auv_1,auv_2 = plot_auv_coordinates_log(log_position_a)
    x_1 = [a1[0] for a1 in auv_1]
    y_1 = [a1[1] for a1 in auv_1]
    x_2 = [a2[0] for a2 in auv_2]
    y_2 = [a2[1] for a2 in auv_2]

    # Calculate the total distance traveled
    total_distance_1 = calculate_total_distance(auv_1)
    total_distance_2 = calculate_total_distance(auv_2)



print (arguments[0],';',arguments[1],';',arguments[2],';',arguments[3],';',sum(difference)/len(difference),';',total_distance,';',tp,';',fn,';',tn,';',fp,';',e,';',total_distance_1,';',total_distance_2,'\n') # time spent in error distance

#print (arguments[0],';',arguments[1],';',arguments[2],';',arguments[3],';',0,';',0,';',tp,';',fn,';',tn,';',fp,';',e,'\n') # time spent in error distance


#acc_ #error_p #sigma #time #distance + to add false detection + false release
 




