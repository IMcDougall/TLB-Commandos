
from zplot import *
import sys

ctype = 'eps' if len(sys.argv) < 2 else sys.argv[1]
tableName = sys.argv[2]
fileName = sys.argv[3]
c = canvas(ctype, title=fileName, dimensions=['3in', '2.4in'])

# load some data
t = table(file=tableName)

# make a drawable region for a graph
d = drawable(canvas=c, xrange=[0,t.getmax('numberOfPages')], yrange=[0,t.getmax('accessesPerSecond')+20],
                          coord=['0.5in','0.4in'], dimensions=['2.3in','1.7in'])

# make some axes
axis(drawable=d, title=fileName, xtitle='Pages Accessed',
          ytitle='Mega-Accesses/Second')

# plot the points
p = plotter()
p.line(drawable=d, table=t, xfield='numberOfPages', yfield='accessesPerSecond')

# finally, output the graph to a file
c.render()
