dnl -----------------------------------------------------------------------------------------------
dnl  METIS.M4
dnl
dnl    Provides the framework to ensure that Metis is setup
dnl    correctly and if necessary provides macros to install
dnl    Metis.
dnl -----------------------------------------------------------------------------------------------

dnl-----------------------------------------------------------------------------------
dnl CONFIGURE METIS
dnl   Untar the package, then run the necessary installation steps
dnl-----------------------------------------------------------------------------------
AC_DEFUN([FATHOM_CONFIGURE_METIS],[

  AC_ARG_WITH([metis],
               [AS_HELP_STRING([--with-metis=DIR],[Directory containing Metis library installation])],
               [if (test "x$withval" != "x" && test "x$withval" != "xno"); then 
                  enablemetis=yes
                  DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --with-metis=\"${withval}\""
                  METIS_DIR="$withval"
                fi],
               [enablemetis=no; METIS_DIR=""]
              )

  # Supported Metis versions: 4.0.3, 5.1.0, 5.1.0p2
  # Arguments: 1) Default Version Number, 2) Download by default ?
  AUSCM_CONFIGURE_DOWNLOAD_METIS([5.1.0p2],[no])

  if (test "x$enablemetis" == "xyes" && test "x$METIS_DIR" != "x"); then
    dnl Honor METIS_DIR if it is set
    if (test ! -d $METIS_DIR); then
       AC_MSG_ERROR([Invalid METIS_DIR ($METIS_DIR) specified.])
    fi

    # Check to see if we want to override the include directory
    # (do not honor if we are installing our own version of metis)
    if (test "$withmetisinc" != "no" && test "$downloadmetis" != "yes"); then
      METIS_INCLUDES="$withmetisinc"
    fi

    # Check to see if we want to override the library directory
    # (do not honor if we are installing our own version of metis)
    if (test "$withmetislib" != "no" && test "$downloadmetis" != "yes"); then
      METIS_LIBS="$withmetislib"
    fi
    
    if (test "x$METIS_INCLUDES" == "x" && test "x$METIS_LIBS" == "x"); then
      PREFIX_PRINT([Searching for Metis version style in $METIS_DIR.])
      METIS_INCLUDES="-I$METIS_DIR/include"
      METIS_LIBS="-lmetis"
      # Check if we need to download & build METIS
      if (test "$downloadmetis" != "no"); then
        if (test "$metis_manual_install" != "no"); then
          PREFIX_PRINT([Found Metis version 4.0.3.])
    	  else
          PREFIX_PRINT([Found Metis version METIS_DOWNLOAD_VERSION.])
      	fi
	    # If we already have Metis make sure it's arranged correctly  
      elif ((test -e $METIS_DIR/libmetis.a || test -e $METIS_DIR/libmetis.so || test -e $METIS_DIR/libmetis.dylib) && test -e $METIS_DIR/Lib/metis.h); then
        PREFIX_PRINT([Found Metis version 4.x style METIS.])
      else # For version 5.0 and above
        PREFIX_PRINT([Found Metis version 5.x or above.])
      fi
    fi
    
    #---------------------------------
    # Check Metis Header and Library
    #---------------------------------
    AC_LANG_PUSH(C)
    oldCPPFLAGS=$CPPFLAGS
    CPPFLAGS="$METIS_INCLUDES $ZOLTAN_INC_FLAGS $CPPFLAGS"
    AC_CHECK_HEADER([metis.h], 
      [PREFIX_PRINT(Successfully found metis.h!)], 
      [enablemetis=no; AC_MSG_WARN([Metis header files not found!])]
    )
    CPPFLAGS=$oldCPPFLAGS
    
    oldLDFLAGS=$LDFLAGS
    LDFLAGS="-L$METIS_DIR/lib $ZOLTAN_LIB_FLAGS $LDFLAGS"
    AC_CHECK_LIB([metis], [METIS_MeshToDual],
      [enablemetis=$enablemetis], 
      [enablemetis=no; AC_MSG_WARN([Could not find Metis library!])],
      [-lm $LIBS]
    )
    LDFLAGS=$oldLDFLAGS
    AC_LANG_POP(C)

    # If the checks pass we can call it a success
    if (test "x$enablemetis" = "xyes"); then
      AC_DEFINE(HAVE_METIS, 1, [Flag indicating whether the library will be compiled with Metis support])
      PREFIX_PRINT([Configuring library with Metis support])
      ZOLTAN_LIB_FLAGS="-L$METIS_DIR/lib $ZOLTAN_LIB_FLAGS"
      ZOLTAN_INC_FLAGS="$METIS_INCLUDES $ZOLTAN_INC_FLAGS"
      ZOLTAN_LIBS="$METIS_LIBRARY $ZOLTAN_LIBS"
    else
      AC_MSG_ERROR([Could not find a valid copy of Metis in $METIS_DIR.  See config.log for details.])
    fi
  fi
  
  # Return some variables
  AC_SUBST(METIS_DIR)
  AC_SUBST(METIS_INCLUDES)
  AC_SUBST(METIS_LIBS)
  AC_SUBST(enablemetis)
  AM_CONDITIONAL(ENABLE_metis, test "x$enablemetis" == "xyes")
])


