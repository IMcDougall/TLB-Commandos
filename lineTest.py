
from zplot import *
import sys

ctype = 'eps' if len(sys.argv) < 2 else sys.argv[1]
c = canvas(ctype, title='example4', dimensions=['3in', '2.4in'])

# load some data
t = table(file='data.20221210-174857')

# make a drawable region for a graph
d = drawable(canvas=c, xrange=[0,250], yrange=[0,100],
                          coord=['0.5in','0.4in'], dimensions=['2.3in','1.7in'])

# make some axes
axis(drawable=d, title='A Sample Graph', xtitle='The X-Axis',
          ytitle='The Y-Axis')

# plot the points
p = plotter()
p.line(drawable=d, table=t, xfield='numberOfPages', yfield='accessesPerSecond')

# finally, output the graph to a file
c.render()
