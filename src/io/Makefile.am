# Subdirectories to build
if HAVE_HDF5
  SUBDIRS = mhdf .
else
  SUBDIRS = .
endif

# Things to build
noinst_LTLIBRARIES = libmoabio.la 
libmoabio_la_LIBADD = $(NETCDF_LIBS) $(PNETCDF_LIBS) $(CGNS_LIBS) $(HDF5_LIBS) 

AM_CPPFLAGS  = -DIS_BUILDING_MB \
               -DLOCDIR=src/io/ \
               -I..  -I$(srcdir)/.. -I$(srcdir)/../moab -I$(srcdir)/../parallel

if HAVE_NETCDF
  MOAB_NETCDF_SRCS = ReadNCDF.cpp  ReadNCDF.hpp \
                     WriteNCDF.cpp WriteNCDF.hpp \
                     WriteSLAC.cpp WriteSLAC.hpp \
                     ReadNC.cpp ReadNC.hpp \
                     WriteNC.cpp WriteNC.hpp \
                     NCHelper.cpp NCHelper.hpp \
                     NCHelperEuler.cpp NCHelperEuler.hpp \
                     NCHelperFV.cpp NCHelperFV.hpp \
                     NCHelperHOMME.cpp NCHelperHOMME.hpp \
                     NCHelperMPAS.cpp NCHelperMPAS.hpp \
                     NCHelperGCRM.cpp NCHelperGCRM.hpp \
                     NCWriteHelper.cpp NCWriteHelper.hpp \
                     NCWriteEuler.cpp NCWriteEuler.hpp \
                     NCWriteFV.cpp NCWriteFV.hpp \
                     NCWriteHOMME.cpp NCWriteHOMME.hpp \
                     NCWriteMPAS.cpp NCWriteMPAS.hpp \
                     NCWriteGCRM.cpp NCWriteGCRM.hpp 
else
  MOAB_NETCDF_SRCS =
endif

if HAVE_PNETCDF
if !HAVE_NETCDF
     MOAB_NETCDF_SRCS += ReadNC.cpp ReadNC.hpp \
                     WriteNC.cpp WriteNC.hpp \
                     NCHelper.cpp NCHelper.hpp \
                     NCHelperEuler.cpp NCHelperEuler.hpp \
                     NCHelperFV.cpp NCHelperFV.hpp \
                     NCHelperHOMME.cpp NCHelperHOMME.hpp \
                     NCHelperMPAS.cpp NCHelperMPAS.hpp \
                     NCHelperGCRM.cpp NCHelperGCRM.hpp \
                     NCWriteHelper.cpp NCWriteHelper.hpp \
                     NCWriteEuler.cpp NCWriteEuler.hpp \
                     NCWriteFV.cpp NCWriteFV.hpp \
                     NCWriteHOMME.cpp NCWriteHOMME.hpp \
                     NCWriteMPAS.cpp NCWriteMPAS.hpp \
                     NCWriteGCRM.cpp NCWriteGCRM.hpp 
endif
endif

if ENABLE_mbpart
  AM_CPPFLAGS += $(ZOLTAN_INC_FLAGS) 
endif

if HAVE_CGNS
  MOAB_CGNS_SRCS = ReadCGNS.cpp  ReadCGNS.hpp \
                   WriteCGNS.cpp  WriteCGNS.hpp
else
  MOAB_CGNS_SRCS =
endif

if HAVE_HDF5
  libmoabio_la_LIBADD += mhdf/libmhdf.la
  MOAB_HDF5_SRCS = HDF5Common.cpp \
                   HDF5Common.hpp \
                   ReadHDF5.cpp \
                   ReadHDF5.hpp \
                   ReadHDF5Dataset.hpp \
                   ReadHDF5Dataset.cpp \
                   ReadHDF5VarLen.hpp \
                   ReadHDF5VarLen.cpp \
                   WriteHDF5.cpp \
                   WriteHDF5.hpp
  AM_CPPFLAGS += -I$(srcdir)/mhdf/include
else
  MOAB_HDF5_SRCS = 
endif

if HAVE_CCMIO
  MOAB_CCMIO_SRCS = WriteCCMIO.cpp WriteCCMIO.hpp ReadCCMIO.cpp ReadCCMIO.hpp
  libmoabio_la_LIBADD += $(CCMIO_LIBS)
else
  MOAB_CCMIO_SRCS = 
endif

if HAVE_DAMSEL
  MOAB_DAMSEL_SRCS = WriteDamsel.cpp WriteDamsel.hpp ReadDamsel.cpp ReadDamsel.hpp DamselUtil.cpp DamselUtil.hpp
  libmoabio_la_LIBADD += $(DAMSEL_LIBS)
else
  MOAB_DAMSEL_SRCS =
endif

if HAVE_CGM
  AM_CPPFLAGS += @CGM_CPPFLAGS@
if USING_DEV_CGM
  AM_CPPFLAGS += -I$(CGM_DIR)/src
endif
  MOAB_CGM_SRCS =  ReadCGM.cpp  ReadCGM.hpp
  libmoabio_la_LIBADD += @CGM_LIBS@
else
  MOAB_CGM_SRCS = 
endif

# The list of source files, and any header files that do not need to be installed
libmoabio_la_SOURCES = \
  exodus_order.h \
  patran_order.h \
  IODebugTrack.cpp \
  IODebugTrack.hpp \
  ExoIIUtil.cpp \
  ExoIIUtil.hpp \
  FileTokenizer.cpp \
  FileTokenizer.hpp \
  GmshUtil.cpp \
  GmshUtil.hpp \
  ReadABAQUS.cpp \
  ReadABAQUS.hpp \
  ReadGmsh.cpp \
  ReadGmsh.hpp \
  ReadIDEAS.cpp \
  ReadIDEAS.hpp \
  ReadMCNP5.cpp \
  ReadMCNP5.hpp \
  ReadNASTRAN.cpp \
  ReadNASTRAN.hpp \
  ReadRTT.cpp \
  ReadRTT.hpp \
  ReadOBJ.cpp \
  ReadOBJ.hpp \
  ReadSmf.hpp \
  ReadSmf.cpp \
  ReadSms.hpp \
  ReadSms.cpp \
  ReadSTL.cpp \
  ReadSTL.hpp \
  ReadTemplate.cpp \
  ReadTemplate.hpp \
  ReadTetGen.cpp \
  ReadTetGen.hpp \
  ReadVtk.cpp \
  ReadVtk.hpp \
  SMF_State.cpp \
  SMF_State.hpp \
  Tqdcfr.cpp \
  Tqdcfr.hpp \
  VtkUtil.cpp \
  WriteAns.cpp \
  WriteAns.hpp \
  WriteGMV.cpp \
  WriteGMV.hpp \
  WriteGmsh.cpp \
  WriteGmsh.hpp \
  WriteSTL.cpp \
  WriteSTL.hpp \
  WriteSmf.cpp \
  WriteSmf.hpp \
  WriteTemplate.cpp \
  WriteTemplate.hpp \
  WriteVtk.cpp \
  WriteVtk.hpp \
  $(MOAB_NETCDF_SRCS) \
  $(MOAB_CGNS_SRCS) \
  $(MOAB_HDF5_SRCS) \
  $(MOAB_CCMIO_SRCS) \
  $(MOAB_DAMSEL_SRCS) \
  $(MOAB_CGM_SRCS)

libmoabio_la_includedir = $(includedir)
nobase_libmoabio_la_include_HEADERS = moab/ExoIIInterface.hpp moab/VtkUtil.hpp
