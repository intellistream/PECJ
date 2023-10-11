import itertools as it
import os

import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import pylab
from matplotlib.font_manager import FontProperties
from matplotlib.ticker import MaxNLocator
from matplotlib.font_manager import FontProperties
from matplotlib.ticker import LinearLocator, LogLocator, MaxNLocator, ScalarFormatter
from numpy import double
import matplotlib.patches as patches

OPT_FONT_NAME = 'Helvetica'
TICK_FONT_SIZE = 26
LABEL_FONT_SIZE = 26
LEGEND_FONT_SIZE = 20
LABEL_FP = FontProperties(style='normal', size=LABEL_FONT_SIZE)
LEGEND_FP = FontProperties(style='normal', size=LEGEND_FONT_SIZE)
TICK_FP = FontProperties(style='normal', size=TICK_FONT_SIZE)

MARKERS = (['o', 's', 'v', "^", "h", "v", ">", "x", "d", "<", "|", "p", "+", "_", "%", "|", "|", "|", "|", "|"])
# you may want to change the color map for different figures
COLOR_MAP = (
    '#F15854', '#5DA5DA', '#60BD68', '#B276B2', '#DECF3F', '#F17CB0', '#B2912F', '#FAA43A', '#AFAFAF', '#087878',
    '#783456',
    '#560012', '#431256', "#00AABB", "#AA00BB")
# you may want to change the patterns for different figures
PATTERNS = (["|", "\\", "/", "+", "-", ".", "*", "x", "o", "O", "////", ".", "|||", "o", "---", "+", "\\\\", "*"])
LABEL_WEIGHT = 'bold'
LINE_COLORS = COLOR_MAP
LINE_WIDTH = 3.0
MARKER_SIZE = 13.0
MARKER_FREQUENCY = 1000

matplotlib.rcParams['ps.useafm'] = True
matplotlib.rcParams['pdf.use14corefonts'] = True
matplotlib.rcParams['xtick.labelsize'] = TICK_FONT_SIZE
matplotlib.rcParams['ytick.labelsize'] = TICK_FONT_SIZE
matplotlib.rcParams['font.family'] = OPT_FONT_NAME

FIGURE_FOLDER = '/data1/xtra/results/figure'


# there are some embedding problems if directly exporting the pdf figure using matplotlib.
# so we generate the eps format first and convert it to pdf.
def ConvertEpsToPdf(dir_filename):
    os.system("epstopdf --outfile " + dir_filename + ".pdf " + dir_filename + ".eps")
    os.system("rm -rf " + dir_filename + ".eps")


def DrawLegend(legend_labels, filename):
    fig = pylab.figure()
    ax1 = fig.add_subplot(111)
    FIGURE_LABEL = legend_labels
    LINE_WIDTH = 8.0
    MARKER_SIZE = 12.0
    LEGEND_FP = FontProperties(style='normal', size=26)

    figlegend = pylab.figure(figsize=(12, 0.5))
    idx = 0
    lines = [None] * (len(FIGURE_LABEL))
    data = [1]
    x_values = [1]

    idx = 0
    for group in range(len(FIGURE_LABEL)):
        lines[idx], = ax1.plot(x_values, data,
                               color=LINE_COLORS[idx], linewidth=LINE_WIDTH,
                               marker=MARKERS[idx], markersize=MARKER_SIZE, label=str(group))
        idx = idx + 1

    # LEGEND
    figlegend.legend(lines, FIGURE_LABEL, prop=LEGEND_FP,
                     loc=1, ncol=len(FIGURE_LABEL), mode="expand", shadow=False,
                     frameon=False, borderaxespad=0.0, handlelength=2)

    if not os.path.exists(FIGURE_FOLDER):
        os.makedirs(FIGURE_FOLDER)
    # no need to export eps in this case.
    figlegend.savefig(filename + '.pdf')


