import matplotlib.pyplot as plt
import csv
import numpy as np
import imageio
import pandas as pd


MIN_COUNT = 0
MAX_COUNT = 228 #number of EVA data ranges
MAX_AGE = 10*1024
PLOT_FREQ = 20 # Plot every nth EVA range

data = pd.read_csv('../'+str(MAX_AGE)+'eva_ages.txt',header=None,sep=',').to_numpy()

print("Reading CSV Done \n")
age = np.arange(0,MAX_AGE-1)
id=0
for rec in np.arange(MIN_COUNT, MAX_COUNT, PLOT_FREQ):
    plt.figure(figsize=(6,6))
    for i in np.arange(rec, rec+1, 1):
        subset = data[i*MAX_AGE:((i+1)*MAX_AGE)-1,:]        
        eva = subset[:,0]
        hit_p = subset[:,1]
        # exp_life = subset[:,3]
        opp_cost = subset[:,2]
        # line_gain = subset[:,5]

        plt.plot(age, eva, label='eva')
        plt.plot(age, hit_p, label='hit probability')
        plt.plot(age, opp_cost, label='opportunity cost')
        plt.title("Image :"+str(i))
        plt.xlabel("age")
        plt.ylabel("eva")
        plt.ylim([-3, 3])
        # plt.xlim([0, 1000])
        plt.grid()
        # plt.ylim([2.5, 3])
        plt.legend(loc='upper right')

    plt.savefig(f'./img/img_{id}.png',
                transparent = False,
                facecolor = 'white')
    plt.close()
    id+=1
    print("Image Number : "+str(id)+"\n")

frames = []
for t in range(id):
    image = imageio.v2.imread(f'./img/img_{t}.png')
    frames.append(image)

imageio.mimsave('./eva.gif', # output gif
                frames,          # array of input frames
                fps = 3)         # optional: frames per second


data = pd.read_csv('../'+str(MAX_AGE)+'ewmevents_ages.txt',header=None,sep=',').to_numpy()

print("Reading 2nd CSV Done \n")
age = np.arange(0,MAX_AGE-1)
id=0
for rec in np.arange(MIN_COUNT, MAX_COUNT, PLOT_FREQ):
    plt.figure(figsize=(6,6))
    for i in np.arange(rec, rec+1, 1):
        subset = data[i*MAX_AGE:((i+1)*MAX_AGE)-1,:]        
        eva = subset[:,0]
        hit_p = subset[:,1]
        # exp_life = subset[:,3]
        opp_cost = subset[:,2]
        # line_gain = subset[:,5]

        plt.plot(age, eva, label='Hits')
        plt.plot(age, hit_p, label='Evictions')
        plt.plot(age, opp_cost, label='Total Events Above')
        plt.title("Image :"+str(i))
        plt.xlabel("age")
        plt.ylabel("eva")
        plt.ylim([0, 2000])
        # plt.xlim([0, 1000])
        plt.grid()
        # plt.ylim([2.5, 3])
        plt.legend(loc='upper right')

    plt.savefig(f'./img2/img_{id}.png',
                transparent = False,
                facecolor = 'white')
    plt.close()
    id+=1
    print("Image Number : "+str(id)+"\n")

frames = []
for t in range(id):
    image = imageio.v2.imread(f'./img2/img_{t}.png')
    frames.append(image)

imageio.mimsave('./events.gif', # output gif
                frames,          # array of input frames
                fps = 3)         # optional: frames per second
