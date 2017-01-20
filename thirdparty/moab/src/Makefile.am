# Don't require GNU-standard files (Changelog, README, etc.)
AUTOMAKE_OPTIONS = foreign subdir-objects

# Things to build
lib_LTLIBRARIES = libMOAB.la
AM_CPPFLAGS =

# Subdirectories to build
SUBDIRS =
libMOAB_la_LIBADD =
if PARALLEL
  SUBDIRS += parallel
  libMOAB_la_LIBADD += parallel/libMOABpar.la
  AM_CPPFLAGS += -I$(srcdir)/parallel
endif

include moab/EigenHeaders

SUBDIRS += io LocalDiscretization verdict RefineMesh .
libMOAB_la_LIBADD += io/libmoabio.la LocalDiscretization/libLocalDiscretization.la verdict/libmoabverdict.la RefineMesh/libRefineMesh.la

# Some variables
AM_CPPFLAGS += -DIS_BUILDING_MB \
               -DLOCDIR=src/ \
               -I. \
               -I$(srcdir)/io \
               -I$(srcdir)/io/mhdf/include \
               -I$(srcdir)/parallel \
               -I$(srcdir)/LocalDiscretization \
               -I$(srcdir)/verdict \
               -I$(srcdir)/RefineMesh

# The directory in which to install headers
libMOAB_la_includedir = $(includedir)

# The list of source files, and any header files that do not need to be installed
libMOAB_la_SOURCES = \
  AdaptiveKDTree.cpp \
  AEntityFactory.cpp \
  AEntityFactory.hpp \
  AffineXform.cpp \
  AffineXform.hpp \
  AxisBox.cpp \
  AxisBox.hpp \
  BitPage.cpp \
  BitPage.hpp \
  BitTag.cpp \
  BitTag.hpp \
  BoundBox.cpp \
  BSPTree.cpp \
  BSPTreePoly.cpp \
  BVHTree.cpp \
  CN.cpp \
  CartVect.cpp \
  Core.cpp \
  DebugOutput.cpp \
  DebugOutput.hpp \
  DenseTag.hpp \
  DenseTag.cpp \
  DualTool.cpp \
  ElementSequence.hpp \
  EntitySequence.cpp \
  EntitySequence.hpp \
  ErrorHandler.cpp \
  ErrorOutput.cpp \
  ErrorOutput.hpp \
  Factory.cpp \
  FBEngine.cpp \
  FileOptions.cpp \
  GeomUtil.cpp \
  GeomTopoTool.cpp \
  HalfFacetRep.cpp \
  HigherOrderFactory.cpp \
  HomXform.cpp \
  iMOAB.cpp \
  Internals.hpp \
  LloydSmoother.cpp \
  MBCNArrays.hpp \
  MergeMesh.cpp \
  MeshSet.cpp \
  MeshSet.hpp \
  MeshSetSequence.cpp \
  MeshSetSequence.hpp \
  MeshTag.hpp \
  MeshTag.cpp \
  MeshTopoUtil.cpp \
  OrientedBox.cpp \
  OrientedBoxTreeTool.cpp \
  lotte/poly.c \
  lotte/findpt.c \
  lotte/errmem.c \
  lotte/tensor.c \
  PolyElementSeq.cpp \
  PolyElementSeq.hpp \
  ProgOptions.cpp \
  Range.cpp \
  RangeSeqIntersectIter.cpp \
  RangeSeqIntersectIter.hpp \
  ReadUtil.cpp \
  ReadUtil.hpp \
  ReaderWriterSet.cpp \
  ReorderTool.cpp \
  ScdElementData.cpp \
  ScdElementData.hpp \
  ScdInterface.cpp \
  ScdVertexData.cpp \
  ScdVertexData.hpp \
  SequenceData.hpp \
  SequenceData.cpp \
  SequenceManager.cpp \
  SequenceManager.hpp \
  SetIterator.cpp \
  Skinner.cpp \
  SmoothCurve.cpp \
  SmoothCurve.hpp \
  SmoothFace.cpp \
  SmoothFace.hpp \
  SparseTag.cpp \
  SparseTag.hpp \
  SpatialLocator.cpp \
  SpectralMeshTool.cpp \
  StructuredElementSeq.cpp \
  StructuredElementSeq.hpp \
  SweptElementData.cpp \
  SweptElementData.hpp \
  SweptElementSeq.cpp \
  SweptElementSeq.hpp \
  SweptVertexData.cpp \
  SweptVertexData.hpp \
  SysUtil.cpp \
  SysUtil.hpp \
  TagCompare.hpp \
  TagInfo.cpp \
  TagInfo.hpp \
  Tree.cpp \
  TupleList.cpp \
  Types.cpp \
  TypeSequenceManager.cpp \
  TypeSequenceManager.hpp \
  UnstructuredElemSeq.cpp \
  UnstructuredElemSeq.hpp \
  Util.cpp \
  VarLenDenseTag.hpp \
  VarLenDenseTag.cpp \
  VarLenSparseTag.hpp \
  VarLenSparseTag.cpp \
  VarLenTag.hpp \
  VertexSequence.hpp \
  VertexSequence.cpp \
  WriteUtil.cpp \
  WriteUtil.hpp \
  moab_mpe.h

