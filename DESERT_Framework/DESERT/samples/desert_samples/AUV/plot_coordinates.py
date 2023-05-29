import csv
import matplotlib.pyplot as plt

def plot_coordinates(file_path,x,y,z):

    with open(file_path, 'r') as file:        
        reader = csv.reader(file)
        for row in reader:
            x.append(float(row[0]))
            y.append(float(row[1]))
            z.append(float(row[2]))

    


# Example usage
X_0 = []
Y_0 = []
Z_0 = []
X_1 = []
Y_1 = []
Z_1 = []
X_s = []
Y_s = []
Z_s = []
file_path = 'test_uwauv0_results.csv'  # Replace with your file pathplot_coordinates(file_path)
plot_coordinates(file_path,X_0,Y_0,Z_0)
file_path = 'test_uwauv1_results.csv'  # Replace with your file pathplot_coordinates(file_path)
plot_coordinates(file_path,X_1,Y_1,Z_1)
file_path = 'test_uwsuv_results.csv'  # Replace with your file pathplot_coordinates(file_path)
plot_coordinates(file_path,X_s,Y_s,Z_s)

fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')
ax.plot(X_0, Y_0, Z_0, label='auv_0')
ax.plot(X_1, Y_1, Z_1, label='auv_1')
ax.plot(X_s, Y_s, Z_s,label='sv')
ax.set_xlabel('X')
ax.set_ylabel('Y')
ax.set_zlabel('Z')
ax.legend()
#ax.set_zlabel('Z')
plt.show()