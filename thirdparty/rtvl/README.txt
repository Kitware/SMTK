This directory requires one to build vxl from its Subversion repository:

  http://vxl.sourceforge.net/

Download instructions:

  http://sourceforge.net/scm/?type=svn&group_id=24293

  svn co https://vxl.svn.sourceforge.net/svnroot/vxl/trunk vxl

One uses CMake to build and install the vxl package.  The following
CMake cache options should be set ON when configuring vxl:

  BUILD_CONTRIB
  BUILD_CORE_GEOMETRY
  BUILD_CORE_IMAGING
  BUILD_CORE_NUMERICS
  BUILD_CORE_SERIALISATION
  BUILD_CORE_UTILITIES
  BUILD_MUL
  BUILD_OXL
  BUILD_RPL
  BUILD_RPL_RGTL

Other options may be set OFF:

  BUILD_BRL
  BUILD_CONTRIB_VIDL2
  BUILD_CONVERSIONS
  BUILD_EXAMPLES
  BUILD_FOR_VXL_DASHBOARD
  BUILD_GEL
  BUILD_MUL_TOOLS
  BUILD_OUL
  BUILD_PRIP
  BUILD_SHARED_LIBS
  BUILD_TARGETJR
  BUILD_TBL
  BUILD_TESTING
  BUILD_UNMAINTAINED_LIBRARIES
  BUILD_VGUI

After building vxl one may optionally install it, but one may also use
vxl directly from its build tree.  When configuring this project,
set VXL_DIR either to point at the vxl build tree or at the

  <prefix>/share/vxl/cmake

directory of the vxl install tree.

------------------------------------------------------------------------------
This directory contains Brad King's research code for analysis of
LiDaR points and extraction of a terrain surface hypothesis.
Currently the interface is very primitive.  There are two executables
which must be run in succession:

  cmbTokenRefine    input.vtp outname
  cmbTerrainExtract input.vtp name max min

The "cmbTokenRefine" tool reads input points from a Visualization
Toolkit Polygonal data file (.vtp) and generates a set of files whose
name start with <outname>.  These files contain a serialized
representation of the Tensor Voting tokens suitable for use at each
voting scale selected by the tool.  They are numbered by "scale index"
and serve as input to the second tool.

The "cmbTerrainExtract" tool loads the representation serialized by
the first tool for a range of scale indices given by the <max> and
<min> arguments.  The given <name> must match the <outname> given to
the first tool.  This tool generates a terrain surface hypothesis in a
coarse-to-fine manner.  It generates a series of .vtp files with
increasingly detailed terrain.

Example Usage:

$ mkdir TestTrackGroupA && cd TestTrackGroupA
$ /pathto/cmbTokenRefine /pathto/TestTrackGroupA.vtp TestTrackGroupA
$ /pathto/cmbTerrainExtract /pathto/TestTrackGroupA.vtp TestTrackGroupA 17 8

The code has not been tested with input data sizes more than 1e6 points.
