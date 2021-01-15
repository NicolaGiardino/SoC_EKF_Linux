#############################################################################################
# This file is part of The SoC_EKF_Linux Project.                                           #
#                                                                                           #
# Copyright � 2020-2021 By Nicola di Gruttola Giardino. All rights reserved.                #
# @ mail: nicoladgg@protonmail.com                                                          #
#                                                                                           #
# SoC_EKF_Linux is free software: you can redistribute it and / or modify                   #
# it under the terms of the GNU General Public License as published by                      #
# the Free Software Foundation, either version 3 of the License, or *                       #
# (at your option) any later version.                                                       #
#                                                                                           #
# SoC_EKF_Linux is distributed in the hope that it will be useful,                          #
# but WITHOUT ANY WARRANTY                                                                  #
# without even the implied warranty of                                                      #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                             #
# GNU General Public License for more details.                                              #
#                                                                                           #
# You should have received a copy of the GNU General Public License                         #
# along with The SoC_EKF_Linux Project.  If not, see < https: // www.gnu.org/licenses/>.    #
#                                                                                           #
# In case of use of this project, I ask you to mention me, to whom it may concern.          #
#############################################################################################
import numpy as np
import matplotlib.pyplot as plt

data1 = np.loadtxt('./logs/SOC.txt')

plt.plot(data1[:, 1], data1[:, 0] * 100)

data2 = np.loadtxt('./logs/SOC_True.txt')

plt.plot(data2[:, 1], data2[:, 0] * 100)
plt.xlabel('Time')  
plt.ylabel('Est. SOC vs True SOC')  
plt.title("SOC graph")

plt.show()

err = (data2[:, 0] - data1[:, 0]) * 100

plt.plot(data2[:, 1], err[:])
plt.xlabel('Time')  
plt.ylabel('Error')  
plt.title("Error graph")

plt.show()