def DrawFigureCdf(xvalues, yvalues, legend_labels, x_label, y_label, y_min, y_max, filename, allow_legend):
    fig = plt.figure(figsize=(10, 4))

    markers = ['s', 'o', '^', 'v', '+', '*', ',', 'x', 'p', '1', '2', 'o']
    linestyles = ['-.', '-.', 'dotted', 'dotted', 'dotted', 'dotted', 'dotted', ':', 'dashed', 'dotted', 'dotted', '-']
    colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd', '#8c564b', '#e377c2', '#7f7f7f', '#bcbd22',
              '#17becf', '#1f77b4']
    linewidth = 2

    FIGURE_LABEL = legend_labels
    x_values = xvalues
    y_values = yvalues

    lines = [None] * (len(FIGURE_LABEL))
    for i in range(len(y_values)):
        lines[i], = plt.plot(x_values[i], y_values[i], color=colors[i], \
                             linewidth=linewidth, marker=markers[i], \
                             markersize=9, linestyle=linestyles[i], \
                             label=FIGURE_LABEL[i])

    # for i in range(len(x_values)):
    #     plt.axvline(x=x_values[i][0], linestyle='--', color='gray')
    # plt.xticks(x_values.flatten())

    if allow_legend:
        plt.legend(lines,
                   FIGURE_LABEL,
                   fontsize=LEGEND_FONT_SIZE,
                   loc='upper center',
                   ncol=1,
                   bbox_to_anchor=(0.81, 0.65),
                   edgecolor='black',borderaxespad=1,
                   frameon=True,shadow=True)
    plt.xlabel(x_label, fontsize=20)
    plt.ylabel(y_label, fontsize=20)
    plt.axhline(y=95, color='red', linestyle='--')
    plt.text(170.0, 96, "95%", fontsize=TICK_FONT_SIZE, ha='center')
    #plt.ylim(y_min, y_max)
    plt.grid(axis='y', color='gray', alpha=0.5, linewidth=0.5)

    #plt.show()

    fig.savefig(filename + ".pdf", bbox_inches='tight')

# draw a line chart
def DrawFigure2(xvalues, yvalues, legend_labels, x_label, y_label, y_min, y_max, filename, allow_legend):
    fig = plt.figure(figsize=(10, 4))

    markers = ['s', 'o', '^', 'v', '+', '*', ',', 'x', 'p', '1', '2', 'o']
    linestyles = ['-.', '-.', 'dotted', 'dotted', 'dotted', 'dotted', 'dotted', ':', 'dashed', 'dotted', 'dotted', '-']
    colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd', '#8c564b', '#e377c2', '#7f7f7f', '#bcbd22',
              '#17becf', '#1f77b4']
    linewidth = 2

    FIGURE_LABEL = legend_labels
    x_values = xvalues
    y_values = yvalues

    lines = [None] * (len(FIGURE_LABEL))
    for i in range(len(y_values)):
        lines[i], = plt.plot(x_values[i], y_values[i], color=colors[i], \
                             linewidth=linewidth, marker=markers[i], \
                             markersize=9, linestyle=linestyles[i], \
                             label=FIGURE_LABEL[i])

    # for i in range(len(x_values)):
    #     plt.axvline(x=x_values[i][0], linestyle='--', color='gray')
    # plt.xticks(x_values.flatten())

    if allow_legend:
        plt.legend(lines,
                   FIGURE_LABEL,
                   fontsize=12,
                   loc='upper center',
                   ncol=3,
                   bbox_to_anchor=(0.5, 1.15),
                   borderaxespad=0.,
                   frameon=True)
    plt.xlabel(x_label, fontsize=20)
    plt.ylabel(y_label, fontsize=20)

    plt.ylim(y_min, y_max)
    plt.grid(axis='y', color='gray', alpha=0.5, linewidth=0.5)

    #plt.show()

    fig.savefig(filename + ".pdf", bbox_inches='tight')