nobase_includedir = $(includedir)/moab
nodist_nobase_include_HEADERS = moab/MOABConfig.h

# The list of header files which are to be installed
nobase_libMOAB_la_include_HEADERS = \
  moab/AdaptiveKDTree.hpp \
  moab/BoundBox.hpp \
  moab/BSPTree.hpp \
  moab/BSPTreePoly.hpp \
  moab/BVHTree.hpp \
  moab/CN.hpp \
  moab/CartVect.hpp \
  moab/Compiler.hpp \
  moab/Core.hpp \
  moab/CpuTimer.hpp \
  moab/DualTool.hpp \
  moab/Error.hpp \
  moab/GeomTopoTool.hpp \
  moab/HalfFacetRep.hpp \
  moab/HigherOrderFactory.hpp \
  moab/HomXform.hpp \
  moab/EntityType.hpp \
  moab/EntityHandle.hpp \
  moab/ErrorHandler.hpp \
  moab/FBEngine.hpp \
  moab/FileOptions.hpp \
  moab/FindPtFuncs.h \
  moab/Forward.hpp \
  moab/GeomUtil.hpp \
  moab/Interface.hpp \
  moab/iMOAB.h \
  moab/imoab_protos.h \
  moab/LloydSmoother.hpp \
  moab/point_locater/tree/common_tree.hpp \
  moab/point_locater/tree/element_tree.hpp \
  moab/point_locater/tree/bvh_tree.hpp \
  moab/point_locater/io.hpp  \
  moab/point_locater/element_maps/linear_hex_map.hpp  \
  moab/point_locater/element_maps/linear_tet_map.hpp  \
  moab/point_locater/element_maps/spectral_hex_map.hpp  \
  moab/point_locater/element_maps/quadratic_hex_map.hpp  \
  moab/point_locater/point_locater.hpp \
  moab/point_locater/parametrizer.hpp \
  moab/Matrix3.hpp \
  moab/MergeMesh.hpp \
  moab/MeshTopoUtil.hpp \
  moab/OrientedBox.hpp \
  moab/OrientedBoxTreeTool.hpp \
  moab/PartitionerBase.hpp \
  moab/ProgOptions.hpp \
  moab/Range.hpp \
  moab/RangeMap.hpp \
  moab/ReadUtilIface.hpp \
  moab/ReaderIface.hpp \
  moab/ReaderWriterSet.hpp \
  moab/ReorderTool.hpp \
  moab/ScdInterface.hpp \
  moab/SetIterator.hpp \
  moab/Skinner.hpp \
  moab/SpatialLocator.hpp \
  moab/SpatialLocatorTimes.hpp \
  moab/SpectralMeshTool.hpp \
  moab/Tree.hpp \
  moab/TreeStats.hpp \
  moab/TupleList.hpp \
  moab/Types.hpp \
  moab/UnknownInterface.hpp \
  moab/Util.hpp \
  moab/WriteUtilIface.hpp \
  moab/WriterIface.hpp \
  $(EIGEN_INST_HDRS) \
  MBEntityType.h \
  MBCN.h \
  MBCN_protos.h \
  moab/MOABConfig.h \
  MBTagConventions.hpp \
  moab_export.h

if ENABLE_metis
  libMOAB_la_SOURCES += MetisPartitioner.cpp
  nobase_libMOAB_la_include_HEADERS += moab/MetisPartitioner.hpp
  AM_CPPFLAGS += $(METIS_INCLUDES)
  # not sure yet if this is needed; will see when linking io reader (MPAS so far)
  # the tool mbpart needed it, and it just includes MetisPartitioner.hpp;
  # so it needs ways to find the libraries zoltan depends on
  libMOAB_la_LIBADD += $(METIS_LIBS)
endif

if HAVE_ZOLTAN
  libMOAB_la_SOURCES += ZoltanPartitioner.cpp
  nobase_libMOAB_la_include_HEADERS += moab/ZoltanPartitioner.hpp
  AM_CPPFLAGS += $(ZOLTAN_INC_FLAGS)
  # not sure yet if this is needed; will see when linking io reader (MPAS so far)
  # the tool mbpart needed it, and it just includes ZoltanPartitioner.hpp;
  # so it needs ways to find the libraries zoltan depends on
  libMOAB_la_LIBADD += $(ZOLTAN_LIBS) $(ZOLTAN_LIB_FLAGS)
endif

if HAVE_CGM
  AM_CPPFLAGS += @CGM_CPPFLAGS@
endif

DISTCLEANFILES = moab_export.h
