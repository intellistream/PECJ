#!/usr/bin/env python3
import csv
import numpy as np
import matplotlib.pyplot as plt
import accuBar as accuBar
import groupBar as groupBar
import groupBar2 as groupBar2
import groupLine as groupLine
from autoParase import *
import itertools as it
import os

import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import pylab
from matplotlib.font_manager import FontProperties
from matplotlib.ticker import LogLocator, LinearLocator
import os
import pandas as pd
import sys
from OoOCommon import *

OPT_FONT_NAME = 'Helvetica'
TICK_FONT_SIZE = 22
LABEL_FONT_SIZE = 28
LEGEND_FONT_SIZE = 30
LABEL_FP = FontProperties(style='normal', size=LABEL_FONT_SIZE)
LEGEND_FP = FontProperties(style='normal', size=LEGEND_FONT_SIZE)
TICK_FP = FontProperties(style='normal', size=TICK_FONT_SIZE)

MARKERS = (['*', '|', 'v', "^", "", "h", "<", ">", "+", "d", "<", "|", "", "+", "_"])
# you may want to change the color map for different figures
COLOR_MAP = (
    '#B03A2E', '#2874A6', '#239B56', '#7D3C98', '#FFFFFF', '#F1C40F', '#F5CBA7', '#82E0AA', '#AEB6BF', '#AA4499')
# you may want to change the patterns for different figures
PATTERNS = (["////", "o", "", "||", "-", "//", "\\", "o", "O", "////", ".", "|||", "o", "---", "+", "\\\\", "*"])
LABEL_WEIGHT = 'bold'
LINE_COLORS = COLOR_MAP
LINE_WIDTH = 3.0
MARKER_SIZE = 15.0
MARKER_FREQUENCY = 1000

matplotlib.rcParams['ps.useafm'] = True
matplotlib.rcParams['pdf.use14corefonts'] = True
matplotlib.rcParams['xtick.labelsize'] = TICK_FONT_SIZE
matplotlib.rcParams['ytick.labelsize'] = TICK_FONT_SIZE
matplotlib.rcParams['font.family'] = OPT_FONT_NAME
matplotlib.rcParams['pdf.fonttype'] = 42


def runSkew(exePath, skew, resultPath):
    # resultFolder="skewTests"
    configFname = "config_skew_" + str(skew) + ".csv"
    configTemplate = "config.csv"
    # clear old files
    os.system("cd " + exePath + "&& rm *.csv")
    # prepare new file
    editConfig(configTemplate, exePath + configFname, "maxArrivalSkewMs", skew)
    # run
    os.system("cd " + exePath + "&& ./benchmark " + configFname)
    # copy result
    os.system("rm -rf " + resultPath + "/" + str(skew))
    os.system("mkdir " + resultPath + "/" + str(skew))
    os.system("cd " + exePath + "&& cp *.csv " + resultPath + "/" + str(skew))


def runskewVector(exePath, skewVec, resultPath):
    for i in skewVec:
        runSkew(exePath, i, resultPath)


def readResultSkew(skew, resultPath):
    resultFname = resultPath + "/" + str(skew) + "/default_general.csv"
    avgLat = readConfig(resultFname, "AvgLatency")
    lat95 = readConfig(resultFname, "95%Latency")
    thr = readConfig(resultFname, "Throughput")
    err = readConfig(resultFname, "Error")
    return avgLat, lat95, thr, err


def readResultVectorSkew(skewVec, resultPath):
    avgLatVec = []
    lat95Vec = []
    thrVec = []
    errVec = []
    compVec = []
    for i in skewVec:
        avgLat, lat95, thr, err = readResultSkew(i, resultPath)
        avgLatVec.append(float(avgLat) / 1000.0)
        lat95Vec.append(float(lat95) / 1000.0)
        thrVec.append(float(thr) / 1000.0)
        errVec.append(abs(float(err)))
        compVec.append(1 - abs(float(err)))
    return avgLatVec, lat95Vec, thrVec, errVec, compVec


def main():
    exeSpace = os.path.abspath(os.path.join(os.getcwd(), "../..")) + "/"
    resultPath = os.path.abspath(os.path.join(os.getcwd(), "../..")) + "/results/skewTest/"
    figPath = os.path.abspath(os.path.join(os.getcwd(), "../..")) + "/figures/"
    configTemplate = exeSpace + "config.csv"
    skewVec = [1, 2, 5, 10, 15, 20]
    skewVecDisp = np.array(skewVec)
    skewVecDisp = skewVecDisp
    print(configTemplate)
    # run
    if (len(sys.argv) < 2):
        os.system("rm -rf " + resultPath)
        os.system("mkdir " + resultPath)
        runskewVector(exeSpace, skewVec, resultPath)
    avgLatVec, lat95Vec, thrVec, errVec, compVec = readResultVectorSkew(skewVec, resultPath)
    os.system("mkdir " + figPath)
    draw2yLine("max skewness (ms)", skewVecDisp, lat95Vec, errVec, "95% Latency (ms)", "Error", "ms", "",
               figPath + "skew_lat")
    draw2yLine("max skewness (ms)", skewVecDisp, thrVec, errVec, "Throughput (KTp/s)", "Error", "KTp/s", "",
               figPath + "skew_thr")
    draw2yLine("max skewness (ms)", skewVecDisp, lat95Vec, compVec, "95% Latency (ms)", "Completeness", "ms", "",
               figPath + "skew_comp")
    print(errVec)
    # readResultSkew(50,resultPath)


if __name__ == "__main__":
    main()