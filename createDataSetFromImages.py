import subprocess
import sys
import os
from os import walk
from os import path

path = str(sys.argv[1])
numThreads = sys.argv[2]
percBlocks2Save = sys.argv[3]
isTraining = str(sys.argv[4]) == 'TRAIN'

setFolder = []

if isTraining:
    setFolder = 'trainingSet/'
else:
    setFolder = 'testSet/'

files = []
for (dirpath, dirnames, filenames) in walk(path):
        files.extend(filenames)
        break

print 'Found the following images in ', path, ':\n', files

print 'Creating dataset...'

for f in files:
    bsName = os.path.splitext(f)[0]
    ext = os.path.splitext(f)[1]
    command = './nnBM3D 0.0 ' + str(numThreads) \
            + ' ' + path + 'noisy/' + f \
            + ' ' + path + 'tmp/' + bsName + '_denoised' +  ext \
            + ' ' + path + f \
            + ' ' + path + setFolder + 'Xs/' + bsName + '.bin ' \
            + ' ' + path + setFolder + 'Ys/' + bsName + '.bin ' \
            + 'WRITE ' + str(percBlocks2Save)
    print command
    subprocess.call(command, shell=True)