# def DrawFigure2(xvalues, yvalues, legend_labels, x_label, y_label, y_min, y_max, filename, allow_legend):
#     # you may change the figure size on your own.
#     fig = plt.figure(figsize=(10, 3))
#     figure = fig.add_subplot(111)
#
#     FIGURE_LABEL = legend_labels
#
#     x_values = xvalues
#     y_values = yvalues
#
#     lines = [None] * (len(FIGURE_LABEL))
#     for i in range(len(y_values)):
#         lines[i], = figure.plot(x_values[i], y_values[i], color=LINE_COLORS[i], \
#                                 linewidth=LINE_WIDTH, marker=MARKERS[i], \
#                                 markersize=MARKER_SIZE, label=FIGURE_LABEL[i])
#
#     # sometimes you may not want to draw legends.
#     if allow_legend == True:
#         plt.legend(lines,
#                    FIGURE_LABEL,
#                    prop=LEGEND_FP,
#                    loc='upper center',
#                    ncol=3,
#                    # mode='expand',
#                    bbox_to_anchor=(0.55, 1.6), shadow=False,
#                    columnspacing=0.1,
#                    frameon=True, borderaxespad=0.0, handlelength=1.5,
#                    handletextpad=0.1,
#                    labelspacing=0.1)
#     # plt.xscale('log')
#     # plt.yscale('log')
#     # plt.yscale('log')
#
#     # you may control the limits on your own.
#
#     # lt.ylim(y_min, y_max)
#
#     plt.grid(axis='y', color='gray')
#     # figure.yaxis.set_major_locator(LogLocator(base=10))
#     # figure.xaxis.set_major_locator(LogLocator(base=10))
#
#     # figure.get_xaxis().set_tick_params(direction='in', pad=10)
#     # figure.get_yaxis().set_tick_params(direction='in', pad=10)
#
#     plt.xlabel(x_label, fontproperties=LABEL_FP)
#     plt.ylabel(y_label, fontproperties=LABEL_FP)
#
#     size = fig.get_size_inches()
#     dpi = fig.get_dpi()
#
#     plt.savefig(filename + ".pdf", bbox_inches='tight')


# draw a line chart
def DrawFigureYnormal(xvalues, yvalues, legend_labels, x_label, y_label, y_min, y_max, filename, allow_legend):
    # you may change the figure size on your own.
    fig = plt.figure(figsize=(10, 3))
    figure = fig.add_subplot(111)

    FIGURE_LABEL = legend_labels

    x_values = xvalues
    y_values = yvalues

    lines = [None] * (len(FIGURE_LABEL))
    for i in range(len(y_values)):
        lines[i], = figure.plot(x_values[i], y_values[i], color=LINE_COLORS[i], \
                                linewidth=LINE_WIDTH, marker=MARKERS[i], \
                                markersize=MARKER_SIZE, label=FIGURE_LABEL[i], markeredgecolor='k')

    # sometimes you may not want to draw legends.
    if allow_legend == True:
        plt.legend(lines,
                   FIGURE_LABEL,
                   prop=LEGEND_FP,
                   loc='upper center',
                   ncol=3,
                   bbox_to_anchor=(0.55, 1.5), shadow=False,
                   columnspacing=0.1,
                   frameon=True, borderaxespad=0, handlelength=1.2,
                   handletextpad=0.1,
                   labelspacing=0.1)
    # plt.xscale('log')
    # plt.yscale('log')
    # plt.yscale('log')

    # you may control the limits on your own.

    # plt.ylim(y_min, y_max)

    plt.grid(axis='y', color='gray')
    # figure.yaxis.set_major_locator(LogLocator(base=10))
    # figure.xaxis.set_major_locator(LogLocator(base=10))
    plt.xticks(fontsize=TICK_FONT_SIZE)
    figure.get_xaxis().set_tick_params(direction='in', pad=10)
    figure.get_yaxis().set_tick_params(direction='in', pad=10)
    # Create a rectangle with bias lines
    rectangle = patches.Rectangle((6.0, 0.00), 2.5, 0.2, edgecolor='black', hatch='\\', fill=False)
    figure.text(7.0, 0.21, "user demand", fontsize=TICK_FONT_SIZE, ha='center')
    figure.add_patch(rectangle)
    plt.xlabel(x_label, fontproperties=LABEL_FP)
    plt.ylabel(y_label, fontproperties=LABEL_FP)

    size = fig.get_size_inches()
    dpi = fig.get_dpi()

    plt.savefig(filename + ".pdf", bbox_inches='tight')


