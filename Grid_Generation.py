import numpy as np
import pandas as pd
import json
from matplotlib import pyplot as plt
plt.style.use('dark_background')

# Use this to quickly redefine grid and config variables

grid_size_x = 2
grid_size_y = 256
rho = np.zeros((grid_size_x,grid_size_y))
u = np.zeros((grid_size_x,grid_size_y))
v = np.zeros((grid_size_x,grid_size_y))
boundary = np.zeros((grid_size_x,grid_size_y),dtype='int')
indices = np.zeros((grid_size_x,grid_size_y,2),dtype='int')
for i in range(grid_size_x):
    for j in range(grid_size_y):
        indices[i,j] = [i,j]


boundary[:,0] = 1
boundary[:,-1] = 1
rho[:,:] = 1.

# u[:,32] = -0.01
# v[:,32] = 0.01
# rho[128,128] = 2.
# rho[16,1] = 2.
# rho[32+16,1] = 2.
# centerx = 32
# centery = 32
# u[centerx-4:centerx+7,centery-4:centery+7] = 0.

output = pd.DataFrame(columns=['xi','yi','rho','u','v','boundary'])

output['xi'] = indices.reshape((-1,2))[:,0]
output['yi'] = indices.reshape((-1,2))[:,1]
output['rho'] = rho.reshape(-1)
output['u'] = u.reshape(-1)
output['v'] = v.reshape(-1)
output['boundary'] = boundary.reshape(-1)

output.to_csv('grid_variables.csv',index=False)


# Config variables
config = {}
config['grid_size_x'] = grid_size_x
config['grid_size_y'] = grid_size_y
config['frame_rate'] = 0
config['dt'] = 0.1
config['dx'] = 1./grid_size_x
config['dy'] = 1./grid_size_y
config['viscosity'] = 1.8e-5
config['c'] = 347.0
config['force'] = 1e-4
config['run_graphics'] = 0
config["tolerance"] = 1e-5

with open('config.json','w') as fp:
    json.dump(config, fp, indent='\t')
