#
# This example creates a polygonal model of a cone, and then rendered it to
# the screen. It willrotate the cone 360 degrees and then exit. The basic
# setup of source -> mapper -> actor -> renderer -> renderwindow is 
# typical of most VTK programs.
#

#
# Do not forget to add the path to the Wrapping/Tcl directory to your 
# TCLLIBPATH environment variable. Use forward slash / instead of \ 
# and quote (") path containing spaces. 
# Also check that the path to your DLL (i.e. your build dir) is also
# in your PATH environment variable.
# 

#
# First we include the Tcl packages which will make available 
# all of the vtk commands to Tcl
#
package require vtkMOABReaderTCL

vtkMOABReader vtkmr

vtkmr ListInstances

vtkmr ListMethods

vtkmr Print