# draw a line chart
def DrawFigureTradeOff(xvalues, yvalues, legend_labels, x_label, y_label, y_min, y_max, filename, allow_legend):
    # you may change the figure size on your own.
    fig = plt.figure(figsize=(10, 4))
    figure = fig.add_subplot(111)
    markers = ['s', 'o', '^', 'v', '+', '*', ',', 'x', 'p', '1', '2', 'o']
    linestyles = ['-.', '-.', 'dotted', 'dotted', 'dotted', 'dotted', 'dotted', ':', 'dashed', 'dotted', 'dotted', '-']
    colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd', '#8c564b', '#e377c2', '#7f7f7f', '#bcbd22',
              '#17becf', '#1f77b4']
    linewidth = 2

    FIGURE_LABEL = legend_labels
  
    x_values =xvalues
    print(x_values[0])
    y_values = yvalues
    #FIGURE_LABEL.append('User demand')
    lines = [None] * (len(FIGURE_LABEL))
    for i in range(len(y_values)):
        lines[i], = plt.plot(x_values[i], y_values[i], color=colors[i], \
                             linewidth=linewidth, marker=markers[i], \
                             markersize=9, linestyle=linestyles[i], \
                             label=FIGURE_LABEL[i])
    lines[len(FIGURE_LABEL)-1], = plt.plot(x_values[0], y_values[0]+5, color='#ffffff', \
                             linewidth=0.001, 
                             label=FIGURE_LABEL[len(FIGURE_LABEL)-1])

    # lines = [None] * (len(FIGURE_LABEL))
    # for i in range(len(y_values)):
    #     lines[i], = figure.plot(x_values[i], y_values[i], color=LINE_COLORS[i], \
    #                             linewidth=LINE_WIDTH, marker=MARKERS[i], \
    #                             markersize=MARKER_SIZE, label=FIGURE_LABEL[i], markeredgecolor='k')

    # sometimes you may not want to draw legends.
    #FIGURE_LABEL=FIGURE_LABEL.append('User Demand')
    if allow_legend == True:
        plt.legend(lines,
                   FIGURE_LABEL,
                   prop=LEGEND_FP,
                   loc='upper center',
                   ncol=1,
                   bbox_to_anchor=(0.815, 1.06),
                   columnspacing=0.1,
                    shadow=True,frameon=True,edgecolor='black',borderaxespad=1,
                   handletextpad=0.1,
                   labelspacing=0.1)
    #plt.xscale('log')
    # plt.yscale('log')
    # plt.yscale('log')

    # you may control the limits on your own.

    plt.ylim(0, 100.0)

    plt.grid(axis='y', color='0.1', alpha=0.5, linewidth=2.0)
    #figure.xaxis.set_major_locator(LogLocator(base=10))
    plt.xticks(fontsize=TICK_FONT_SIZE)
    plt.yticks(fontsize=TICK_FONT_SIZE)
    figure.get_xaxis().set_tick_params(direction='in', pad=10)
    figure.get_yaxis().set_tick_params(direction='in', pad=10)
    plt.xlabel(x_label, fontsize=TICK_FONT_SIZE)
    plt.ylabel(y_label, fontsize=TICK_FONT_SIZE)
    xmin=0
    rectangle = patches.Rectangle((xmin, 0.00),200-xmin , 20,  color = '0.85',alpha = 0.5  )
    #figure.text((xmin+8.5)/2, 10, "user demand", fontsize=TICK_FONT_SIZE, ha='center')
    figure.add_patch(rectangle)
    rectangle = patches.Rectangle((770.2, 50),80 , 5,  color = '0.85',alpha = 0.5, zorder=65535 )
    #figure.text((xmin+8.5)/2, 10, "user demand", fontsize=TICK_FONT_SIZE, ha='center')
    figure.add_patch(rectangle)
    # the bound of latency
    figure.annotate('', xy=(190, 22), xytext=(10,22),
            arrowprops=dict(arrowstyle='<-', color='black', lw=1.5))
    # Draw a rectangle to represent the bounded vertical line
    rect1 = patches.Rectangle((10, 22), 0.01, 3, linewidth=1.5, edgecolor='red', facecolor='none')
    figure.add_patch(rect1)
    rect1 = patches.Rectangle((190, 22), 0.01, 3, linewidth=1.5, edgecolor='red', facecolor='none')
    figure.add_patch(rect1)
    figure.text(120, 24,r"$\leq 200 ms$", fontsize=TICK_FONT_SIZE, ha='center')
    figure.axvline(x=200, color='red', linestyle='-.', linewidth=0.8)
    # the bound of error
    figure.annotate('', xy=(210, 19), xytext=(210,4),
            arrowprops=dict(arrowstyle='<-', color='black', lw=1.5))
    # Draw a rectangle to represent the bounded vertical line

    rect2 = patches.Rectangle((210, 19), 10, 0.01, linewidth=2, edgecolor='red', facecolor='none')
    figure.add_patch(rect2)
    rect2 = patches.Rectangle((210, 4), 10, 0.01, linewidth=2, edgecolor='red', facecolor='none')
    figure.add_patch(rect2)
    figure.axhline(y=20, color='red', linestyle='-.', linewidth=0.8)
    figure.text(310, 10,r"$\leq 20\%$", fontsize=TICK_FONT_SIZE, ha='center')
