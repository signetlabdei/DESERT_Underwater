import csv
import matplotlib.pyplot as plt

def plot_coordinates(file_path,x,y,t):

    with open(file_path, 'r') as file:        
        reader = csv.reader(file)
        for row in reader:
            x.append(float(row[1]) + float(row[2]))
            y.append(float(row[2]))
            t.append(float(row[0]))

def plot_coordinates_log(file_path,x,y):

    with open(file_path, 'r') as file:        
        reader = csv.reader(file)
        for row in reader:
            x.append(float(row[1]))
            y.append(float(row[2]))

def plot_movement_log(file_path,t,axis):

    with open(file_path, 'r') as file:        
        reader = csv.reader(file)
        for row in reader:
            t.append(float(row[0]))
            axis.append(float(row[1]) + float(row[2]))


    


# Example usage
X_0 = []
Y_0 = []
Z_0 = []
#file_path = 'test_uwauv0_results.csv'  # Replace with your file pathplot_coordinates(file_path)
#plot_coordinates(file_path,X_0,Y_0,Z_0)


X_1 = []
Y_1 = []
Z_1 = []
file_path = 'postion_log_a.csv'  # Replace with your file pathplot_coordinates(file_path)
plot_coordinates_log(file_path,X_1,Y_1)
#file_path = 'test_uwauv1_results.csv'  # Replace with your file pathplot_coordinates(file_path)
#plot_coordinates(file_path,X_1,Y_1,Z_1)

X_s = []
Y_s = []
#file_path = 'test_uwsuv_results.csv'  # Replace with your file pathplot_coordinates(file_path)
#plot_coordinates(file_path,X_s,Y_s,Z_s)
file_path = 'position_log.csv'  # Replace with your file pathplot_coordinates(file_path)
plot_coordinates_log(file_path,X_s,Y_s)

X_e = []
Y_e = []
file_path = 'error_log.csv'  # Replace with your file pathplot_coordinates(file_path)
plot_coordinates_log(file_path,X_e,Y_e)

X_e_c = []
Y_e_c = []
file_path = 'error_calling_log.csv'  # Replace with your file pathplot_coordinates(file_path)
plot_coordinates_log(file_path,X_e_c,Y_e_c)

X_0 = []
Y_0 = []
Z_0 = []
file_path = 'test_uwauv0_results.csv'  # Replace with your file pathplot_coordinates(file_path)
plot_coordinates(file_path,X_0,Y_0,Z_0)

X_1a = []
Y_1a = []
Z_1a = []
#file_path = 'postion_log_a.csv'  # Replace with your file pathplot_coordinates(file_path)
#plot_coordinates_log(file_path,X_1a,Y_1a)
file_path = 'test_uwauv1_results.csv'  # Replace with your file pathplot_coordinates(file_path)
plot_coordinates(file_path,X_1a,Y_1a,Z_1a)



fig = plt.figure()
ax = plt.subplot()
ax.plot(X_1a, Y_1a,'y-', label='auv_1')
ax.plot(X_s, Y_s, 'g--',label='sv')
ax.plot(X_e_c, Y_e_c, 'b^',label='error_called')
ax.plot(X_e, Y_e, 'rx',label='error_Solved')
ax.set_xlabel('X')
ax.set_ylabel('Y')
ax.legend()
#ax.set_zlabel('Z')
plt.show()

Axis = []
t = []
file_path = 'postion_log_a.csv'  # Replace with your file pathplot_coordinates(file_path)
plot_movement_log(file_path,t,Axis)

s_ax = []
s_t = []
file_path = 'position_log.csv'  # Replace with your file pathplot_coordinates(file_path)
plot_movement_log(file_path,s_t,s_ax)

X_e_c = []
Y_e_c = []
file_path = 'error_calling_log.csv'  # Replace with your file pathplot_coordinates(file_path)
plot_movement_log(file_path,X_e_c,Y_e_c)
X_e = []
Y_e = []
file_path = 'error_log.csv'  # Replace with your file pathplot_coordinates(file_path)
plot_movement_log(file_path,X_e,Y_e)
fig = plt.figure()
ax = plt.subplot()
ax.plot(Z_1a, X_1a,'y-', label='auv_1')
ax.plot(Z_0, X_0,'m-', label='auv_1')
ax.plot(t, Axis,'k.', label='auv_1')
ax.plot(s_t, s_ax,'g--', label='asv')
ax.plot(X_e_c, Y_e_c, 'b^',label='error_called')
ax.plot(X_e, Y_e, 'rx',label='error_Solved')
ax.legend()
plt.show()
