import matplotlib.pyplot as plt
import numpy as np
from scipy import stats
import seaborn as sns
import pandas as pd
import tikzplotlib

def compute_power():
    v = 1.5
    Pm= 16.3*(v**2)
    #Pd = 0.5*a_dens*(v**3)*Cd*A   #1/2*rho*v^3*C*A
    return Pm#+Pd

def compute_error(file_path,pkt_size):
    d_1 = {}
    d_2 = {}
    d_3 = {}
    d_4 = {}
    m_t = {}
    m_d ={}
    m_tp={}
    m_fp={}
    m_tn={}
    m_fn={}
    acc=[]
    pr = {}
    rc = {}
    e = {}
    auv_e = {}
    asv_e = {}

    with open(file_path, 'r') as file:
        p=compute_power()
        v=1.5
        t=pkt_size*8/4800
        #t=0.208

        for row in file:
            row = row.replace(',','.')
            row = row.strip().split(';')

            if row != ['']:
                if float(row[0]) not in m_t:
                    if float(row[10]):
                        m_t[float(row[0])]=[float(row[4])/float(row[10])]
                        d=float(row[5])/float(row[10])
                        power = ((d/v)/3600)*(p/1000)
                        m_d[float(row[0])]=[power]
                    else:
                        m_t[float(row[0])]=[float(row[4])]
                        d=float(row[5])
                        power = ((d/v)/3600)*(p/1000)
                        m_d[float(row[0])]=[power]
                    
                    d_1[float(row[0])]=[float(row[11])]
                    d_2[float(row[0])]=[float(row[12])]
                    d_3[float(row[0])]=[float(row[13])]
                    d_4[float(row[0])]=[float(row[14])]

                    m_fn[float(row[0])]=[float(row[7])]
                    m_tn[float(row[0])]=[float(row[8])]
                    m_fp[float(row[0])]=[float(row[9])]
                    m_tp[float(row[0])]=[float(row[6])]

                    e[float(row[0])]=[float(row[10])]


                    if float(row[10]):       
                        if float(row[6])+float(row[9]):
                            pr[float(row[0])]=[float(row[6])/(float(row[6])+float(row[9]))]
                        if float(row[6])+float(row[7]) and float(row[6]):
                            rc[float(row[0])]=[float(row[6])/(float(row[6])+float(row[7]))] 

                    sent_u = float(row[15])
                    sent_u_er = float(row[19])
                    rcv_s = float(row[16])
                    rcv_s_er = float(row[20])

                    

                    sent_s = float(row[17])
                    sent_s_er = float(row[21])
                    rcv_u = float(row[18]) 
                    rcv_u_er = float(row[22])

                    #asv_e[float(row[0])] = [sent_s]
                    #auv_e[float(row[0])] = [sent_u]
                    listen_time = 150000-(rcv_s_er)*(t)-(sent_s +rcv_s+sent_s_er)*0.008

                    asv_e[float(row[0])] = [(rcv_s_er * (t/3600) + rcv_s*0.008/3600) * 0.8/1000 + ((sent_s_er + sent_s)*0.008/3600)*60/1000 + (listen_time/3600)*0.01/1000]
                    listen_time = 150000-(sent_u_er +rcv_u_er)*(t) -(sent_u +rcv_u)*(0.008)
                    #auv_e[float(row[0])] = [rcv_s*(t/3600)*0.8/1000 + sent_u*(t/3600)*60/1000 + listen_time*0.01/1000]#*t*60 + rcv*t*0.8]
                    #auv_e[float(row[0])] = [sent_u_er
                    

                else:
                    e[float(row[0])].append(float(row[10]))

                    d_1[float(row[0])].append(float(row[11]))
                    d_2[float(row[0])].append(float(row[12]))
                    d_3[float(row[0])].append(float(row[13]))
                    d_4[float(row[0])].append(float(row[14]))

                    if float(row[10]):
                        m_t[float(row[0])].append(float(row[4])/float(row[10]))
                        d=float(row[5])/float(row[10])
                        power=((d/v)/3600)*p/1000
                        m_d[float(row[0])].append(((d/v)/3600)*(p/1000))
                    else:
                        m_t[float(row[0])].append(float(row[4]))
                        d=float(row[5])
                        power =((d/v)/3600)*(p/1000)
                        m_d[float(row[0])].append(power)
                    
                    m_fn[float(row[0])].append(float(row[7]))
                    m_tn[float(row[0])].append(float(row[8]))
                    m_fp[float(row[0])].append(float(row[9]))
                    m_tp[float(row[0])].append(float(row[6]))

                    sent_u = float(row[15])
                    rcv_u = float(row[16])
                    sent_s = float(row[17])
                    rcv_s = float(row[18]) 
                    sent_u_er = float(row[19])
                    
                    rcv_s_er = float(row[20])

                    
                    sent_s_er = float(row[21])
                    
                    rcv_u_er = float(row[22])

                    #asv_e[float(row[0])] = [sent_s]
                    #auv_e[float(row[0])] = [sent_u]
                    listen_time = 150000-(rcv_s_er)*(t)-(sent_s +rcv_s+sent_s_er )*0.008

                    asv_e[float(row[0])].append((rcv_s_er * (t/3600) + rcv_s*0.008/3600) * 0.8/1000 + ((sent_s_er + sent_s)*0.008/3600)*60/1000 + (listen_time/3600)*0.01/1000)
                    listen_time = 150000-(sent_u_er +rcv_u_er)*(t) -(sent_u +rcv_u)*(0.008)
                    #auv_e[float(row[0])].append(rcv_s*(t/3600)*0.8/1000 + sent_u*(t/3600)*60/1000 + listen_time*0.01/1000) #*t*60 + rcv*t*0.8]
                    #auv_e[float(row[0])].append(sent_u_er)#*t*60 + rcv*t*0.8]
                    
                    #asv_e[float(row[0])].append(sent_s)#*t*60 + rcv*t*0.8)
                    #auv_e[float(row[0])].append(sent_u)#*t*60 + rcv*t*0.8)

                    if float(row[10]): 
                        if float(row[0]) in m_tp:
                            if float(row[6])+float(row[9]):
                                if float(row[0]) in pr:
                                    pr[float(row[0])].append(float(row[6])/(float(row[6])+float(row[9])))
                                else:
                                    pr[float(row[0])]=[float(row[6])/(float(row[6])+float(row[9]))]
                            
                            if float(row[6]):
                                if float(row[6])+float(row[7]):
                                    if float(row[0]) in rc :
                                        rc[float(row[0])].append(float(row[6])/(float(row[6])+float(row[7])))
                                    else:
                                        rc[float(row[0])]=[float(row[6])/(float(row[6])+float(row[7]))] 
                        else:
                            m_tp[float(row[0])]=[float(row[6])]
                            if float(row[6])+float(row[9]):
                                pr[float(row[0])]=[float(row[6])/(float(row[6])+float(row[9]))]
                            if float(row[6])+float(row[7]) and float(row[6]):
                                rc[float(row[0])]=[float(row[6])/(float(row[6])+float(row[7]))] 


                acc.append(float(row[0]))

    return m_t,m_d,m_tp,m_fp,m_tn,m_fn,acc,pr,rc, d_1, d_2, d_3, d_4, asv_e, auv_e, e


