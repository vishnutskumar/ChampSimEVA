import matplotlib.pyplot as plt
import csv
import numpy as np
import imageio
import pandas as pd


MIN_COUNT = 0
MAX_COUNT = 340 #number of EVA data ranges
MAX_AGE = 307200
PLOT_FREQ = 20 # Plot every nth EVA range

data = pd.read_csv('../'+str(MAX_AGE)+'eva_ages.txt',header=None,sep=',').to_numpy()

print("Reading CSV Done \n")

id=0
for rec in np.arange(MIN_COUNT, MAX_COUNT, PLOT_FREQ):
    plt.figure(figsize=(6,6))
    for i in np.arange(rec, rec+1, 1):
        subset = data[i*MAX_AGE:((i+1)*MAX_AGE)-1,:]
        age = subset[:,0]
        eva = subset[:,1]
        hit_p = subset[:,2]
        exp_life = subset[:3]
        opp_cost = subset[:4]
        line_gain = subset[:5]

        plt.plot(age, eva, label='eva')
        plt.plot(age, hit_p, label='hit probability')
        plt.plot(age, opp_cost, label='opportunity cost')

        plt.xlabel("age")
        plt.ylabel("eva")
        plt.ylim([-10, 5])
        plt.grid()
        # plt.ylim([2.5, 3])
        plt.legend(loc='lower right')

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

imageio.mimsave('./example.gif', # output gif
                frames,          # array of input frames
                fps = 5)         # optional: frames per second
