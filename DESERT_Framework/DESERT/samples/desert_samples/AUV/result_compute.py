import matplotlib.pyplot as plt
import numpy as np
from scipy import stats
import seaborn as sns
import pandas as pd

def compute_error(file_path):
    d_1 = {}
    d_2 = {}
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
    with open(file_path, 'r') as file:
        for row in file:
            row = row.replace(',','.')
            row = row.strip().split(';')

            if row != ['']:
                if float(row[0]) not in m_t:
                    if float(row[10]):
                        m_t[float(row[0])]=[float(row[4])/float(row[10])]
                        m_d[float(row[0])]=[float(row[5])/float(row[10])]
                    else:
                        m_t[float(row[0])]=[float(row[4])]
                        m_d[float(row[0])]=[float(row[5])]
                    
                    d_1[float(row[0])]=[float(row[11])]
                    d_2[float(row[0])]=[float(row[12])]

                    m_fn[float(row[0])]=[float(row[7])]
                    m_tn[float(row[0])]=[float(row[8])]
                    m_fp[float(row[0])]=[float(row[9])]

                    e[float(row[0])]=[float(row[10])]


                    if float(row[10]):
                        m_tp[float(row[0])]=[float(row[6])]
                        if float(row[6])+float(row[9]):
                            pr[float(row[0])]=[float(row[6])/(float(row[6])+float(row[9]))]
                        if float(row[6])+float(row[7]):
                            rc[float(row[0])]=[float(row[6])/(float(row[6])+float(row[7]))]  

                else:
                    e[float(row[0])].append(float(row[10]))

                    d_1[float(row[0])].append(float(row[11]))
                    d_2[float(row[0])].append(float(row[12]))

                    if float(row[10]):
                        m_t[float(row[0])].append(float(row[4])/float(row[10]))
                        m_d[float(row[0])].append(float(row[5])/float(row[10]))
                    else:
                        m_t[float(row[0])].append(float(row[4]))
                        m_d[float(row[0])].append(float(row[5]))
                    
                    m_fn[float(row[0])].append(float(row[7]))
                    m_tn[float(row[0])].append(float(row[8]))
                    m_fp[float(row[0])].append(float(row[9]))

                    if float(row[10]): 
                        if float(row[0]) in m_tp:
                            m_tp[float(row[0])].append(float(row[6]))
                            if float(row[6])+float(row[9]):
                                pr[float(row[0])].append(float(row[6])/(float(row[6])+float(row[9])))
                            if float(row[6])+float(row[7]):
                                rc[float(row[0])].append(float(row[6])/(float(row[6])+float(row[7])))
                        else:
                            m_tp[float(row[0])]=[float(row[6])]
                            if float(row[6])+float(row[9]):
                                pr[float(row[0])]=[float(row[6])/(float(row[6])+float(row[9]))]
                            if float(row[6])+float(row[7]):
                                rc[float(row[0])]=[float(row[6])/(float(row[6])+float(row[7]))] 


                acc.append(float(row[0]))

    return m_t,m_d,m_tp,m_fp,m_tn,m_fn,acc,pr,rc, d_1, d_2


file_path = "result.csv"

m1_t,m1_d,m1_tp,m1_fp,m1_tn,m1_fn,acc1,pr1,rc1, d11,d12 = compute_error(file_path)

file_path = "result_2.csv"

m2_t,m2_d,m2_tp,m2_fp,m2_tn,m2_fn,acc2,pr2,rc2,d21,d22 = compute_error(file_path)



pr1 = {'Key':pr1.keys(),
        'Values':pr1.values()}
pr1 = pd.DataFrame(pr1)

ax = plt.figure()
pr1 = pr1.explode('Values')
pr1 = pr1.reset_index(drop=True)
pr1.columns= ['Key','Values']
sns.lineplot(data=pr1,x='Key',y='Values',markers=True,label='smart')

pr2 = {'Key':pr2.keys(),
        'Values':pr2.values()}
pr2 = pd.DataFrame(pr2)

pr2 = pr2.explode('Values')
pr2 = pr2.reset_index(drop=True)
pr2.columns= ['Key','Values']
print(pr2['Key'])
sns.lineplot(data=pr2,x='Key',y='Values',markers=True,label='basic')

plt.title('Precision')
plt.xscale('log')
plt.grid()
plt.xlabel('Accuracy')
ax.legend()
plt.show()