file_path = "result.csv"

m1_t,m1_d,m1_tp,m1_fp,m1_tn,m1_fn,acc1,pr1,rc1, d11,d12,d13,d14, asv_e, auv_e, e1 = compute_error(file_path,125)

file_path = "result_2.csv"

m2_t,m2_d,m2_tp,m2_fp,m2_tn,m2_fn,acc2,pr2,rc2,d21,d22,d23,d24,asv_e2, auv_e2, e2 = compute_error(file_path,5)


for key in acc1:
    for j in range(len(d11[key])):
        d11[key][j] = 0.25*(d12[key][j]+ d11[key][j]+d13[key][j]+d14[key][j])
        #d11[key][j] = (d12[key][j]+ d11[key][j]+d13[key][j]+d14[key][j])

for key in acc2:
     for j in range(len(d21[key])):
         d21[key][j] = 0.25*(d22[key][j]+ d21[key][j]+d23[key][j]+d24[key][j])
         #d21[key][j] = (d22[key][j]+ d21[key][j]+d23[key][j]+d24[key][j])

pr1 = {'Key':pr1.keys(),
        'Values':pr1.values()}
pr1 = pd.DataFrame(pr1)

ax = plt.figure()
pr1 = pr1.explode('Values')
pr1 = pr1.reset_index(drop=True)
pr1.columns= ['Key','Values']
sns.lineplot(data=pr1,x='Key',y='Values',marker='o',label='IER')

pr2 = {'Key':pr2.keys(),
         'Values':pr2.values()}
pr2 = pd.DataFrame(pr2)

pr2 = pr2.explode('Values')
pr2 = pr2.reset_index(drop=True)
pr2.columns= ['Key','Values']
sns.lineplot(data=pr2,x='Key',y='Values',marker='^',markersize='8',label='SER')