# Add the rectangle to the plot
   
   
    #figure.axvline(x=8.5, ymax=23,ymin=0,color='red', linestyle='--', linewidth=2)
    size = fig.get_size_inches()
    dpi = fig.get_dpi()

    plt.savefig(filename + ".pdf", bbox_inches='tight')


# example for reading csv file
def ReadFile():
    y = []
    col1 = []
    col2 = []
    col3 = []
    col4 = []
    col5 = []
    col6 = []
    col7 = []
    col8 = []

    for id in it.chain(range(28, 32)):
        file = '/data1/xtra/results/timestamps/PRJ_{}.txt'.format(id)
        f = open(file, "r")
        read = f.readlines()
        x = float(read.pop(len(read) - 1).strip("\n"))  # get last timestamp
        value = len(read) / x  # get throughput (#items/ms)
        col1.append(value)
    y.append(col1)

    for id in it.chain(range(28, 32)):
        file = '/data1/xtra/results/timestamps/NPJ_{}.txt'.format(id)
        f = open(file, "r")
        read = f.readlines()
        x = float(read.pop(len(read) - 1).strip("\n"))  # get last timestamp
        value = len(read) / x  # get throughput (#items/ms)
        col2.append(value)
    y.append(col2)

    for id in it.chain(range(28, 32)):
        file = '/data1/xtra/results/timestamps/MPASS_{}.txt'.format(id)
        f = open(file, "r")
        read = f.readlines()
        x = float(read.pop(len(read) - 1).strip("\n"))  # get last timestamp
        value = len(read) / x  # get throughput (#items/ms)
        col3.append(value)
    y.append(col3)

    for id in it.chain(range(28, 32)):
        file = '/data1/xtra/results/timestamps/MWAY_{}.txt'.format(id)
        f = open(file, "r")
        read = f.readlines()
        x = float(read.pop(len(read) - 1).strip("\n"))  # get last timestamp
        value = len(read) / x  # get throughput (#items/ms)
        col4.append(value)
    y.append(col4)

    for id in it.chain(range(28, 32)):
        file = '/data1/xtra/results/timestamps/SHJ_JM_NP_{}.txt'.format(id)
        f = open(file, "r")
        read = f.readlines()
        x = float(read.pop(len(read) - 1).strip("\n"))  # get last timestamp
        value = len(read) / x  # get throughput (#items/ms)
        col5.append(value)
    y.append(col5)

    for id in it.chain(range(28, 32)):
        file = '/data1/xtra/results/timestamps/SHJ_JBCR_NP_{}.txt'.format(id)
        f = open(file, "r")
        read = f.readlines()
        x = float(read.pop(len(read) - 1).strip("\n"))  # get last timestamp
        value = len(read) / x  # get throughput (#items/ms)
        col6.append(value)
    y.append(col6)

    for id in it.chain(range(28, 32)):
        file = '/data1/xtra/results/timestamps/PMJ_JM_NP_{}.txt'.format(id)
        f = open(file, "r")
        read = f.readlines()
        x = float(read.pop(len(read) - 1).strip("\n"))  # get last timestamp
        value = len(read) / x  # get throughput (#items/ms)
        col7.append(value)
    y.append(col7)

    for id in it.chain(range(28, 32)):
        file = '/data1/xtra/results/timestamps/PMJ_JBCR_NP_{}.txt'.format(id)
        f = open(file, "r")
        read = f.readlines()
        x = float(read.pop(len(read) - 1).strip("\n"))  # get last timestamp
        value = len(read) / x  # get throughput (#items/ms)
        col8.append(value)
    y.append(col8)
    return y


if __name__ == "__main__":
    # x_values = ['Unique', 'Zipf(0)', 'Zipf(0.2)', 'Zipf(0.4)', 'Zipf(0.8)', 'Zipf(1)']
    x_values = [1600, 3200, 6400, 12800, 25600]

    y_values = ReadFile()

    legend_labels = ['NPJ', 'PRJ', 'MWAY', 'MPASS', 'SHJ$^{JM}$', 'SHJ$^{JB}$', 'PMJ$^{JM}$',
                     'PMJ$^{JB}$']

    DrawFigure(x_values, y_values, legend_labels,
               'Input arrival rate of R (e/ms)', 'Tpt. (#matches/ms)', x_values[0],
               x_values[4], 'throughput_figure1_1', False)

#   DrawLegend(legend_labels, 'factor_legend')