rc1 = {'Key':rc1.keys(),
        'Values':rc1.values()}
rc1 = pd.DataFrame(rc1)

ax = plt.figure()
rc1 = rc1.explode('Values')
rc1 = rc1.reset_index(drop=True)
rc1.columns= ['Key','Values']
sns.lineplot(data=rc1,x='Key',y='Values',markers='o',label='smart')

rc2 = {'Key':rc2.keys(),
        'Values':rc2.values()}
rc2 = pd.DataFrame(rc2)

rc2 = rc2.explode('Values')
rc2 = rc2.reset_index(drop=True)
rc2.columns= ['Key','Values']
sns.lineplot(data=rc2,x='Key',y='Values',markers='o',label='basic')

plt.title('Recall')
plt.xscale('log')
plt.grid()
plt.xlabel('Accuracy')
ax.legend()
plt.show()


# m1_tp = {'Key':m1_t.keys(),
#         'Values':m1_tp.values()}
# m1_tp = pd.DataFrame(m1_tp)

# ax = plt.figure()
# m1_tp = m1_tp.explode('Values')
# m1_tp = m1_tp.reset_index(drop=True)
# m1_tp.columns= ['Key','Values']
# sns.lineplot(data=m1_tp,x='Key',y='Values',label='smart')

# m2_tp = {'Key':m2_t.keys(),
#         'Values':m2_tp.values()}
# tp_df = pd.DataFrame(m2_tp)
# tp_df = tp_df.explode('Values')
# tp_df = tp_df.reset_index(drop=True)
# tp_df.columns= ['Key','Values']
# sns.lineplot(data=tp_df,x='Key',y='Values',label='basic')

# plt.title('True positive')
# plt.xscale('log')
# plt.grid()
# plt.xlabel('Accuracy')
# ax.legend()
# plt.show()

# plt.figure()
# #plt.errorbar(m_fp.keys(),mean_fp,yerr=ci_fp,label='fp',fmt='o',color='#1f77b4')
# #plt.plot(m_fp.keys(),mean_fp,'o--',color='#1f77b4')
# data = {'Key':m1_t.keys(),
#         'Values':m1_fp.values()}
# tp_df = pd.DataFrame(data)
# tp_df = tp_df.explode('Values')
# tp_df = tp_df.reset_index(drop=True)
# tp_df.columns= ['Key','Values']
# sns.lineplot(data=tp_df,x='Key',y='Values',label='smart')

# data = {'Key':m2_t.keys(),
#         'Values':m2_fp.values()}
# tp_df = pd.DataFrame(data)
# tp_df = tp_df.explode('Values')
# tp_df = tp_df.reset_index(drop=True)
# tp_df.columns= ['Key','Values']
# sns.lineplot(data=tp_df,x='Key',y='Values',label='basic')
# plt.xscale('log')
# plt.grid()
# plt.title('False positive')
# plt.xlabel('Accuracy')
# plt.show()

# plt.figure()
# #plt.errorbar(m_fn.keys(),mean_fn,yerr=ci_fn,label='fn',fmt='o',color='#1f77b4')
# #plt.plot(m_fn.keys(),mean_fn,'^--',color='#1f77b4')
# data = {'Key':m1_t.keys(),
#         'Values':m1_fn.values()}
# tp_df = pd.DataFrame(data)
# tp_df = tp_df.explode('Values')
# tp_df = tp_df.reset_index(drop=True)
# tp_df.columns= ['Key','Values']
# sns.lineplot(data=tp_df,x='Key',y='Values',label='smart')
# data = {'Key':m2_t.keys(),
#         'Values':m2_fn.values()}
# tp_df = pd.DataFrame(data)
# tp_df = tp_df.explode('Values')
# tp_df = tp_df.reset_index(drop=True)
# tp_df.columns= ['Key','Values']
# sns.lineplot(data=tp_df,x='Key',y='Values',label='basic')
# plt.grid()
# plt.xscale('log')
# plt.title('False negative')
# plt.xlabel('Accuracy')
# plt.show()

# plt.figure()
# #plt.errorbar(m_tn.keys(),mean_tn,yerr=ci_tn,label='tn',fmt='o',color='#1f77b4')
# #plt.plot(m_tn.keys(),mean_tn,'^--',color='#1f77b4')
# data = {'Key':m1_t.keys(),
#         'Values':m1_tn.values()}
# tp_df = pd.DataFrame(data)
# tp_df = tp_df.explode('Values')
# tp_df = tp_df.reset_index(drop=True)
# tp_df.columns= ['Key','Values']
# sns.lineplot(data=tp_df,x='Key',y='Values',label='smart')

