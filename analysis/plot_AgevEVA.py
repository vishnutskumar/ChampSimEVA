import matplotlib.pyplot as plt
import csv
import numpy as np
import imageio


MIN_COUNT = 0
MAX_COUNT = 300 #number of EVA data ranges
MAX_AGE = 500
PLOT_FREQ = 20 # Plot every nth EVA range


with open('../'+str(MAX_AGE)+'eva_ages.txt', newline='') as csvfile:
    reader = csv.reader(csvfile, delimiter=',')
    data = list(reader)

data = np.array(data, dtype=float)

id=0
for rec in np.arange(MIN_COUNT, MAX_COUNT, PLOT_FREQ):
    plt.figure(figsize=(6,6))
    for i in np.arange(rec, rec+2, 1):
        subset = data[i*MAX_AGE:((i+1)*MAX_AGE)-1,:]
        age = subset[:,0];
        eva = subset[:,1];
        if i%2:
            plt.plot(age, eva, label = "REUSED");
        else:
            plt.plot(age, eva, label = "NON-REUSED");
        plt.xlabel("age")
        plt.ylabel("eva")
        plt.ylim([-10, 5])
        # plt.ylim([2.5, 3])
        plt.legend(loc='lower right')

    plt.savefig(f'./img/img_{id}.png',
                transparent = False,
                facecolor = 'white')
    plt.close()
    id+=1

frames = []
for t in range(id):
    image = imageio.v2.imread(f'./img/img_{t}.png')
    frames.append(image)

imageio.mimsave('./example.gif', # output gif
                frames,          # array of input frames
                fps = 5)         # optional: frames per second