dnl-----------------------------------------------------------------------------------
dnl CONFIGURE PARMETIS
dnl   Untar the package, then run the necessary installation steps
dnl-----------------------------------------------------------------------------------
AC_DEFUN([FATHOM_CONFIGURE_PARMETIS],[
 
  AC_ARG_WITH([parmetis], 
               [AS_HELP_STRING([--with-parmetis=DIR],[Directory containing ParMetis library installation])],
               [if (test "x$withval" != "x" && test "x$withval" != "xno"); then
                 enableparmetis=yes
                 DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --with-parmetis=\"${withval}\""
                 PARMETIS_DIR="$withval"
                fi],
               [enableparmetis=no; PARMETIS_DIR=""]
              )

  # Supported ParMetis versions: 4.0.3, 3.2.0
  # Arguments: 1) Default Version Number, 2) Download by default ?
  AUSCM_CONFIGURE_DOWNLOAD_PARMETIS([4.0.3],[no])

  if (test "x$enableparmetis" == "xyes" && test "x$PARMETIS_DIR" != "x"); then
    dnl Honor PARMETIS_DIR if it is set
    if (test ! -d $PARMETIS_DIR); then
       AC_MSG_ERROR([Invalid PARMETIS_DIR ($PARMETIS_DIR) specified.])
    fi

    # Check to see if we want to override the include directory
    # (do not honor if we are installing our own version of parmetis)
    if (test "$withparmetisinc" != "no" && test "$downloadparmetis" != "yes"); then
      PARMETIS_INCLUDES="$withparmetisinc"
    fi

    # Check to see if we want to override the library directory
    # (do not honor if we are installing our own version of parmetis)
    if (test "$withparmetislib" != "no" && test "$downloadparmetis" != "yes"); then
      PARMETIS_LIBS="$withparmetislib"
    fi
    
    if (test "x$PARMETIS_INCLUDES" == "x" && test "x$PARMETIS_LIBS" == "x"); then
      PREFIX_PRINT([Searching for ParMetis version style in $PARMETIS_DIR.])
      PARMETIS_INCLUDES="-I$PARMETIS_DIR/include"
      PARMETIS_LIBS="-lparmetis"
      # Check if we need to download & build PARMETIS
      if (test "$downloadparmetis" != "no"); then
        if (test "$parmetis_manual_install" != "no"); then
          PREFIX_PRINT([Found ParMetis version 3.2.0.])
    	  else
          PREFIX_PRINT([Found ParMetis version PARMETIS_DOWNLOAD_VERSION.])
      	fi
	    # If we already have ParMetis make sure it's arranged correctly  
      elif ((test -e $PARMETIS_DIR/libparmetis.a || test -e $PARMETIS_DIR/libparmetis.so || test -e $PARMETIS_DIR/libparmetis.dylib) && test -e $PARMETIS_DIR/Lib/parmetis.h); then
        PREFIX_PRINT([Found ParMetis version 3.x style PARMETIS.])
      else # For version 5.0 and above
        PREFIX_PRINT([Found ParMetis version 4.x or above.])
      fi
    fi
    
    #---------------------------------
    # Check Metis Header and Library
    #---------------------------------
    AC_LANG_PUSH(C)
    oldCPPFLAGS=$CPPFLAGS
    CPPFLAGS="$PARMETIS_INCLUDES $ZOLTAN_INC_FLAGS $CPPFLAGS"
    AC_CHECK_HEADER([parmetis.h], 
      [PREFIX_PRINT(Successfully found parmetis.h!)], 
      [enablemetis=no; AC_MSG_WARN([ParMetis header files not found!])]
    )
    CPPFLAGS=$oldCPPFLAGS
    
    oldLDFLAGS=$LDFLAGS
    LDFLAGS="-L$PARMETIS_DIR/lib $ZOLTAN_LIB_FLAGS $LDFLAGS"
    AC_CHECK_LIB([parmetis], [ParMETIS_V3_Mesh2Dual], 
      [enableparmetis=$enableparmetis; PARMETIS_LIBS="-lparmetis"], 
      [enableparmetis=no; AC_MSG_WARN([Could not find ParMetis library!])],
      [-lmetis -lm $LIBS]
    )
    LDFLAGS=$oldLDFLAGS
    AC_LANG_POP(C)

    # If the checks pass we can call it a success
    if (test "$enableparmetis" != "no"); then
      AC_DEFINE(HAVE_PARMETIS, 1, [Flag indicating whether the library will be compiled with ParMetis support])
      PREFIX_PRINT([Configuring library with ParMetis support])
      ZOLTAN_LIB_FLAGS="-L$PARMETIS_DIR/lib $ZOLTAN_LIB_FLAGS"
      ZOLTAN_INC_FLAGS="$PARMETIS_INCLUDES $ZOLTAN_INC_FLAGS"
      ZOLTAN_LIBS="$PARMETIS_LIBRARY $ZOLTAN_LIBS"
    else
      AC_MSG_ERROR([Could not find a valid copy of ParMetis in $PARMETIS_DIR.  See config.log for details.])
    fi
  fi
  
  # Return some variables
  AC_SUBST(PARMETIS_DIR)
  AC_SUBST(PARMETIS_INCLUDES)
  AC_SUBST(PARMETIS_LIBS)
  AC_SUBST(enableparmetis)
  AM_CONDITIONAL(ENABLE_parmetis, test "x$enableparmetis" == "xyes")
])