# data = {'Key':m2_t.keys(),
#         'Values':m2_tn.values()}
# tp_df = pd.DataFrame(data)
# tp_df = tp_df.explode('Values')
# tp_df = tp_df.reset_index(drop=True)
# tp_df.columns= ['Key','Values']
# sns.lineplot(data=tp_df,x='Key',y='Values',label='basic')

# plt.xscale('log')
# plt.title('True negative')
# plt.xlabel('Accuracy')
# plt.grid()
# plt.show()

ax = plt.figure()
#plt.plot(m_d.keys(),mean_d,'o--',label='distance', color='#1f77b4')
#plt.errorbar(m_d.keys(),mean_d,yerr=ci_d,fmt='o',color='#1f77b4')
data = {'Key':m1_t.keys(),
        'Values':m1_d.values()}
tp_df = pd.DataFrame(data)
tp_df = tp_df.explode('Values')
tp_df = tp_df.reset_index(drop=True)
tp_df.columns= ['Key','Values']
sns.lineplot(data=tp_df,x='Key',y='Values',label='smart')

data = {'Key':m2_t.keys(),
        'Values':m2_d.values()}
tp_df = pd.DataFrame(data)
tp_df = tp_df.explode('Values')
tp_df = tp_df.reset_index(drop=True)
tp_df.columns= ['Key','Values']
sns.lineplot(data=tp_df,x='Key',y='Values',label='basic')

ax.legend()
plt.xscale('log')
plt.yscale('log')
plt.xlabel('Accuracy')
plt.ylabel('Distance')
plt.grid()
plt.show()

ax = plt.figure()
#plt.plot(m_t.keys(),mean_t,'o--',label='time',color='#1f77b4')
#plt.errorbar(m_t.keys(),mean_t,yerr=ci_t,fmt='o', color='#1f77b4')
data = {'Key':m1_t.keys(),
        'Values':m1_t.values()}
tp_df = pd.DataFrame(data)
tp_df = tp_df.explode('Values')
tp_df = tp_df.reset_index(drop=True)
tp_df.columns= ['Key','Values']
sns.lineplot(data=tp_df,x='Key',y='Values',label='smart')

data = {'Key':m2_t.keys(),
        'Values':m2_t.values()}
tp_df = pd.DataFrame(data)
tp_df = tp_df.explode('Values')
tp_df = tp_df.reset_index(drop=True)
tp_df.columns= ['Key','Values']
sns.lineplot(data=tp_df,x='Key',y='Values',label='basic')
ax.legend()
plt.xscale('log')
plt.yscale('log')
plt.xlabel('Accuracy')
plt.ylabel('Time')
plt.show()

data = {'Key':d11.keys(),
        'Values':d11.values()}
tp_df = pd.DataFrame(data)
tp_df = tp_df.explode('Values')
tp_df = tp_df.reset_index(drop=True)
tp_df.columns= ['Key','Values']
sns.lineplot(data=tp_df,x='Key',y='Values',label='smart_1')

data = {'Key':d12.keys(),
        'Values':d12.values()}
tp_df = pd.DataFrame(data)
tp_df = tp_df.explode('Values')
tp_df = tp_df.reset_index(drop=True)
tp_df.columns= ['Key','Values']
sns.lineplot(data=tp_df,x='Key',y='Values',label='smart_2')

data = {'Key':d21.keys(),
        'Values':d21.values()}
tp_df = pd.DataFrame(data)
tp_df = tp_df.explode('Values')
tp_df = tp_df.reset_index(drop=True)
tp_df.columns= ['Key','Values']
sns.lineplot(data=tp_df,x='Key',y='Values',label='basic_1')

data = {'Key':d22.keys(),
        'Values':d22.values()}
tp_df = pd.DataFrame(data)
tp_df = tp_df.explode('Values')
tp_df = tp_df.reset_index(drop=True)
tp_df.columns= ['Key','Values']
sns.lineplot(data=tp_df,x='Key',y='Values',label='basic_2')

ax.legend()
plt.xscale('log')
plt.yscale('log')
plt.xlabel('Accuracy')
plt.ylabel('Distance covered')
plt.show()