plt.ylim(bottom=0)
plt.xscale('log')
plt.grid(True, linestyle='dashed', linewidth=0.5, color='gray',axis='y',which='both')
plt.xlabel('$\\theta$')
#plt.show()
tikzplotlib.save('pr.tex')
plt.close()


rc1 = {'Key':rc1.keys(),
        'Values':rc1.values()}
rc1 = pd.DataFrame(rc1)

ax = plt.figure()
rc1 = rc1.explode('Values')
rc1 = rc1.reset_index(drop=True)
rc1.columns= ['Key','Values']
sns.lineplot(data=rc1,x='Key',y='Values',marker='o',label='IER')

rc2 = {'Key':rc2.keys(),
        'Values':rc2.values()}
rc2 = pd.DataFrame(rc2)

rc2 = rc2.explode('Values')
rc2 = rc2.reset_index(drop=True)
rc2.columns= ['Key','Values']
sns.lineplot(data=rc2,x='Key',y='Values',marker='^',markersize='8',label='SER')

plt.ylim(bottom=0)
plt.xscale('log')
plt.grid(True, linestyle='dashed', linewidth=0.5, color='gray',axis='y',which='both')
plt.xlabel('$\\theta$')
#plt.show()
tikzplotlib.save('rec.tex')
plt.close()


ax = plt.figure()
#plt.plot(m_d.keys(),mean_d,'o--',label='distance', color='#1f77b4')
#plt.errorbar(m_d.keys(),mean_d,yerr=ci_d,fmt='o',color='#1f77b4')
data = {'Key':m1_t.keys(),
        'Values':m1_d.values()}
tp_df = pd.DataFrame(data)
tp_df = tp_df.explode('Values')
tp_df = tp_df.reset_index(drop=True)
tp_df.columns= ['Key','Values']
sns.lineplot(data=tp_df,x='Key',y='Values',marker='o',label='IER')

data = {'Key':m2_t.keys(),
        'Values':m2_d.values()}
tp_df = pd.DataFrame(data)
tp_df = tp_df.explode('Values')
tp_df = tp_df.reset_index(drop=True)
tp_df.columns= ['Key','Values']
sns.lineplot(data=tp_df,x='Key',y='Values',marker='^',markersize='8',label='SER')

plt.xscale('log')
#plt.yscale('log')
plt.grid(True, linestyle='dashed', linewidth=0.5, color='gray',axis='y',which='both')
plt.xlabel('$\\theta$')
plt.ylabel('kWh',rotation=0)
#plt.title('ASV\'s Distance Traveled')
#plt.title('ASV\'s Energy Consumption (movement) per Error')
#plt.show()
tikzplotlib.save('energy_mov.tex')
plt.close()

ax = plt.figure()
#plt.plot(m_d.keys(),mean_d,'o--',label='distance', color='#1f77b4')
#plt.errorbar(m_d.keys(),mean_d,yerr=ci_d,fmt='o',color='#1f77b4')
data = {'Key':m1_t.keys(),
        'Values':asv_e.values()}
tp_df = pd.DataFrame(data)
tp_df = tp_df.explode('Values')
tp_df = tp_df.reset_index(drop=True)
tp_df.columns= ['Key','Values']
sns.lineplot(data=tp_df,x='Key',y='Values',marker='o',label='IER')

data = {'Key':m2_t.keys(),
        'Values':asv_e2.values()}
tp_df = pd.DataFrame(data)
tp_df = tp_df.explode('Values')
tp_df = tp_df.reset_index(drop=True)
tp_df.columns= ['Key','Values']
sns.lineplot(data=tp_df,x='Key',y='Values',marker='^',markersize='8',label='SER')

plt.xscale('log')
#plt.yscale('log')
plt.grid(True, linestyle='dashed', linewidth=0.5, color='gray',axis='y',which='both')
plt.xlabel('$\\theta$')
plt.ylabel('kWh',rotation=0)
#plt.title('ASV\'s Distance Traveled')
#plt.ylim((50000,500000))
#plt.ylim((10,100))
#plt.title('ASV\'s Energy Consumption (total)')
#plt.show()
tikzplotlib.save('energy_tx.tex')
plt.close()

#ax = plt.figure()
#plt.plot(m_d.keys(),mean_d,'o--',label='distance', color='#1f77b4')
#plt.errorbar(m_d.keys(),mean_d,yerr=ci_d,fmt='o',color='#1f77b4')
#data = {'Key':m1_t.keys(),
#        'Values':auv_e.values()}
#tp_df = pd.DataFrame(data)
#tp_df = tp_df.explode('Values')
#tp_df = tp_df.reset_index(drop=True)
#tp_df.columns= ['Key','Values']
#sns.lineplot(data=tp_df,x='Key',y='Values',marker='o',label='IER')

