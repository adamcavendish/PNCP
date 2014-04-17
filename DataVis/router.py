import sys
import pylab
import matplotlib
import numpy.core

if len(sys.argv) < 2:
    sys.stderr.write('Usage: ', sys.argv[0], ' router.log file')
    sys.exit(1)

# build data
data = []
for i in range(0, 26):
    data.append([])

# read data
counter = 0
dataFile = open(sys.argv[1])
for line in dataFile.readlines():
    counter = counter + 1

    line_sp = line.split(',')
    if len(line_sp) != 26:
        raise Exception("File probably is not router.log -- should be 26 columns each line")
    for i in range(0, 26):
        data[i].append(float(line_sp[i]))

    print '\rReading line: {}'.format(counter),
dataFile.close()

print '\nReading Finished!'

# plot it
figDpi = 100
figSizeW = 10.24
figSizeH = 7.68

matplotlib.pyplot.clf()
matplotlib.pyplot.figure(figsize=(figSizeW, figSizeH), dpi=figDpi)
maxnum = int(max(data[3]))
datalen = len(data[0])
queuePressure0 = [ float(data[1].count(i))/datalen for i in range(0, maxnum+1) ]
queuePressure1 = [ float(data[2].count(i))/datalen for i in range(0, maxnum+1) ]
xbar = range(0, maxnum+1)
matplotlib.pyplot.bar(xbar, +numpy.array(queuePressure0), \
        facecolor='blue' , edgecolor='white', label='dest-queue[0] number')
matplotlib.pyplot.bar(xbar, -numpy.array(queuePressure1), \
        facecolor='green', edgecolor='white', label='dest-queue[1] number')
for x,y in zip(xbar, queuePressure0):
    matplotlib.pyplot.text(x+0.4,  y+0.05, '%.4f' % y, ha='center', va= 'bottom')
for x,y in zip(xbar, queuePressure1):
    matplotlib.pyplot.text(x+0.4, -y-0.05, '%.4f' % y, ha='center', va= 'top')
matplotlib.pyplot.legend(loc='best')
matplotlib.pyplot.savefig('figures/Router-queueLength.png', format='png')
print 'Router-queueLength.png finished!'

matplotlib.pyplot.clf()
matplotlib.pyplot.figure(figsize=(figSizeW, figSizeH), dpi=figDpi)
matplotlib.pyplot.plot(data[0], data[4], label='router average dest-queue[0] length')
matplotlib.pyplot.plot(data[0], data[5], label='router average dest-queue[1] length')
matplotlib.pyplot.plot(data[0], data[6], label='router average dest-queue all length')
matplotlib.pyplot.legend(loc='best')
matplotlib.pyplot.savefig('figures/Router-avgQueueLength.png', format='png')
print 'Router-avgQueueLength.png finished!'

matplotlib.pyplot.clf()
matplotlib.pyplot.figure(figsize=(figSizeW, figSizeH), dpi=figDpi)
matplotlib.pyplot.plot(data[0], data[7], label='package from 0 number')
matplotlib.pyplot.plot(data[0], data[8], label='package from 1 number')
matplotlib.pyplot.plot(data[0], data[9], label='package from all number')
matplotlib.pyplot.legend(loc='best')
matplotlib.pyplot.savefig('figures/Router-packageFrom.png', format='png')
print 'Router-packageFrom.png finished!'

matplotlib.pyplot.clf()
matplotlib.pyplot.figure(figsize=(figSizeW, figSizeH), dpi=figDpi)
matplotlib.pyplot.plot(data[0], data[10], label='forward package from 0 number')
matplotlib.pyplot.plot(data[0], data[11], label='forward package from 1 number')
matplotlib.pyplot.plot(data[0], data[12], label='forward package from all number')
matplotlib.pyplot.legend(loc='best')
matplotlib.pyplot.savefig('figures/Router-forwardPackage.png', format='png')
print 'Router-forwardPackage.png finished!'

matplotlib.pyplot.clf()
matplotlib.pyplot.figure(figsize=(figSizeW, figSizeH), dpi=figDpi)
matplotlib.pyplot.plot(data[0], data[13], label='forward package from 0 in %')
matplotlib.pyplot.plot(data[0], data[14], label='forward package from 1 in %')
matplotlib.pyplot.plot(data[0], data[15], label='forward package from all in %')
matplotlib.pyplot.legend(loc='best')
matplotlib.pyplot.savefig('figures/Router-forwardPackagePct.png', format='png')
print 'Router-forwardPackagePct.png finished!'

matplotlib.pyplot.clf()
matplotlib.pyplot.figure(figsize=(figSizeW, figSizeH), dpi=figDpi)
matplotlib.pyplot.plot(data[0], data[16], label='code package number')
matplotlib.pyplot.legend(loc='best')
matplotlib.pyplot.savefig('figures/Router-codePackage.png', format='png')
print 'Router-codePackage.png finished!'

matplotlib.pyplot.clf()
matplotlib.pyplot.figure(figsize=(figSizeW, figSizeH), dpi=figDpi)
matplotlib.pyplot.plot(data[0], data[17], label='code package in %')
matplotlib.pyplot.legend(loc='best')
matplotlib.pyplot.savefig('figures/Router-codePackagePct.png', format='png')
print 'Router-codePackagePct.png finished!'

matplotlib.pyplot.clf()
matplotlib.pyplot.figure(figsize=(figSizeW, figSizeH), dpi=figDpi)
matplotlib.pyplot.plot(data[0], data[18], label='lost package from 0 number')
matplotlib.pyplot.plot(data[0], data[19], label='lost package from 1 number')
matplotlib.pyplot.plot(data[0], data[20], label='lost package from all number')
matplotlib.pyplot.legend(loc='best')
matplotlib.pyplot.savefig('figures/Router-lostPackage.png', format='png')
print 'Router-lostPackage.png finished!'

matplotlib.pyplot.clf()
matplotlib.pyplot.figure(figsize=(figSizeW, figSizeH), dpi=figDpi)
matplotlib.pyplot.plot(data[0], data[21], label='lost package from 0 in %')
matplotlib.pyplot.plot(data[0], data[22], label='lost package from 1 in %')
matplotlib.pyplot.plot(data[0], data[23], label='lost package from all in %')
matplotlib.pyplot.legend(loc='best')
matplotlib.pyplot.savefig('figures/Router-lostPackagePct.png', format='png')
print 'Router-lostPackagePct.png finished!'

matplotlib.pyplot.clf()
matplotlib.pyplot.figure(figsize=(figSizeW, figSizeH), dpi=figDpi)
matplotlib.pyplot.plot(data[0], data[24], label='wait time')
matplotlib.pyplot.legend(loc='best')
matplotlib.pyplot.savefig('figures/Router-waitTime.png', format='png')
print 'Router-waitTime.png finished!'

matplotlib.pyplot.clf()
matplotlib.pyplot.figure(figsize=(figSizeW, figSizeH), dpi=figDpi)
matplotlib.pyplot.plot(data[0], data[25], label='wait time in %')
matplotlib.pyplot.legend(loc='best')
matplotlib.pyplot.savefig('figures/Router-waitTimePct.png', format='png')
print 'Router-waitTimePct.png finished!'

