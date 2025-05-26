import csv
import matplotlib.pyplot as plt
import tikzplotlib


def plot_coordinates(file_path,x,y,t):

    with open(file_path, 'r') as file:        
        reader = csv.reader(file)
        for row in reader:
            x.append(float(row[1]) + float(row[2]))
            y.append(float(row[2]))
            t.append(float(row[0]))

def plot_coordinates_log(file_path):
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
    auv_1 = set(auv_1)
    auv_2 = set(auv_2)
    auv_3 = set(auv_3)
    auv_4 = set(auv_4)

    return auv_1,auv_2,auv_3,auv_4


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
file_path = 'log/position_log_a.csv'  # Replace with your file pathplot_coordinates(file_path)
auv_1,auv_2,auv_3,auv_4 = plot_coordinates_log(file_path)
#file_path = 'test_uwauv1_results.csv'  # Replace with your file pathplot_coordinates(file_path)
#plot_coordinates(file_path,X_1,Y_1,Z_1)

X_s = []
Y_s = []
#file_path = 'test_uwsuv_results.csv'  # Replace with your file pathplot_coordinates(file_path)
#plot_coordinates(file_path,X_s,Y_s,Z_s)
file_path = 'log/position_log.csv'  # Replace with your file pathplot_coordinates(file_path)
#plot_coordinates_log(file_path,X_s,Y_s)

X_e = []
Y_e = []
file_path = 'error_log.csv'  # Replace with your file pathplot_coordinates(file_path)
#plot_coordinates_log(file_path,X_e,Y_e)

X_e_c = []
Y_e_c = []
file_path = 'error_calling_log.csv'  # Replace with your file pathplot_coordinates(file_path)
#plot_coordinates_log(file_path,X_e_c,Y_e_c)

X_0 = []
Y_0 = []
Z_0 = []
file_path = 'test_uwauv0_results.csv'  # Replace with your file pathplot_coordinates(file_path)
#plot_coordinates(file_path,X_0,Y_0,Z_0)

X_1a = []
Y_1a = []
Z_1a = []
#file_path = 'postion_log_a.csv'  # Replace with your file pathplot_coordinates(file_path)
#plot_coordinates_log(file_path,X_1a,Y_1a)
file_path = 'test_uwauv1_results.csv'  # Replace with your file pathplot_coordinates(file_path)
#plot_coordinates(file_path,X_1a,Y_1a,Z_1a)

x_1 = [a1[0] for a1 in auv_1]
y_1 = [a1[1] for a1 in auv_1]
x_2 = [a2[0] for a2 in auv_2]
y_2 = [a2[1] for a2 in auv_2]
x_3 = [a3[0] for a3 in auv_3]
y_3 = [a3[1] for a3 in auv_3]
x_4 = [a4[0] for a4 in auv_4]
y_4 = [a4[1] for a4 in auv_4]


x_1n=[]
y_1n=[]
x_2n=[]
y_2n=[]
x_3n=[]
y_3n=[]
x_4n=[]
y_4n=[]

for i in range(0,len(x_1),5):
    x_1n.append(x_1[i])
    y_1n.append(y_1[i])

for i in range(0,len(x_2),5):
    x_2n.append(x_2[i])
    y_2n.append(y_2[i])

for i in range(0,len(x_3),5):
    x_3n.append(x_3[i])
    y_3n.append(y_3[i])

for i in range(0,len(x_4),5):
    x_4n.append(x_4[i])
    y_4n.append(y_4[i])

print(x_1n)
print(y_1n)
fig = plt.figure()
ax = plt.subplot()
ax.plot(x_1,y_1,'--',label='auv_1')
ax.plot(x_2,y_2,'--',label='auv_2')
ax.plot(x_3,y_3,'--',label='auv_3')
ax.plot(x_4,y_4,'--',label='auv_4')
ax.plot(0,0,marker='p',markersize='15',markerfacecolor='black',color='white',linewidth='2',label='asv')
#ax.plot(X_s, Y_s, 'g--',label='sv')
#ax.plot(X_e_c, Y_e_c, 'b^',label='error_called')
#ax.plot(X_e, Y_e, 'rx',label='error_Solved')
ax.set_xlabel('X')
ax.set_ylabel('Y')
ax.legend()
#ax.set_zlabel('Z')
#plt.show()
tikzplotlib.save('traj.tex')
plt.close()