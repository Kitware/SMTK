import os.path
import paraview

# retrieve the expanded path to paraview's __init__.py file.
paraview_init = os.path.realpath(paraview.__file__)

# the directory that we wish to use as paraview's PYTHONPATH is the directory
# above the one that contains paraview's __init__.py file.
paraview_pythonpath = os.path.abspath(
    os.path.dirname(os.path.dirname(paraview_init)))

print 'PARAVIEW_PYTHONPATH=%s' % paraview_pythonpath
