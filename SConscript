import os
from building import *

cwd = GetCurrentDir()

# init src and inc
src = []
inc = []

# add lvgl_rtt common include
inc += [cwd]
inc += [cwd + '/inc']

# define LVGL_RTT group
objs = DefineGroup('lvgl_rtt', src, depend = ['PKG_USING_LVGL'], CPPPATH = inc)

list = os.listdir(cwd)
if GetDepend('PKG_USING_LVGL'):
    for d in list:
        path = os.path.join(cwd, d)
        if os.path.isfile(os.path.join(path, 'SConscript')):
            objs = objs + SConscript(os.path.join(d, 'SConscript'))

Return('objs')