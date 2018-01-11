import numpy as np
import matplotlib.pyplot as plt

k = np.loadtxt("example_get.txt")
d = np.sort(k)


k1 = np.loadtxt("example_put.txt")
d1 = np.sort(k1)

x_axis = []
y_axis = []
x1 = []
x2 = []

print "Get mean: " + str(np.mean(d)) 
print "Get 90 percentile: " + str(np.percentile(d, 90));
print "Get 99 percentile: " + str(np.percentile(d, 99));
 
print "Put Mean: " + str(np.mean(d1))
print "Put 90 percentile: " + str(np.percentile(d1, 90));
print "Put 99 percentile: " + str(np.percentile(d1, 99));