#data = {'Key':m2_t.keys(),
#        'Values':auv_e2.values()}
#tp_df = pd.DataFrame(data)
#tp_df = tp_df.explode('Values')
#tp_df = tp_df.reset_index(drop=True)
#tp_df.columns= ['Key','Values']
#sns.lineplot(data=tp_df,x='Key',y='Values',marker='^',markersize='8',label='SER')

#plt.xscale('log')
#plt.yscale('log')
#plt.grid(True, linestyle='dashed', linewidth=0.5, color='gray',axis='y',which='both')
#plt.xlabel('$\epsilon$')
#plt.ylabel('kWh',rotation=0)
#plt.ylim(top=10800)
#plt.title('ASV\'s Distance Traveled')
#plt.ylim((10,100))
#plt.title('AUV\'s Energy Consumption (tx)')
#plt.show()
#tikzplotlib.save('energy_tx.tex')
#plt.close()


ax = plt.figure()
#plt.plot(m_t.keys(),mean_t,'o--',label='time',color='#1f77b4')
#plt.errorbar(m_t.keys(),mean_t,yerr=ci_t,fmt='o', color='#1f77b4')
data = {'Key':m1_t.keys(),
        'Values':m1_t.values()}
tp_df = pd.DataFrame(data)
tp_df = tp_df.explode('Values')
tp_df = tp_df.reset_index(drop=True)
tp_df.columns= ['Key','Values']
sns.lineplot(data=tp_df,x='Key',y='Values',marker='o',label='IER')

data = {'Key':m2_t.keys(),
        'Values':m2_t.values()}
tp_df = pd.DataFrame(data)
tp_df = tp_df.explode('Values')
tp_df = tp_df.reset_index(drop=True)
tp_df.columns= ['Key','Values']
sns.lineplot(data=tp_df,x='Key',y='Values',marker='^',markersize='8',label='SER')
plt.xscale('log')
#plt.yscale('log')
plt.grid(True, linestyle='dashed', linewidth=0.5, color='gray',axis='y',which='both')
plt.xlabel('$\\theta$')
plt.ylabel('Time')
#plt.title('Average Downtime per Error')
#plt.show()
tikzplotlib.save('time.tex')
plt.close()

ax = plt.figure()

data = {'Key':d11.keys(),
        'Values':d11.values()}
tp_df = pd.DataFrame(data)
tp_df = tp_df.explode('Values')
tp_df = tp_df.reset_index(drop=True)
tp_df.columns= ['Key','Values']
sns.lineplot(data=tp_df,x='Key',y='Values',marker='o',label='IER')

data = {'Key':d21.keys(),
        'Values':d21.values()}
tp_df = pd.DataFrame(data)
tp_df = tp_df.explode('Values')
tp_df = tp_df.reset_index(drop=True)
tp_df.columns= ['Key','Values']
sns.lineplot(data=tp_df,x='Key',y='Values',marker='^',markersize='8',label='SER')


plt.xscale('log')
plt.yscale('log')
plt.grid(True, linestyle='dashed', linewidth=0.5, color='gray',axis='y',which='both')
plt.xlabel('$\\theta$')
plt.ylabel('Distance')
#plt.title('AUV\'s Distance Covered')
plt.show()
#tikzplotlib.save('dist.tex')
plt.close()



ax = plt.figure()

data = {'Key':d11.keys(),
        'Values':m1_tp.values()}
tp_df = pd.DataFrame(data)
tp_df = tp_df.explode('Values')
tp_df = tp_df.reset_index(drop=True)
tp_df.columns= ['Key','Values']
sns.lineplot(data=tp_df,x='Key',y='Values',marker='o',label='IER')

data = {'Key':d21.keys(),
        'Values':m2_tp.values()}
tp_df = pd.DataFrame(data)
tp_df = tp_df.explode('Values')
tp_df = tp_df.reset_index(drop=True)
tp_df.columns= ['Key','Values']
sns.lineplot(data=tp_df,x='Key',y='Values',marker='^',markersize='8',label='SER')


plt.xscale('log')
plt.grid(True, linestyle='dashed', linewidth=0.5, color='gray',axis='y',which='both')
plt.xlabel('$\\theta$')
plt.ylabel('True Error')
#plt.title('AUV\'s Distance Covered')
plt.show()
#tikzplotlib.save('t_e.tex')
plt.close()

