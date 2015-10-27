#!/usr/bin/python
import pylab
import numpy
import threading

global fN
t0 = 0
tN = 100
h = 0.001

fN = 0

def van_der_pol(x0, e):
	global fN
	N = int((tN - t0)/h)
	t = numpy.arange (t0, tN, h)
	x = numpy.arange (t0, tN, h)
	z = numpy.arange (t0, tN, h)
	
	x[0] = x0
	
	for i in range(1, N):
		#t.append(i*h)
		x[i] = (z[i-1]*h + x[i-1])
		z[i] = (h*(e*(1-(x[i-1])**2)*z[i-1]-x[i-1])+z[i-1])
	
	fN += 1
	pylab.figure(fN)
	pylab.subplot(1, 2, 1)
	pylab.plot (t, z, ',r')
	pylab.xlabel('t')
	pylab.ylabel('z(t)')
	pylab.title("Van der Pol x0 = {}, e = {}".format(x0, e))
	
	pylab.subplot(1, 2, 2)
	pylab.plot (x, z, ',r')
	pylab.xlabel('x(t)')
	pylab.ylabel('z(t)')
	pylab.title("Van der Pol x0 = {}, e = {}".format(x0, e))


van_der_pol(0.1, 0.05)
van_der_pol(0.1, 0.1)
van_der_pol(0.1, 0.2)
van_der_pol(0.1, 0.3)
van_der_pol(0.1, 0.8)
van_der_pol(0.1, 2)

pylab.show()
