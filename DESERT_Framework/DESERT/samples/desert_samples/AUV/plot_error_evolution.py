import csv
import matplotlib.pyplot as plt

def plot_coordinates_log(file_path):

    x = []
    y = []

    with open(file_path, 'r') as file:        
        reader = csv.reader(file)
        for row in reader:
            x.append(float(row[1]))
            y.append(float(row[2]))
    return x,y

def plot_movement_log(file_path):

    t=[]
    axis = []

    with open(file_path, 'r') as file:        
        reader = csv.reader(file)
        for row in reader:
            t.append(float(row[0]))
            axis.append(float(row[1]) + float(row[2]))
    return t,axis

def separate_rows_s(filename):
    error_g = []
    error_r = []
    error_w = []

    with open(filename, 'r') as file:
        for line in file:
            line = line.strip()
            if line.endswith('ON'):
                line_l = line.split(',')
                line_l[2] = float(line_l[2]) + float(line_l[3])
                line_l[-1] = 1
            else:
                line_l = line.split(',')
                line_l[2] = float(line_l[2]) + float(line_l[3])
                line_l[-1] = 0
            
            if line.startswith('G'):
                #print(line_l)
                error_g.append(line_l)
            elif line.startswith('R'):
                error_r.append(line_l)
            else:
                error_w.append(line_l)

    return error_g, error_r, error_w

    return error_g, error_r

def separate_rows(filename):
    error_g = []
    error_r = []
    error_w = []

    with open(filename, 'r') as file:
        for line in file:
            line = line.strip()
            if line.endswith('ON'):
                line_l = line.split(',')
                #line_l[2] = float(line_l[2]) + float(line_l[3])
                line_l[-1] = 1
            else:
                line_l = line.split(',')
                #line_l[2] = float(line_l[2]) + float(line_l[3])
                line_l[-1] = 0
            
            if line.startswith('G'):
                #print(line_l)
                error_g.append(line_l)
            elif line.startswith('R'):
                error_r.append(line_l)
            else:
                error_w.append(line_l)

    return error_g, error_r, error_w


    

log_error = "log/error_log_t.csv"
log_position = "log/position_log.csv"

error_g, error_r, error_w = separate_rows_s(log_error)
error_g_on = [row for row in error_g if row[-1] == 1]
error_g_off = [row for row in error_g if row[-1] == 0]

error_r_on = [row for row in error_r if row[-1] == 1]
error_r_off = [row for row in error_r if row[-1] == 0]

r_on_t = [float(row[1]) for row in error_r_on]
r_off_t = [float(row[1])  for row in error_r_off]
r_on_ax= [float(row[2])  for row in error_r_on]
r_off_ax= [float(row[2]) for row in error_r_off]

print(r_on_ax)

g_on_t = [float(row[1])  for row in error_g_on]
g_off_t = [float(row[1])  for row in error_g_off]
g_on_ax= [float(row[2]) for row in error_g_on]
g_off_ax= [float(row[2]) for row in error_g_off]

w_t = [float(row[1]) for row in error_w]
w_ax= [float(row[2])  for row in error_w]

x_c, y_c = plot_coordinates_log(log_position)
t,axis = plot_movement_log(log_position)

file_path = 'log/position_log_a.csv'  # Replace with your file pathplot_coordinates(file_path)
x_a, y_a = plot_coordinates_log(file_path)
t_a, ax_a = plot_movement_log(file_path)

fig = plt.figure()
ax = plt.subplot()
ax.plot(t, axis,'g--', label='sv')
ax.plot(t_a, ax_a,'m.', label='sv')
ax.plot(g_on_t, g_on_ax,'ko', label='g_error_on')
#ax.plot(g_off_t, g_off_ax,'kx', label='g_error_off')
ax.plot(r_on_t, r_on_ax,'ro', label='r_error_on')
#ax.plot(r_off_t,r_off_ax,'rx', label='r_error_off')
ax.plot(w_t,w_ax,'^',color='orange', label='error_off')

ax.legend()
plt.show()

error_g, error_r,error_w = separate_rows(log_error)
error_g_on = [row for row in error_g if row[-1] == 1]
error_g_off = [row for row in error_g if row[-1] == 0]

error_r_on = [row for row in error_r if row[-1] == 1]
error_r_off = [row for row in error_r if row[-1] == 0]

r_on_t = [float(row[1]) for row in error_r_on]
r_off_t = [float(row[1])  for row in error_r_off]
r_on_x= [float(row[2])  for row in error_r_on]
r_off_x= [float(row[2]) for row in error_r_off]
r_on_y= [float(row[3])  for row in error_r_on]
r_off_y= [float(row[3]) for row in error_r_off]

g_on_t = [float(row[1])  for row in error_g_on]
g_off_t = [float(row[1])  for row in error_g_off]
g_on_x= [float(row[2]) for row in error_g_on]
g_off_x= [float(row[2]) for row in error_g_off]
g_on_y= [float(row[3])  for row in error_g_on]
g_off_y= [float(row[3]) for row in error_g_off]



fig = plt.figure()
ax = plt.subplot()
ax.plot(x_c, y_c,t,'g--', label='sv')
ax.plot(x_a, y_a,t_a,'m.', label='sv')
ax.plot(g_on_x, g_on_y,g_on_t,'ko', label='g_error_on')
ax.plot(g_off_x, g_off_y,g_off_t,'kx', label='g_error_off')
ax.plot(r_on_x, r_on_y,'ro',r_on_t, label='r_error_on')
ax.plot(r_off_x,r_off_y,'rx',r_off_t, label='r_error_off')


ax.legend()
plt.show()







