dnl -------------------------------------------------------------
dnl  external_packages_util.m4
dnl
dnl  This file contains general instructions for downloading,
dnl  unpacking, configuring, building, and installing 
dnl  dependent packages.  It also contains definitions of 
dnl  configure line download options.
dnl -------------------------------------------------------------

dnl Need the following
dnl  MOAB_ARCH

################
# DO NOT TOUCH #
################

m4_define([_m4_divert(HELP_BEGIN)],     100)
m4_define([_m4_divert(HELP_CANON)],     101)
m4_define([_m4_divert(HELP_ENABLE)],    102)
m4_define([_m4_divert(HELP_WITH)],      103)
m4_define([_m4_divert(HELP_DOWNLOAD)],  104)
m4_define([_m4_divert(HELP_VAR)],       105)
m4_define([_m4_divert(HELP_VAR_END)],   106)
m4_define([_m4_divert(HELP_END)],       107)

# --------------------------------------------------------------------
# AC_ARG_DOWNLOAD(PACKAGE, HELP-STRING, ACTION-IF-TRUE, [ACTION-IF-FALSE])
# --------------------------------------------------------------------
AC_DEFUN([AC_ARG_DOWNLOAD],
[AC_PROVIDE_IFELSE([AC_PRESERVE_HELP_ORDER],
[],
[m4_divert_once([HELP_DOWNLOAD], [[
Optional Downloads:
  --download-PACKAGE[=ARG]  download/configure PACKAGE with default options [ARG=yes,no,url]]])])dnl
m4_divert_once([HELP_DOWNLOAD], [$2])dnl
_AC_ENABLE_IF([download], [$1], [$3], [$4])dnl
])# AC_ARG_DOWNLOAD

AU_DEFUN([AC_DOWNLOAD],
[AC_ARG_DOWNLOAD([$1], [  --download-$1], [$2], [$3])])

# _AC_INIT_PARSE_ENABLE(OPTION-NAME)                                                                                                                                                                                             
# ----------------------------------
# A trivial front-end for _AC_INIT_PARSE_ENABLE2.
#
m4_define([_AC_INIT_PARSE_ENABLE_ORIG],
[m4_bmatch([$1], [^with],
     [_AC_INIT_PARSE_ENABLE2([$1], [with])],
     [m4_bmatch([$1], [^able],
        [_AC_INIT_PARSE_ENABLE2([$1], [enable])],
        [_AC_INIT_PARSE_ENABLE2([download], [download])])] )])

m4_define([_AC_INIT_PARSE_ENABLE],
[m4_bmatch([$1], [^with],
     [_AC_INIT_PARSE_ENABLE2([$1], [with])
      m4_ifdef([_AC_DEFINED_DOWNLOAD_MACRO], [], 
        [ _AC_INIT_PARSE_ENABLE2([download], [download]) 
          m4_define([_AC_DEFINED_DOWNLOAD_MACRO], [yes] )] )],
     [_AC_INIT_PARSE_ENABLE2([$1], [enable]) ]) ])


# _AC_INIT_PARSE_ENABLE2(OPTION-NAME, POSITIVE-NAME)
# --------------------------------------------------
# Handle an `--enable', a `--with' or a `--download` option.
#
# OPTION-NAME is `enable', `disable', `with', `without' or `download'.
# POSITIVE-NAME is the corresponding positive variant, i.e. `enable' or `with'.
#
# Positive variant of the option is recognized by the condition
# OPTION-NAME == POSITIVE-NAME .
#
m4_define([_AC_INIT_PARSE_ENABLE2],
[-$1-* | --$1-*)
    ac_useropt=`expr "x$ac_option" : 'x-*$1-\(m4_if([$1], [$2], [[[^=]]], [.])*\)'`
    # Reject names that are not valid shell variable names.
    expr "x$ac_useropt" : "[.*[^-+._$as_cr_alnum]]" >/dev/null &&
      AC_MSG_ERROR(
  [invalid ]m4_if([$2], [[with | download]], [package], [feature])[ name: $ac_useropt])                                                                                                                                                       
    ac_useropt_orig=$ac_useropt
    ac_useropt=`AS_ECHO(["$ac_useropt"]) | sed 's/[[-+.]]/_/g'`
    case $ac_user_opts in
      *"
"$2_$ac_useropt"
"*) ;;
      *) ac_unrecognized_opts="$ac_unrecognized_opts$ac_unrecognized_sep--$1-$ac_useropt_orig"
   ac_unrecognized_sep=', ';;
    esac
    eval $2_$ac_useropt=m4_if([$1], [$2], [\$ac_optarg], [no]) ;;dnl
])

####################
# RESTRICTION ENDS #
####################

AC_DEFUN([INITIALIZE_EXTERNAL_PACKAGES],
[
  # Check for command line utility/archive/download programs
  # that are essential for configure to work correctly.
  AC_CHECK_PROG(HAVE_READLINK, readlink, yes, no)
  AC_CHECK_PROG(HAVE_DIRNAME, dirname, yes, no)
  AC_CHECK_PROG(HAVE_BASENAME, basename, yes, no)
  AC_CHECK_PROG(HAVE_RSYNC, rsync, yes, no)
  # network download programs
  AC_CHECK_PROG(HAVE_WGET, wget, yes, no)
  AC_CHECK_PROG(HAVE_SCP, scp, yes, no)
  AC_CHECK_PROG(HAVE_CURL, curl, yes, no)
  # archive file inflation/deflation programs
  AC_CHECK_PROG(HAVE_TAR, tar, yes, no)
  AC_CHECK_PROG(HAVE_UNZIP, unzip, yes, no)
  AC_CHECK_PROG(HAVE_BZIP2, bzip2, yes, no)
  # file/directory hash computation programs
  AC_CHECK_PROG(HAVE_MD5SUM, md5sum, yes, no)
  if (test "$HAVE_MD5SUM" != "no"); then
    AC_PATH_PROG(MD5SUM_X, md5sum, "")
    HASHPRGM="$MD5SUM_X"
  else
    AC_CHECK_PROG(HAVE_SHASUM, shasum, yes, no)
    if (test "$HAVE_SHASUM" != "no"); then
      AC_PATH_PROG(SHASUM_X, shasum, "")
      HASHPRGM="$SHASUM_X -a1"
    else
      AC_ERROR([No file/directory hash computation program is available. Report to moab-dev@mcs.anl.gov])
    fi
  fi
  # other essential programs
  AC_PROG_LN_S
  AC_PATH_PROG(MKDIR_P, mkdir, "")
  if (test "x$MKDIR_P" != "x"); then
    MKDIR_P="$MKDIR_P -p"
  else
    AC_ERROR([Make directory command not found ? Seriously ? Report to moab-dev@mcs.anl.gov])
  fi
  AC_PROG_MKDIR_P
  AC_PROG_MAKE_SET

  # Some aliases for colors to pretty-print
  NORMAL=$(tput sgr0)
  GREEN=$(tput setaf 2)
  RED=$(tput setaf 1)

  # AC_PREFIX_DEFAULT($PACKAGE_NAME)
  MOAB_ARCH="$host"
  MOAB_SANDBOX="$PWD/sandbox"
  if (test "x$prefix" != "x$ac_default_prefix" && test "x$prefix" != "xNONE"); then
    # install dependencies in the same custom folder
    # specified by user
    MOAB_ARCH_DIR="$prefix"
  else
    # install it in a local sandbox folder
    MOAB_ARCH_DIR="$MOAB_SANDBOX/$MOAB_ARCH"
  fi
  MOAB_PACKAGES_DIR="$MOAB_SANDBOX/archives"
])

# AC_PROG_MKDIR_P
# is a backport of autoconf-2.60's AC_PROG_MKDIR_P.
# Remove this macro when we can assume autoconf >= 2.60.
m4_ifdef([AC_PROG_MKDIR_P], [], [
  AC_DEFUN([AC_PROG_MKDIR_P],
    [AC_REQUIRE([AM_PROG_MKDIR_P])dnl defined by automake
     MKDIR_P='$(mkdir_p)'
     AC_SUBST([MKDIR_P])])])

dnl -------------------------------------------------------------
dnl Fetches an external package into MOAB_ARCH using:
dnl $1 = Package Name
dnl $2 = URL
dnl $3 = Storage location (Archive name)
dnl -------------------------------------------------------------
AC_DEFUN([DOWNLOAD_EXTERNAL_PACKAGE],
[
  PREFIX_PRINT(Downloading sources from URL: $2 )
  cdir=`dirname $3`
  if (test "x$cdir" != "x."); then
    op_dirname="$cdir"
  else
    op_dirname="$MOAB_PACKAGES_DIR"
  fi
  hashtarfile1="0"
  if (test -f "$3"); then
    hashtarfile1="`$HASHPRGM $3 | cut -d ' ' -f1`"
  fi
  filedownloaded=no
  remoteprotocol=yes
  
  # decipher protocol needed to download
  case $2 in
    @*) remoteprotocol=no ;;
    *)  remoteprotocol=yes ;;
  esac
  currdir="$PWD"
  if (test $remoteprotocol != no); then
    if (test "$HAVE_WGET" != "no" ); then
      PREFIX_PRINT([   WGET: $1 package downloading to $3 ])
      if (test -f "$3"); then
        # Check if the file requested exists in the remote directory -- inform user if there is a network error 
        op_checkifexists="`wget --spider -O/dev/null -q $2 && echo yes || echo no`"
        if (test "$op_checkifexists" != "yes"); then
          AC_ERROR([ --  Requested URL does not exist in remote host. Try again later. ($2)  -- ])
        fi
        #op_needdownload="`wget --spider -N -q $2 && echo yes || echo no; cd $currdir`"
        op_downloadlog$1="`wget -q -c -N --progress=bar $2 -O $3`"
      else
        # No need to check for time-stamping
        op_downloadlog$1="`wget -q --progress=bar $2 -O $3`"
      fi
      filedownloaded=yes
    fi

    if (test "$HAVE_CURL" != "no" && test "$filedownloaded" != "yes"); then
      PREFIX_PRINT([   CURL: $1 package downloading to $3 ])
      op_downloadlog$1="`curl -R -s $2 -z $3 -o $3`"
      filedownloaded=yes
    fi
  else
    if (test "$HAVE_SCP" != "no" && test "$filedownloaded" != "yes"); then
      bnamerem="`echo $2 | cut -c 2-`"
      # op_downloadlog$1="`scp -q $bnamerem $3`"
      PREFIX_PRINT([   SCP: $1 package downloading to $3 ])
      op_downloadlog$1="`scp -q $2 $3`"
      filedownloaded=yes
    fi
  fi
  
  if (test "$filedownloaded" != "yes"); then
    AC_ERROR([ --  The archive URL ($2) specified cannot be handled by wget, curl or scp  -- ])
  else
    MSG_ECHO_LOG(${op_downloadlog$1})
  fi

  hashtarfile2="`$HASHPRGM $3 | cut -d ' ' -f1`"
  if (test "$hashtarfile1" != "$hashtarfile2"); then
    new_download=true
  else
    new_download=false
  fi

])

dnl -------------------------------------------------------------
dnl Unpacks an external package using:
dnl $1 = Package Name
dnl $2 = Storage location (Archive name)
dnl $3 = Source location (to untar)
dnl -------------------------------------------------------------
AC_DEFUN([DEFLATE_EXTERNAL_PACKAGE],
[
  PREFIX_PRINT([Deflating archive ($2 => $3) ])

  currdir="$PWD"
  need_configuration=false
  op_pkg_subdir=""
  if (test "x`basename $2|grep -E '\.tar'`" != "x" && test "$HAVE_TAR" != "no" ); then
    ##op_pkg_subdir="`tar -tf $2 | head -n 1 | $SED -e 's/\/$//'`"
    op_pkg_subdir="`tar -tf $2 | $SED -e 's@/.*@@' | uniq`"
    PREFIX_PRINT([   Untar file: $2 ])
    PREFIX_PRINT([   Source dir: $pkg_srcdir ])
    if (test ! -d "$3/$op_pkg_subdir"); then
      op_deflatelog$1="`cd $3 && tar -xf $2 && cd $currdir`"
      need_configuration=true
    fi
    if [ $new_download ]; then
      need_configuration=true
    fi
  elif (test "x`basename $2|grep -E '\.zip'`" != "x" && test "$HAVE_UNZIP" != "no" ); then
    PREFIX_PRINT([   Unzip file: $2, and $1-SRCDIR=$pkg_srcdir <<<])
    if ($new_download -eq true || test ! -d "$pkg_srcdir"); then
      op_deflatelog$1="`cd $3 && unzip -q $2 -d $3 && cd $currdir`"
      need_configuration=true
    fi
  elif (test "x`basename $2|grep -E '\.bz'`" != "x" && test "$HAVE_BZIP2" != "no" ); then
    PREFIX_PRINT([   Bunzip file: $2, and $1-SRCDIR=$pkg_srcdir <<<])
    if ( $new_download -eq true || test ! -d "$pkg_srcdir"); then
      op_deflatelog$1="`cd $3 && bzip2 -d -q $2 && cd $currdir`"
      need_configuration=true
    fi
  else
    AC_ERROR([ --  Unhandled file format for deflating package $1 -- Filename = $2 -- ])
  fi

  if (test "x$op_pkg_subdir" != "x"); then
    pkg_srcdir="$3/$op_pkg_subdir"
    MSG_ECHO_LOG(${op_deflatelog$1})
  else
    AC_ERROR([ --  Unhandled file format for getting the source tree name for package $1 -- Filename = $2 -- ])
  fi
])

dnl -------------------------------------------------------------
dnl $1 = Package Name
dnl $2 = Source tree location
dnl $3 = Tarball location
dnl -------------------------------------------------------------
AC_DEFUN([CHECK_SOURCE_RECOMPILATION_HASH],
[
  PREFIX_PRINTN([Checking whether $1 sources need compilation and installation... ])
  # Compute the hash of the source directory - Recompile only if sources have changed
  # ac_cv_sha_moabcpp="`find $moab_src_dir -name '*.cpp' \( -exec $HASHPRGM "$PWD"/{} \; -o -print \) | $HASHPRGM | cut -d ' ' -f1`"
  # defaultshasum="`find $2 -type f -regex '.*\(hpp\|cpp\|c\|h\|f\|f90\)$' \( -exec $HASHPRGM {} \; -o -print \) | $HASHPRGM | cut -d ' ' -f1`"
  # defaultshasum="`find $2/src $2/Source $2/SRC $2/include $2/inc $2/INC -type f -regex '.*\(hpp\|cpp\|c\|h\|f\|f90\)$' | xargs ls -al | $HASHPRGM | cut -d ' ' -f1`"
  defaultshasum="`cd $2/..; tar -tf $3 | xargs ls -l | $HASHPRGM | cut -d ' ' -f1`"
  AC_CACHE_VAL([ac_cv_sha_$1], [ac_cv_sha_$1="0"])
  if (test "$defaultshasum" != "$ac_cv_sha_$1" || test $need_configuration != false); then
    recompile_and_install=true
    ac_cv_sha_$1="$defaultshasum"
    AC_MSG_RESULT(yes)
  else
    recompile_and_install=false
    AC_MSG_RESULT(no)
  fi
])

dnl -------------------------------------------------------------
dnl Print out whether the configure, build, and install steps were succcessful
dnl -------------------------------------------------------------
AC_DEFUN([PRINT_AUTOMATION_STATUS],
[
  PREFIX_PRINT(Automation Status: )
  COLOR_PRINT([        xx  Configuration := ], m4_tolower($1)_configured)
  COLOR_PRINT([        xx  Build         := ], m4_tolower($1)_made)
  COLOR_PRINT([        xx  Installation  := ], m4_tolower($1)_installed)
])


dnl -------------------------------------------------------------
dnl AUSCM_CONFIGURE_EXTERNAL_PACKAGE(PACKAGE_NAME, DOWNLOAD_URL, DEFAULT_BEHAVIOR)
dnl Example: 
dnl AUSCM_CONFIGURE_EXTERNAL_PACKAGE(MOAB, "http://ftp.mcs.anl.gov/pub/fathom/moab-4.6-nightly.tar.gz" )
dnl -------------------------------------------------------------
AC_DEFUN([AUSCM_CONFIGURE_EXTERNAL_PACKAGE],
[
  #m4_pushdef([pkg_short_name],[m4_tolower(m4_defn([CURRENT_PACKAGE]))])dnl
  pkg_short_name=m4_tolower($1)
  pkg_download_url="$2"
  current_build_dir=`pwd`
  pkg_basesrcdir="$MOAB_SANDBOX/$pkg_short_name"
  pkg_srcdir="$pkg_basesrcdir"
  download_ext_package=no

  # The default PACKAGE installation is under libraries
  pkg_install_dir="$MOAB_ARCH_DIR"
 
  AC_ARG_DOWNLOAD(m4_tolower($1),
    [AS_HELP_STRING([--download-m4_tolower($1)],[Download and configure $1 with default options (URL:$2)])],
    [case "x${downloadval}" in
		  xyes)  pkg_download_url="$2";   m4_tolower(enable$1)=yes;    download_ext_package=yes ;;
      x)     pkg_download_url="$2";      m4_tolower(enable$1)=yes;    download_ext_package=yes ;;
      *)     pkg_download_url="$downloadval"; m4_tolower(enable$1)=yes;   download_ext_package=yes ;;
      xno)   pkg_download_url="none";  download_ext_package=no ;;
		esac],
    [pkg_download_url="$2"; download_ext_package=$3])

  if (test "$download_ext_package" != "no") ; then

    # The default PACKAGE installation is under libraries
    AS_MKDIR_P("$MOAB_ARCH_DIR")
    AS_MKDIR_P("$MOAB_PACKAGES_DIR")

    # Check if the directory already exists
    # if found, we have already configured and linked the sources - do nothing
    # else, download, configure and make PACKAGE sources
    if (test ! -d "$pkg_install_dir" ); then
      AS_MKDIR_P($pkg_install_dir)
    fi
    
    if (test ! -d "$pkg_srcdir" ); then
      AS_MKDIR_P($pkg_srcdir)
    fi

    # Download the archive file containing the sources
    need_configuration=false
    need_build=false
    need_installation=false
    
    PPREFIX="$1"
    pkg_archive_name="`basename $pkg_download_url`"

	  MSG_ECHO_SEPARATOR

    # Check if we need to download an archive file
    DOWNLOAD_EXTERNAL_PACKAGE([$1], [$pkg_download_url], [$MOAB_PACKAGES_DIR/$pkg_archive_name])
    
    # Deflate the archive file containing the sources, if needed
    DEFLATE_EXTERNAL_PACKAGE([$1], [$MOAB_PACKAGES_DIR/$pkg_archive_name], [$pkg_srcdir])

    # Invoke the package specific configuration and build commands
    
    #m4_expand(m4_toupper([DEFAULT_CONFIGURE_MAKE_$1])([$1],"$pkg_srcdir","$pkg_install_dir", "$pkg_archive_name"))
    
    # Due to differences in autoconf we need to check if we should use m4_expand to call the package specific macros
    # Run the package preprocess and configure macros found in the package specific .m4 files
    m4_version_prereq(2.64, [ 
    	m4_expand(m4_toupper([AUSCM_AUTOMATED_SETUP_PREPROCESS_$1])([$1],"$pkg_srcdir","$pkg_install_dir", "$pkg_archive_name"))dnl
    	m4_expand(m4_toupper([AUSCM_AUTOMATED_CONFIGURE_$1])([$need_configuration]))dnl
    ],[
      	m4_toupper([AUSCM_AUTOMATED_SETUP_PREPROCESS_$1])([$1],"$pkg_srcdir","$pkg_install_dir", "$pkg_archive_name")dnl
    	  m4_toupper([AUSCM_AUTOMATED_CONFIGURE_$1])([$need_configuration])dnl
    ])

    CHECK_SOURCE_RECOMPILATION_HASH([$1],[$pkg_srcdir],[$MOAB_PACKAGES_DIR/$pkg_archive_name])
    
    # Run the build, install, and postprocess macros found in the package specific .m4 files.
    m4_version_prereq(2.64, [
    	m4_expand(m4_toupper([AUSCM_AUTOMATED_BUILD_$1])([$need_build]))dnl
    	m4_expand(m4_toupper([AUSCM_AUTOMATED_INSTALL_$1])([$need_installation]))dnl
    	m4_expand(m4_toupper([AUSCM_AUTOMATED_SETUP_POSTPROCESS_$1])([$1]))dnl
    ],[
	    m4_toupper([AUSCM_AUTOMATED_BUILD_$1])([$need_build])dnl
	    m4_toupper([AUSCM_AUTOMATED_INSTALL_$1])([$need_installation])dnl
	    m4_toupper([AUSCM_AUTOMATED_SETUP_POSTPROCESS_$1])([$1])dnl
    ])
    PRINT_AUTOMATION_STATUS([$1])

    if ( [$]m4_tolower($1)[]_configured && [$]m4_tolower($1)[]_made && [$]m4_tolower($1)[]_installed ) ; then
      pkg_status="yes"
    else
      pkg_status="no"
    fi

    # Determine if the installation process was successful
    if ( test -f $pkg_basesrcdir/install_[]m4_tolower($1).log && test "$pkg_status" != "no" ); then
      PREFIX_PRINT([Successful configure/build/install automated  (status=${GREEN}SUCCESS${NORMAL})])
      PREFIX_PRINT([Installed package under: $pkg_install_dir])
    else
      PREFIX_PRINT([Failed configure/build/install step  (status=${RED}FAILURE${NORMAL})])
    fi

	  MSG_ECHO_SEPARATOR

  fi  # if (test "$download_ext_package" != no) ; then 

  m4_tolower(download$1)="$download_ext_package"
  AC_SUBST(m4_tolower(download$1))

])

dnl ------------------------------------------------------------
dnl  Defines macros for printing colors, 
dnl  copying symlinks, and custom 
dnl  printing definitions.
dnl ------------------------------------------------------------
AC_DEFUN([COLOR_PRINT],
[
  if (test "x$PPREFIX" != "x"); then
    if [ ${$2} ]; then
      PREFIX_PRINT([$1 ${GREEN}SUCCESS${NORMAL}])
    else
      PREFIX_PRINT([$1 ${RED}FAIL${NORMAL}])
    fi
  else
    if [ ${$2} ]; then
      MSG_ECHO_CUSTOM([$1 ${GREEN}SUCCESS${NORMAL}])
    else
      MSG_ECHO_CUSTOM([$1 ${RED}FAIL${NORMAL}])
    fi
  fi
])


AC_DEFUN([RECURSIVE_COPY_DIR_SYMLINKS],
[
  if (test x$1 != x && test x$2 != x); then
    recs_srcdir="$1"
    recs_targetdir="$2"
    _AS_ECHO_LOG([--- Recursively copy directory symlinks ($PPREFIX) ---])
    if (test ! -d $recs_targetdir); then
      AS_MKDIR_P($recs_targetdir)
    fi
    _AS_ECHO_LOG([ Source directory: $recs_srcdir ])
    _AS_ECHO_LOG([ Target directory: $recs_targetdir ])
    recs_dirs="`rsync -ain $recs_srcdir/* $recs_targetdir --exclude '.svn' --exclude '.git' | grep 'cd+++' | cut -d ' ' -f2 `"
    if (test "x$recs_dirs" != "x"); then
      for recs_dname in $recs_dirs; do
        _AS_ECHO_LOG([Executing: $MKDIR_P $recs_targetdir/$recs_dname ])
        recs_mkdir_log="`mkdir -p $recs_targetdir/$recs_dname`"
      done
    fi
    recs_files="`rsync -ain $recs_srcdir/* $recs_targetdir --exclude '.svn' --exclude '.git' | grep 'f+++' | cut -d ' ' -f2`"
    for recs_fname in $recs_files; do
      _AS_ECHO_LOG([Executing: $LN_S -f $recs_srcdir/$recs_fname $recs_targetdir/$recs_fname ])
      recs_symlink_log="`$LN_S -f $recs_srcdir/$recs_fname $recs_targetdir/$recs_fname`"
    done
  fi
])


# Finds the parent path of a file or directory
# AC_FIND_ABSPATH(PATH TO A FILE OR DIR)
# ------------------------
m4_define([AC_FIND_ABSPATH], ["`perl -e 'use Cwd "abs_path";print abs_path(shift)' $1 | xargs dirname`"])


# Finds the parent path of a file or directory
# PREFIX_PRINT(PATH TO A FILE OR DIR)
# ------------------------
m4_define([PREFIX_PRINT], 
[_AS_ECHO_LOG([[[ $PPREFIX ]] --   $1 ]);
  AS_ECHO(["[[ $PPREFIX ]] --   $1 "])])


# Finds the parent path of a file or directory
# PREFIX_PRINTN(PATH TO A FILE OR DIR)
# ------------------------
m4_define([PREFIX_PRINTN], 
[_AS_ECHO_LOG([[[ $PPREFIX ]] --   $1 ]);
  AS_ECHO_N(["[[ $PPREFIX ]] --   $1 "])])


# MSG_ECHO_LOG(MESSAGE)
# ------------------------
m4_define([MSG_ECHO_LOG],
[ _AS_ECHO_LOG([$1]) ])

# MSG_ECHO_CUSTOM(MESSAGE)
# ------------------------
m4_define([MSG_ECHO_CUSTOM],
[_AS_ECHO_LOG([$1]);
  AS_ECHO(["$1"])])

# MSG_ECHON_CUSTOM(MESSAGE)
# ------------------------
m4_define([MSG_ECHON_CUSTOM],
[_AS_ECHO_LOG([$1]);
  AS_ECHO(["$1"])])

# MSG_ECHO_SEPARATOR
# ------------------------
m4_define([MSG_ECHO_SEPARATOR],
[ MSG_ECHO_CUSTOM([*** ================================================================================================================== ***]) ])

# Finds the parent path of a file or directory
# ECHO_EVAL(PATH TO A FILE, COMMAND)
# ------------------------
m4_define([ECHO_EVAL], 
[ echo "$2" >> $1;
  eval $2
])

##########################################
###    HDF5 AUTOMATED CONFIGURATION
##########################################

dnl
dnl Arguments:
dnl   1) Default Version Number,
dnl   2) Download by default ?
dnl
AC_DEFUN([AUSCM_CONFIGURE_DOWNLOAD_HDF5],[

  # Check whether user wants to autodownload HDF5
  # Call package Download/Configure/Installation procedures for HDF5, if requested by user
  PPREFIX=HDF5

  # Set the default HDF5 download version
  m4_pushdef([HDF5_DOWNLOAD_VERSION],[$1])dnl

  # Invoke the download-hdf5 command
  m4_case( HDF5_DOWNLOAD_VERSION, [1.8.15], [ AUSCM_CONFIGURE_EXTERNAL_PACKAGE([HDF5], [http://www.hdfgroup.org/ftp/HDF5/releases/hdf5-1.8.15-patch1/src/hdf5-1.8.15-patch1.tar.gz], [$2] ) ],
                                  [1.8.14], [ AUSCM_CONFIGURE_EXTERNAL_PACKAGE([HDF5], [http://www.hdfgroup.org/ftp/HDF5/releases/hdf5-1.8.14/src/hdf5-1.8.14.tar.gz], [$2] ) ],
                                  [1.8.12], [ AUSCM_CONFIGURE_EXTERNAL_PACKAGE([HDF5], [http://www.hdfgroup.org/ftp/HDF5/releases/hdf5-1.8.12/src/hdf5-1.8.12.tar.gz], [$2] ) ],
                                  [1.8.10], [ AUSCM_CONFIGURE_EXTERNAL_PACKAGE([HDF5], [http://www.hdfgroup.org/ftp/HDF5/releases/hdf5-1.8.10/src/hdf5-1.8.10.tar.gz], [$2] ) ],
                                  [ AUSCM_CONFIGURE_EXTERNAL_PACKAGE([HDF5], [http://www.hdfgroup.org/ftp/HDF5/releases/hdf5-1.8.4/src/hdf5-1.8.14.tar.gz], [$2] ) ] )

  if (test "x$downloadhdf5" == "xyes") ; then
    # download the latest HDF5 sources, configure and install
    HDF5_SRCDIR="$hdf5_src_dir"
    AC_SUBST(HDF5_SRCDIR)
    # The default HDF5 installation is under libraries
    HDF5_DIR="$hdf5_install_dir"
    enablehdf5=yes
  fi  # if (test "$downloadhdf5" != no)
])


dnl ---------------------------------------------------------------------------
dnl AUSCM_AUTOMATED SETUP PREPROCESS HDF5
dnl   Figure out what needs to be done to get a valid HDF5 installation.
dnl   Arguments: [PACKAGE, SRC_DIR, INSTALL_DIR, NEED_CONFIGURATION)
dnl ---------------------------------------------------------------------------
AC_DEFUN([AUSCM_AUTOMATED_SETUP_PREPROCESS_HDF5],[
  # uncompress and configure PACKAGE
  hdf5_src_dir=$2
  hdf5_build_dir=$2/build
  hdf5_install_dir=$3
  hdf5_archive_name=$4
  PPREFIX=HDF5

  if (test ! -d "$hdf5_src_dir" || test ! -f "$hdf5_src_dir/configure" ); then
    AC_MSG_ERROR([Invalid source configuration for HDF5. Source directory $hdf5_src_dir is invalid])
  fi

  # determine what steps we need to take to install hdf5
  hdf5_configured=false
  hdf5_made=false
  hdf5_installed=false
  if (test ! -d "$hdf5_build_dir" ); then
    AS_MKDIR_P( $hdf5_build_dir )
  else
    if (test -f "$hdf5_build_dir/src/H5config.h" ); then
      hdf5_configured=true
      if (test -f "$hdf5_build_dir/src/.libs/libhdf5.a" || test -f "$hdf5_build_dir/src/.libs/libhdf5.so" || test -f "$hdf5_build_dir/src/.libs/libhdf5.dylib" ); then
        hdf5_made=true
        if (test -f "$hdf5_install_dir/lib/libhdf5.settings"); then
          hdf5_installed=true
        fi
      fi
    fi
  fi
  # send the information back
  AS_IF([ ! $hdf5_configured || $need_configuration ], [need_configuration=true], [need_configuration=false])
  AS_IF([ ! $hdf5_made || $need_configuration ], [need_build=true], [need_build=false])
  AS_IF([ ! $hdf5_installed || $need_configuration ], [need_installation=true], [need_installation=false])
])


dnl ---------------------------------------------------------------------------
dnl AUSCM_AUTOMATED SETUP POSTPROCESS HDF5
dnl   Dummy macro to fit standard call pattern.  Tells MOAB we have HDF5.
dnl   Arguments: [PACKAGE, SRC_DIR, INSTALL_DIR, NEED_CONFIGURATION)
dnl ---------------------------------------------------------------------------
AC_DEFUN([AUSCM_AUTOMATED_SETUP_POSTPROCESS_HDF5],[
  # we have already checked configure/build/install logs for errors before getting here..
  enablehdf5=yes
  DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --with-hdf5=\"${hdf5_install_dir}\""
])


dnl ---------------------------------------------------------------------------
dnl AUSCM_AUTOMATED CONFIGURE HDF5
dnl   Runs configure for HDF5 and looks for header files.
dnl   Arguments: [NEED_CONFIGURATION)
dnl ---------------------------------------------------------------------------
AC_DEFUN([AUSCM_AUTOMATED_CONFIGURE_HDF5],[
if [ $1 ]; then
  # configure HDF5
  if [ $need_configuration ]; then
    # configure PACKAGE with a minimal build: MPI
    compiler_opts="CC=$CC CXX=$CXX FC=$FC F90=$FC F77=$F77 MPIEXEC=$MPIEXEC"
    configure_command="$hdf5_src_dir/configure --prefix=$hdf5_install_dir --libdir=$hdf5_install_dir/lib --with-pic=1 $compiler_opts"
    # configure_command="$configure_command --enable-cxx --enable-unsupported"
    # VSM: Adding --enable-debug=all is causing problems in h5legacy test. So disabling debug symbols for HDF5.
    #if (test "$enable_debug" != "no"); then
    #  configure_command="$configure_command --enable-debug"
    #fi
    if (test "$enable_shared" != "no"); then
      configure_command="$configure_command --enable-shared"
    fi
    if (test "$enablefortran" != "no"); then
      configure_command="$configure_command --enable-fortran"
    fi
    if (test "$enable_cxx_optimize" != "no"); then
      configure_command="$configure_command --enable-production=yes"
    fi
    if (test "$enablempi" != "no"); then
      configure_command="$configure_command --enable-parallel"
    fi
    
    hdf5_configlog=`echo "Using configure command :==> cd $hdf5_build_dir && $configure_command > $hdf5_src_dir/../config_hdf5.log > $hdf5_src_dir/../config_hdf5.log"`
    PREFIX_PRINT(Configuring with default options  {debug=$enable_debug production=$enable_cxx_optimize shared=$enable_shared parallel=$enablempi} )
    hdf5_configlog="`cd $hdf5_build_dir && $configure_command >> $hdf5_src_dir/../config_hdf5.log 2>&1 && cd \"\$OLDPWD\"`"
  fi

  # check if configuration - current or previous was successful
  if (test ! -f "$hdf5_build_dir/src/H5config.h" ); then
    AC_MSG_ERROR([HDF5 configuration was unsuccessful. Please refer to $hdf5_build_dir/config.log and $hdf5_src_dir/../config_hdf5.log for further details.])
  fi
  hdf5_configured=true
fi
])


dnl ---------------------------------------------------------------------------
dnl AUSCM_AUTOMATED BUILD HDF5
dnl   Calls make on HDF5 and looks for libraries.
dnl   Arguments: [NEED_BUILD)
dnl ---------------------------------------------------------------------------
AC_DEFUN([AUSCM_AUTOMATED_BUILD_HDF5],
[
  # if we need to build then call make all
  if [ $1 ]; then
    if [ $recompile_and_install || $need_build ]; then
      PREFIX_PRINT(Building the sources in parallel )
      hdf5_makelog="`make --no-print-directory -C $hdf5_build_dir all -j4 > $hdf5_src_dir/../make_hdf5.log 2>&1`"
    fi
  fi
  # check if it worked
  if (test -f "$hdf5_build_dir/src/.libs/libhdf5.a" || test -f "$hdf5_build_dir/src/.libs/libhdf5.so" || test -f "$hdf5_build_dir/src/.libs/libhdf5.dylib") ; then
    hdf5_made=true
  else
    AC_MSG_ERROR([HDF5 build was unsuccessful. Please refer to $hdf5_src_dir/../make_hdf5.log for further details.])
  fi
])


dnl ---------------------------------------------------------------------------
dnl AUSCM_AUTOMATED INSTALL HDF5
dnl   Calls make install on HDF5 and checks for libhdf5.settings
dnl   Arguments: [NEED_INSTALLATION)
dnl ---------------------------------------------------------------------------
AC_DEFUN([AUSCM_AUTOMATED_INSTALL_HDF5],
[
  # if we need to install then call make install
  if [ $1 ]; then
    if [ $recompile_and_install ]; then
      if [ $hdf5_installed ]; then
        hdf5_installlog="`make --no-print-directory -C $hdf5_build_dir uninstall > $hdf5_src_dir/../uninstall_hdf5.log 2>&1`"
      fi
      PREFIX_PRINT(Installing the headers and libraries in to directory ($hdf5_install_dir) )
      hdf5_installlog="`make --no-print-directory -C $hdf5_build_dir install > $hdf5_src_dir/../install_hdf5.log 2>&1`"
    fi
  fi
  # check if it worked
  if (test -f "$hdf5_install_dir/lib/libhdf5.settings"); then
    hdf5_installed=true
  else
    AC_MSG_ERROR([HDF5 installation was unsuccessful. Please refer to $hdf5_src_dir/../install_hdf5.log for further details.])
  fi
])


##########################################
###    NetCDF AUTOMATED CONFIGURATION
##########################################

dnl
dnl Arguments:
dnl   1) Default Version Number,
dnl   2) Download by default ?
dnl
AC_DEFUN([AUSCM_CONFIGURE_DOWNLOAD_NETCDF],[

  # Check whether user wants to autodownload NetCDF
  # Call package Download/Configure/Installation procedures for NetCDF, if requested by user
  PPREFIX=NetCDF

  # Set the default NetCDF download version
  m4_pushdef([NETCDF_DOWNLOAD_VERSION],[$1])dnl

  # Invoke the download-netcdf command
  m4_case( NETCDF_DOWNLOAD_VERSION, [4.3.3], [ AUSCM_CONFIGURE_EXTERNAL_PACKAGE([NetCDF], [ftp://ftp.unidata.ucar.edu/pub/netcdf/netcdf-4.3.3.1.tar.gz], [$2] ) ],
                                  [4.3.2], [ AUSCM_CONFIGURE_EXTERNAL_PACKAGE([NetCDF], [ftp://ftp.unidata.ucar.edu/pub/netcdf/old/netcdf-4.3.2.tar.gz], [$2] ) ],
                                  [4.2.0], [ AUSCM_CONFIGURE_EXTERNAL_PACKAGE([NetCDF], [ftp://ftp.unidata.ucar.edu/pub/netcdf/old/netcdf-4.2.1.1.tar.gz], [$2] ) ],
                                  [ AUSCM_CONFIGURE_EXTERNAL_PACKAGE([NetCDF], [ftp://ftp.unidata.ucar.edu/pub/netcdf/old/netcdf-4.3.2.tar.gz], [$2] ) ] )

  if (test "x$downloadnetcdf" == "xyes") ; then
    # download the latest NetCDF sources, configure and install
    NETCDF_SRCDIR="$netcdf_src_dir"
    AC_SUBST(NETCDF_SRCDIR)
    # The default NETCDF installation is under libraries
    NETCDF_DIR="$netcdf_install_dir"
    enablenetcdf=yes
  fi  # if (test "$downloadnetcdf" != no)
])


dnl ---------------------------------------------------------------------------
dnl AUSCM_AUTOMATED SETUP PREPROCESS NetCDF
dnl   Prepares the system for an existing NETCDF install or sets flags to
dnl   install a new copy of NetCDF
dnl   Arguments: [PACKAGE, SRC_DIR, INSTALL_DIR, NEED_CONFIGURATION]
dnl ---------------------------------------------------------------------------
AC_DEFUN([AUSCM_AUTOMATED_SETUP_PREPROCESS_NETCDF],
[
  # configure PACKAGE
  netcdf_src_dir="$2"
  netcdf_build_dir="$2/build"
  netcdf_install_dir="$3"
  netcdf_archive_name="$4"

  # Check if the NetCDF directory is valid
  if (test ! -d "$netcdf_src_dir" || test ! -f "$netcdf_src_dir/configure" ); then
    AC_MSG_ERROR([Invalid source configuration for NetCDF. Source directory $netcdf_src_dir is invalid])
  fi

  # Check if we need to configure, build, and/or install NETCDF
  netcdf_configured=false
  netcdf_made=false
  netcdf_installed=false
  if (test ! -d "$netcdf_build_dir" ); then
   AS_MKDIR_P( $netcdf_build_dir )
  else
    if (test -f "$netcdf_build_dir/nc-config" ); then
      netcdf_configured=true
      if (test -f "$netcdf_build_dir/liblib/.libs/libnetcdf.*" ); then
        netcdf_made=true
        if (test -f "$netcdf_install_dir/include/netcdf.h"); then
          netcdf_installed=true
        fi
      fi
    fi
  fi
  AS_IF([ ! $netcdf_configured || $need_configuration ], [need_configuration=true], [need_configuration=false])
  AS_IF([ ! $netcdf_made || $need_configuration ], [need_build=true], [need_build=false])
  AS_IF([ ! $netcdf_installed || $need_configuration ], [need_installation=true], [need_installation=false])
])


dnl ---------------------------------------------------------------------------
dnl AUSCM_AUTOMATED SETUP POSTPROCESS NETCDF
dnl   Postprocessing for NETCDF is minimal.  Exists for standardization of all
dnl   package macros.
dnl   Arguments: [PACKAGE]
dnl ---------------------------------------------------------------------------
AC_DEFUN([AUSCM_AUTOMATED_SETUP_POSTPROCESS_NETCDF],
[
  # we have already checked configure/build/install logs for
  # errors before getting here..
  enablenetcdf=yes
  DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --with-netcdf=\"${netcdf_install_dir}\""
])


dnl ---------------------------------------------------------------------------
dnl AUSCM_AUTOMATED CONFIGURE NETCDF
dnl   Sets up the configure command and then ensures it ran correctly.
dnl   Arguments: [NEED_CONFIGURATION)
dnl ---------------------------------------------------------------------------
AC_DEFUN([AUSCM_AUTOMATED_CONFIGURE_NETCDF],
[
if [ $1 ]; then
  # configure NETCDF
  if [ $need_configuration ]; then
    # configure PACKAGE with a minimal build: MPI, HDF5, NETCDF
    compiler_opts="CC=$CC CXX=$CXX FC=$FC F90=$FC F77=$F77"
    configure_command="$netcdf_src_dir/configure --prefix=$netcdf_install_dir --libdir=$netcdf_install_dir/lib --with-pic=1 --enable-shared=$enable_shared $compiler_opts"
    if (test "$enablehdf5" != "no"); then
      configure_command="$configure_command --enable-netcdf-4 LDFLAGS=\"$HDF5_LDFLAGS $LDFLAGS\" CPPFLAGS=\"$HDF5_CPPFLAGS\" LIBS=\"$HDF5_LIBS -ldl -lm -lz\""
    else
      configure_command="$configure_command --disable-netcdf-4 LDFLAGS=\"$LDFLAGS\" CPPFLAGS=\"$CPPFLAGS\" LIBS=\"$LIBS\""
    fi
    eval "echo 'Using configure command :==> cd $netcdf_build_dir && $configure_command > $netcdf_src_dir/../config_netcdf.log' > $netcdf_src_dir/../config_netcdf.log"
    PREFIX_PRINT([Configuring with default options  (debug=$enable_debug with-HDF5=$enablehdf5 shared=$enable_shared) ])
    eval "cd $netcdf_build_dir && $configure_command >> $netcdf_src_dir/../config_netcdf.log 2>&1 && cd \"\$OLDPWD\""
  fi

  if (test ! -f "$netcdf_build_dir/nc-config" ); then
    AC_MSG_ERROR([NetCDF configuration was unsuccessful. Please refer to $netcdf_build_dir/config.log and $netcdf_src_dir/../config_netcdf.log for further details.])
  fi
  netcdf_configured=true

fi
])

dnl ---------------------------------------------------------------------------
dnl AUSCM_AUTOMATED BUILD NETCDF
dnl   Builds NETCDF and looks for libNETCDF.
dnl   Arguments: [NEED_BUILD)
dnl ---------------------------------------------------------------------------
AC_DEFUN([AUSCM_AUTOMATED_BUILD_NETCDF],
[
  if [ $1 ]; then
    if [ $recompile_and_install || $need_build ]; then
      PREFIX_PRINT(Building the sources in parallel)
      netcdf_makelog="`make --no-print-directory -C $netcdf_build_dir all -j4 > $netcdf_src_dir/../make_netcdf.log 2>&1`"
    fi
  fi

  #if (test -f "$netcdf_build_dir/liblib/.libs/libnetcdf.*" ); then
  libexist_check="`ls "$netcdf_build_dir/liblib/.libs/libnetcdf.la" &> /dev/null 2>&1`"
  if (test "x$libexist_check" == "x") ; then
    netcdf_made=true
  else
    AC_MSG_ERROR([NetCDF build was unsuccessful. Please refer to $netcdf_src_dir/../make_netcdf.log for further details.])
  fi
])

dnl ---------------------------------------------------------------------------
dnl AUSCM_AUTOMATED INSTALL NETCDF
dnl   Installs NETCDF and checks headers.
dnl   Arguments: [NEED_INSTALLATION)
dnl ---------------------------------------------------------------------------
AC_DEFUN([AUSCM_AUTOMATED_INSTALL_NETCDF],
[
  if [ $1 ]; then
    if [ $recompile_and_install ]; then
      if [ $netcdf_installed ]; then
        netcdf_installlog="`make --no-print-directory -C $netcdf_build_dir uninstall > $netcdf_src_dir/../uninstall_netcdf.log 2>&1`"
      fi
      PREFIX_PRINT(Installing the headers and libraries in to directory {$netcdf_install_dir} )
      netcdf_installlog="`make --no-print-directory -C $netcdf_build_dir install > $netcdf_src_dir/../install_netcdf.log 2>&1`"
    fi
  fi

  if (test -f "$netcdf_install_dir/include/netcdf.h"); then
    netcdf_installed=true
  else
    AC_MSG_ERROR([NetCDF installation was unsuccessful. Please refer to $netcdf_src_dir/../install_netcdf.log for further details.])
  fi
])



##########################################
###    Metis AUTOMATED CONFIGURATION
##########################################

dnl
dnl Arguments:
dnl   1) Default Version Number,
dnl   2) Download by default ?
dnl
AC_DEFUN([AUSCM_CONFIGURE_DOWNLOAD_METIS],[

  # Check whether user wants to autodownload Metis
  # Call package Download/Configure/Installation procedures for Metis, if requested by user
  PPREFIX=Metis

  # Set the default Metis download version
  m4_pushdef([METIS_DOWNLOAD_VERSION],[$1])dnl

  metis_manual_install=no
  if (test "METIS_DOWNLOAD_VERSION" == "4.0.3"); then
    metis_manual_install=yes
    parmetis_manual_install=yes
    m4_pushdef([PARMETIS_DOWNLOAD_VERSION],[3.2.0])dnl
  else
    m4_pushdef([PARMETIS_DOWNLOAD_VERSION],[4.0.3])dnl
    parmetis_manual_install=no
  fi

  # Invoke the download-metis command
  m4_case( METIS_DOWNLOAD_VERSION, [4.0.3], [ AUSCM_CONFIGURE_EXTERNAL_PACKAGE([METIS], [http://glaros.dtc.umn.edu/gkhome/fetch/sw/metis/OLD/metis-4.0.3.tar.gz], [$2] ) ],
                                   [5.1.0], [ AUSCM_CONFIGURE_EXTERNAL_PACKAGE([METIS], [http://glaros.dtc.umn.edu/gkhome/fetch/sw/metis/metis-5.1.0.tar.gz], [$2] ) ],
                                   [5.1.0p2], [ AUSCM_CONFIGURE_EXTERNAL_PACKAGE([METIS], [http://ftp.mcs.anl.gov/pub/petsc/externalpackages/metis-5.1.0-p2.tar.gz], [$2] ) ],
                                  [ AUSCM_CONFIGURE_EXTERNAL_PACKAGE([METIS], [http://glaros.dtc.umn.edu/gkhome/fetch/sw/metis/metis-5.1.0.tar.gz], [$2] ) ] )

  if (test "x$downloadmetis" == "xyes") ; then
    # download the latest METIS sources, configure and install
    METIS_SRCDIR="$metis_src_dir"
    AC_SUBST(METIS_SRCDIR)
    # The default METIS installation is under libraries
    METIS_DIR="$metis_install_dir"
    enablemetis=yes
  fi  # if (test "$downloadmetis" != no)

])


dnl ---------------------------------------------------------------------------
dnl AUTOMATED SETUP PREPROCESS METIS
dnl   Figure out what needs to be done to get a valid METIS installation.
dnl   Arguments: [PACKAGE, SRC_DIR, INSTALL_DIR, NEED_CONFIGURATION)
dnl ---------------------------------------------------------------------------
AC_DEFUN([AUSCM_AUTOMATED_SETUP_PREPROCESS_METIS],[
  # uncompress and configure PACKAGE
  metis_src_dir="$2"
  metis_build_dir="$2"
  metis_install_dir="$3"
  metis_archive_name="$4"
  PPREFIX=METIS

  if (test ! -d "$metis_src_dir" ); then
  	AC_MSG_ERROR([Invalid source configuration for Metis. Source directory $metis_src_dir is invalid])
  fi

  # determine what steps we need to take to install metis
  metis_configured=false
  metis_made=false
  metis_installed=false
  if (test ! -d "$metis_build_dir" ); then
    AS_MKDIR_P( $metis_build_dir )
  else
    metis_configured=true
    metis_cputype=`uname -m | sed "s/\\ /_/g"`
    metis_systype=`uname -s`
    metis_subbuilddir=$metis_build_dir/build/$metis_systype-$metis_cputype
    if (test -f "$metis_build_dir/libmetis.a" || test -f "$metis_subbuilddir/libmetis/libmetis.a" || test -f "$metis_subbuilddir/libmetis/libmetis.so" || test -f "$metis_subbuilddir/libmetis/libmetis.dylib"); then
      metis_made=true
      if (test -f "$metis_install_dir/include/metis.h"); then
        metis_installed=true
      fi
    fi
  fi
  # send the information back
  AS_IF([ ! $metis_configured || $need_configuration ], [need_configuration=true], [need_configuration=false])
  AS_IF([ ! $metis_made || $need_configuration ], [need_build=true], [need_build=false])
  AS_IF([ ! $metis_installed || $need_configuration ], [need_installation=true], [need_installation=false])
])


dnl ---------------------------------------------------------------------------
dnl AUTOMATED SETUP POSTPROCESS METIS
dnl   Dummy macro to fit standard call pattern.  Tells SHARP we have METIS.
dnl   Arguments: [PACKAGE, SRC_DIR, INSTALL_DIR, NEED_CONFIGURATION)
dnl ---------------------------------------------------------------------------
AC_DEFUN([AUSCM_AUTOMATED_SETUP_POSTPROCESS_METIS],[
  # we have already checked configure/build/install logs for errors before getting here..
  enablemetis=yes
  DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --with-metis=\"${metis_install_dir}\""
])

dnl ---------------------------------------------------------------------------
dnl AUTOMATED CONFIGURE METIS
dnl   Calls configure with necessary options.
dnl   Arguments: [SOURCE_DIRECTORY, INSTALL_DIRECTORY)
dnl ---------------------------------------------------------------------------
AC_DEFUN([AUSCM_AUTOMATED_CONFIGURE_METIS],
[
if [ $1 ]; then
  # configure METIS
  if [ $need_configuration ]; then
    if (test "$metis_manual_install" != "yes"); then
      metis_use_cmake="no"
      # configure PACKAGE with a minimal build: MPI
      #export CFLAGS="$CFLAGS -fPIC -DPIC" CXXFLAGS="$CXXFLAGS -fPIC -DPIC" FCFLAGS="$FCFLAGS -fPIC" FFLAGS="$FFLAGS -fPIC" LDFLAGS="$LDFLAGS"
      # echo "export CC=$CC CXX=$CXX CFLAGS=\"$CFLAGS -fPIC -DPIC\" CXXFLAGS=\"$CXXFLAGS -fPIC -DPIC\" LDFLAGS=\"$LDFLAGS\"" > $metis_src_dir/../config_metis.log
      if (test "$metis_use_cmake" != "yes"); then
        configure_command="make config cc=\"$CC\" cxx=\"$CXX\" prefix=$metis_install_dir gklib_path=$metis_build_dir/GKlib"
      else
        configure_command="cmake $metis_src_dir -DCMAKE_INSTALL_PREFIX=$metis_install_dir -DCMAKE_C_COMPILER=$CC -DCMAKE_CXX_COMPILER=$CXX -DGKLIB_PATH=$metis_build_dir/GKlib"
      fi
      if (test "$enable_debug" != "no"); then
        if (test "$metis_use_cmake" != "yes"); then
          configure_command="$configure_command debug=1"
      	else
      	  configure_command="$configure_command -DDEBUG=1"
        fi
      fi
      if (test "$enable_shared" != "no"); then
        if (test "$metis_use_cmake" != "yes"); then
          configure_command="$configure_command shared=1"
        else
      	  configure_command="$configure_command -DSHARED=1"
        fi
      fi
      eval "echo 'Using configure command :==> cd $metis_build_dir && $configure_command > $metis_src_dir/../config_metis.log' >> $metis_src_dir/../config_metis.log"
      PREFIX_PRINT(Configuring with default options  {debug=$enable_debug} )
      eval "cd $metis_build_dir && $configure_command >> $metis_src_dir/../config_metis.log 2>&1 && cd \"\$OLDPWD\""
    fi
  fi
  
  if (test "$metis_manual_install" != "no"); then
    touch $metis_src_dir/../config_metis.log
  fi

  # check if configuration - current or previous was successful
  if (test "$parmetis_manual_install" != "yes"); then
    metis_cputype=`uname -m | sed "s/\\ /_/g"`
    metis_systype=`uname -s`
    metis_subbuilddir=$metis_build_dir/build/$metis_systype-$metis_cputype
    if(test ! -f "$metis_subbuilddir/cmake_install.cmake" ); then # only generated when CMake succeeds
  	  AC_MSG_ERROR([Metis configuration was unsuccessful. Please refer to $metis_src_dir/../config_metis.log for further details.])
	  fi
  fi

  metis_configured=true
fi
])


dnl ---------------------------------------------------------------------------
dnl AUTOMATED BUILD METIS
dnl   Builds METIS and sets prefix to the install directory
dnl   Arguments: [SOURCE_DIRECTORY, INSTALL_DIRECTORY)
dnl ---------------------------------------------------------------------------
AC_DEFUN([AUSCM_AUTOMATED_BUILD_METIS],
[
  # if we need to build then call make all
  if [ $1 ]; then
    if [ $recompile_and_install || $need_build ]; then
      PREFIX_PRINT(Building the sources in parallel )
      if (test "$metis_manual_install" != "no"); then
        eval "make --no-print-directory -C $metis_build_dir CC=\"$CC\" OPTFLAGS=\"$CFLAGS -fPIC -DPIC\" -j4 > $metis_src_dir/../make_metis.log 2>&1"
      else
        eval "make --no-print-directory -C $metis_build_dir all -j4 > $metis_src_dir/../make_metis.log 2>&1"
      fi
    fi
  fi

  # check if it worked
  if (test "$metis_manual_install" != "no"); then
    if (test -f "$metis_build_dir/libmetis.a" || test -f "$metis_build_dir/libmetis.so" || test -f "$metis_build_dir/libmetis.dylib") ; then
      metis_made=true
    else
      AC_MSG_ERROR([Metis build was unsuccessful. Please refer to $metis_src_dir/../make_metis.log for further details.])
    fi
  else
    metis_cputype=`uname -m | sed "s/\\ /_/g"`
    metis_systype=`uname -s`
    metis_subbuilddir=$metis_build_dir/build/$metis_systype-$metis_cputype
    if (test -f "$metis_subbuilddir/libmetis/libmetis.a" || test -f "$metis_subbuilddir/libmetis/libmetis.so" || test -f "$metis_subbuilddir/libmetis/libmetis.dylib") ; then
      metis_made=true
    else
      AC_MSG_ERROR([Metis build was unsuccessful. Please refer to $metis_src_dir/../make_metis.log for further details.])
    fi
  fi
])


dnl ---------------------------------------------------------------------------
dnl AUTOMATED INSTALL METIS
dnl   Installs METIS into the install directory and ensures the library exists.
dnl   Arguments: [SOURCE_DIRECTORY, INSTALL_DIRECTORY)
dnl ---------------------------------------------------------------------------
AC_DEFUN([AUSCM_AUTOMATED_INSTALL_METIS],
[
  # if we need to install then call make install
  if [ $1 ]; then
    if [ $recompile_and_install ]; then
      if [ $metis_installed ]; then
        if (test "$metis_manual_install" != "no"); then
          AUSCM_UNLINK_METIS_403($metis_install_dir, $metis_src_dir/../uninstall_metis.log)
        else
          eval "make --no-print-directory -C $metis_build_dir uninstall > $metis_src_dir/../uninstall_metis.log 2>&1"
        fi
      fi
      PREFIX_PRINT(Installing the headers and libraries in to directory )
      if (test "$metis_manual_install" != "no"); then
        AUSCM_LINK_METIS_403($metis_build_dir, $metis_install_dir, $metis_src_dir/../install_metis.log)
      else
        eval "make --no-print-directory -C $metis_build_dir install > $metis_src_dir/../install_metis.log 2>&1"
      fi
    fi
  fi
  # check if it worked
  if (test -f "$metis_install_dir/include/metis.h" && test -f "$metis_install_dir/lib/libmetis.a"); then
    metis_installed=true
  else
	  AC_MSG_ERROR([Metis installation was unsuccessful. Please refer to $metis_src_dir/../install_metis.log for further details.])
  fi
])

dnl ---------------------------------------------------------------------------
dnl LINK METIS 4.0.3
dnl   A quick way to link the METIS 4.0.3 installation into a more
dnl     traditional layout for easier management.
dnl   Arguments: [metis_src_dir, metis_install_dir, log_file]
dnl ---------------------------------------------------------------------------
AC_DEFUN([AUSCM_LINK_METIS_403],
[
  ECHO_EVAL($3, "mkdir -p $2/bin $2/lib $2/include")
  ECHO_EVAL($3, "cp $1/libmetis.a $2/lib")
  ECHO_EVAL($3, "cp $1/Lib/defs.h $2/include")
  ECHO_EVAL($3, "cp $1/Lib/macros.h $2/include")
  ECHO_EVAL($3, "cp $1/Lib/metis.h $2/include")
  ECHO_EVAL($3, "cp $1/Lib/proto.h $2/include")
  ECHO_EVAL($3, "cp $1/Lib/rename.h $2/include")
  ECHO_EVAL($3, "cp $1/Lib/struct.h $2/include")
  ECHO_EVAL($3, "cp $1/kmetis $2/bin")
  ECHO_EVAL($3, "cp $1/oemetis $2/bin")
  ECHO_EVAL($3, "cp $1/onmetis $2/bin")
  ECHO_EVAL($3, "cp $1/pmetis $2/bin")
  ECHO_EVAL($3, "cp $1/mesh2dual $2/bin")
  ECHO_EVAL($3, "cp $1/mesh2nodal $2/bin")
  ECHO_EVAL($3, "cp $1/partdmesh $2/bin")
  ECHO_EVAL($3, "cp $1/partnmesh $2/bin")
  ECHO_EVAL($3, "cp $1/graphchk $2/bin")
])


dnl ---------------------------------------------------------------------------
dnl LINK METIS 4.0.3
dnl   A quick way to unlink or uninstall the METIS 4.0.3 installation
dnl   Arguments: [metis_install_dir, log_file]
dnl ---------------------------------------------------------------------------
AC_DEFUN([AUSCM_UNLINK_METIS_403],
[
  ECHO_EVAL($2, "rm -f $1/lib/libmetis.a $1/include/defs.h $1/include/macros.h $1/include/metis.h $1/include/proto.h $1/include/rename.h $1/include/struct.h")
  ECHO_EVAL($2, "rm -f $1/bin/kmetis $1/bin/oemetis $1/bin/onmetis $1/bin/pmetis $1/bin/mesh2dual $1/bin/mesh2nodal $1/bin/partdmesh $1/bin/partnmesh $1/bin/graphchk")
])



##########################################
###   ParMetis AUTOMATED CONFIGURATION
##########################################

dnl
dnl Arguments:
dnl   1) Default Version Number,
dnl   2) Download by default ?
dnl
AC_DEFUN([AUSCM_CONFIGURE_DOWNLOAD_PARMETIS],[

  # Check whether user wants to autodownload Metis
  # Call package Download/Configure/Installation procedures for Metis, if requested by user
  PPREFIX=ParMetis

  # Set the default Metis download version
  m4_pushdef([PARMETIS_DOWNLOAD_VERSION],[$1])dnl

  if (test "PARMETIS_DOWNLOAD_VERSION" == ""); then
    m4_pushdef([PARMETIS_DOWNLOAD_VERSION],[4.0.3])dnl
    parmetis_manual_install=no
  elif (test "PARMETIS_DOWNLOAD_VERSION" == "3.2.0"); then
    parmetis_manual_install=yes
  else
    parmetis_manual_install=no
  fi

  # Invoke the download-parmetis command
  m4_case( PARMETIS_DOWNLOAD_VERSION, [4.0.3], [ AUSCM_CONFIGURE_EXTERNAL_PACKAGE([PARMETIS], [http://glaros.dtc.umn.edu/gkhome/fetch/sw/parmetis/parmetis-4.0.3.tar.gz], [$2] ) ],
                                      [3.2.0], [ AUSCM_CONFIGURE_EXTERNAL_PACKAGE([PARMETIS], [http://glaros.dtc.umn.edu/gkhome/fetch/sw/parmetis/OLD/ParMetis-3.2.0.tar.gz], [$2] ) ],
                                      [ AUSCM_CONFIGURE_EXTERNAL_PACKAGE([PARMETIS], [http://glaros.dtc.umn.edu/gkhome/fetch/sw/parmetis/parmetis-4.0.3.tar.gz], [$2] ) ] )


  if (test "x$downloadparmetis" == "xyes") ; then
    # download the latest PARMETIS sources, configure and install
    PARMETIS_SRCDIR="$parmetis_src_dir"
    AC_SUBST(PARMETIS_SRCDIR)
    # The default PARMETIS installation is under libraries
    PARMETIS_DIR="$parmetis_install_dir"
    enableparmetis=yes
    METIS_DIR="$metis_install_dir"
    enablemetis=yes
  fi  # if (test "$downloadmetis" != no)
])


dnl ---------------------------------------------------------------------------
dnl AUTOMATED SETUP PREPROCESS PARMETIS
dnl   Figure out what needs to be done to get a valid PARMETIS installation.
dnl   Arguments: [PACKAGE, SRC_DIR, INSTALL_DIR, NEED_CONFIGURATION)
dnl ---------------------------------------------------------------------------
AC_DEFUN([AUSCM_AUTOMATED_SETUP_PREPROCESS_PARMETIS],[
  # uncompress and configure PACKAGE
  parmetis_src_dir="$2"
  parmetis_build_dir="$2"
  parmetis_install_dir="$3"
  parmetis_archive_name="$4"
  PPREFIX=PARMETIS

  if (test ! -d "$parmetis_src_dir" ); then
  	AC_MSG_ERROR([Invalid source configuration for ParMetis. Source directory $parmetis_src_dir is invalid])
  fi

  # determine what steps we need to take to install parmetis
  parmetis_configured=false
  parmetis_made=false
  parmetis_installed=false
  if (test ! -d "$parmetis_build_dir" ); then
    AS_MKDIR_P( $parmetis_build_dir )
  else
    parmetis_configured=true
    parmetis_cputype=`uname -m | sed "s/\\ /_/g"`
    parmetis_systype=`uname -s`
    parmetis_subbuilddir=$parmetis_build_dir/build/$parmetis_systype-$parmetis_cputype
    if (test -f "$parmetis_build_dir/libparmetis.a" || test -f "$parmetis_subbuilddir/libparmetis/libparmetis.a" || test -f "$parmetis_subbuilddir/libparmetis/libparmetis.so" || test -f "$parmetis_subbuilddir/libparmetis/libparmetis.dylib"); then
      parmetis_made=true
      if (test -f "$parmetis_install_dir/include/parmetis.h"); then
        parmetis_installed=true
      fi
    fi
  fi
  # send the information back
  AS_IF([ ! $parmetis_configured || $need_configuration ], [need_configuration=true], [need_configuration=false])
  AS_IF([ ! $parmetis_made || $need_configuration ], [need_build=true], [need_build=false])
  AS_IF([ ! $parmetis_installed || $need_configuration ], [need_installation=true], [need_installation=false])
])


dnl ---------------------------------------------------------------------------
dnl AUTOMATED SETUP POSTPROCESS PARMETIS
dnl   Dummy macro to fit standard call pattern.  Tells MOAB we have PARMETIS.
dnl   Arguments: [PACKAGE, SRC_DIR, INSTALL_DIR, NEED_CONFIGURATION)
dnl ---------------------------------------------------------------------------
AC_DEFUN([AUSCM_AUTOMATED_SETUP_POSTPROCESS_PARMETIS],[
  # we have already checked configure/build/install logs for errors before getting here..
  enableparmetis=yes
  enablemetis=yes
  DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --with-parmetis=\"${parmetis_install_dir}\""
])


dnl ---------------------------------------------------------------------------
dnl AUTOMATED CONFIGURE PARMETIS
dnl   Calls configure with necessary options.
dnl   Arguments: [SOURCE_DIRECTORY, INSTALL_DIRECTORY)
dnl ---------------------------------------------------------------------------
AC_DEFUN([AUSCM_AUTOMATED_CONFIGURE_PARMETIS],
[
if [ $1 ]; then
  # configure ParMetis
  if [ $need_configuration ]; then
    if (test "$parmetis_manual_install" != "yes"); then
      # configure PACKAGE with a minimal build: MPI
      export CFLAGS="$CFLAGS -fPIC -DPIC" CXXFLAGS="$CXXFLAGS -fPIC -DPIC" FCFLAGS="$FCFLAGS -fPIC" FFLAGS="$FFLAGS -fPIC" LDFLAGS="$LDFLAGS"
      configure_command="make config cc=$CC cxx=$CXX prefix=$parmetis_install_dir"
      if (test "$enable_debug" != "no"); then
        configure_command="$configure_command debug=1"
      fi
      if (test "$enable_shared" != "no"); then
        configure_command="$configure_command shared=1"
      fi
      eval "echo 'Using configure command :==> cd $parmetis_build_dir && $configure_command > $parmetis_src_dir/../config_metis.log' > $parmetis_src_dir/../config_metis.log"
      PREFIX_PRINT(Configuring Metis with default options  {debug=$enable_debug} )
      eval "cd $parmetis_build_dir/metis && $configure_command >> $parmetis_src_dir/../config_metis.log 2>&1 && cd \"\$OLDPWD\""

      configure_command="$configure_command metis_path=$parmetis_build_dir/metis gklib_path=$parmetis_build_dir/metis/GKlib"
      eval "echo 'Using configure command :==> cd $parmetis_build_dir && $configure_command > $parmetis_src_dir/../config_parmetis.log' > $parmetis_src_dir/../config_parmetis.log"
      PREFIX_PRINT(Configuring ParMetis with default options  {debug=$enable_debug with-metis=yes} )
      eval "cd $parmetis_build_dir && $configure_command >> $parmetis_src_dir/../config_parmetis.log 2>&1 && cd \"\$OLDPWD\""
    fi
  fi

  # check if configuration - current or previous was successful
  if (test "$parmetis_manual_install" != "yes"); then
    parmetis_cputype=`uname -m | sed "s/\\ /_/g"`
    parmetis_systype=`uname -s`
    parmetis_subbuilddir=$parmetis_build_dir/build/$parmetis_systype-$parmetis_cputype
    if(test ! -f "$parmetis_subbuilddir/cmake_install.cmake" ); then # only generated when CMake succeeds
  	  AC_MSG_ERROR([ParMetis configuration was unsuccessful. Please refer to $parmetis_src_dir/../config_parmetis.log for further details.])
	  fi
  fi
  parmetis_configured=true
fi
])


dnl ---------------------------------------------------------------------------
dnl AUTOMATED BUILD PARMETIS
dnl   Builds METIS and sets prefix to the install directory
dnl   Arguments: [SOURCE_DIRECTORY, INSTALL_DIRECTORY)
dnl ---------------------------------------------------------------------------
AC_DEFUN([AUSCM_AUTOMATED_BUILD_PARMETIS],
[
  # if we need to build then call make all
  if [ $1 ]; then
    if [ $recompile_and_install || $need_build ]; then
      PREFIX_PRINT(Building the sources in parallel )
      if (test "$parmetis_manual_install" != "no"); then
        eval "make --no-print-directory -C $parmetis_build_dir CC=\"$CC\" OPTFLAGS=\"$CFLAGS\" COPTIONS=\"$CPPFLAGS -fPIC -DPIC\" -j4 > $parmetis_src_dir/../make_parmetis.log 2>&1"
      else
        PREFIX_PRINT(   *** Building Metis ... )
        eval "make --no-print-directory -C $parmetis_build_dir/metis all -j4 > $parmetis_src_dir/../make_metis.log 2>&1"
        PREFIX_PRINT(   *** Building ParMetis ... )
        eval "make --no-print-directory -C $parmetis_build_dir all -j4 >> $parmetis_src_dir/../make_parmetis.log 2>&1"
      fi
    fi
  fi

  # check if it worked
  if (test "$parmetis_manual_install" != "no"); then
    if (test -f "$parmetis_build_dir/libparmetis.a" || test -f "$parmetis_build_dir/libparmetis.so" || test -f "$parmetis_build_dir/libparmetis.dylib") ; then
      parmetis_made=true
    else
      AC_MSG_ERROR([Metis build was unsuccessful. Please refer to $parmetis_src_dir/../make_parmetis.log for further details.])
    fi
  else
    parmetis_cputype=`uname -m | sed "s/\\ /_/g"`
    parmetis_systype=`uname -s`
    parmetis_subbuilddir=$parmetis_build_dir/build/$parmetis_systype-$parmetis_cputype
    if (test -f "$parmetis_subbuilddir/libparmetis/libparmetis.a" || test -f "$parmetis_subbuilddir/libparmetis/libparmetis.so" || test -f "$parmetis_subbuilddir/libparmetis/libparmetis.dylib") ; then
      parmetis_made=true
    else
      AC_MSG_ERROR([ParMetis build was unsuccessful. Please refer to $parmetis_src_dir/../make_parmetis.log for further details.])
    fi
  fi
])



dnl ---------------------------------------------------------------------------
dnl AUTOMATED INSTALL PARMETIS
dnl   Installs PARMETIS into the install directory and ensures the library exists.
dnl   Arguments: [SOURCE_DIRECTORY, INSTALL_DIRECTORY)
dnl ---------------------------------------------------------------------------
AC_DEFUN([AUSCM_AUTOMATED_INSTALL_PARMETIS],
[
  # if we need to install then call make install
  if [ $1 ]; then
    if [ $recompile_and_install ]; then
      if [ $parmetis_installed ]; then
        if (test "$parmetis_manual_install" != "no"); then
          AUSCM_UNLINK_PARMETIS_302($parmetis_install_dir, $parmetis_src_dir/../uninstall_parmetis.log)
        else
          eval "make --no-print-directory -C $parmetis_build_dir/metis uninstall > $parmetis_src_dir/../uninstall_metis.log 2>&1"
          eval "make --no-print-directory -C $parmetis_build_dir uninstall > $parmetis_src_dir/../uninstall_parmetis.log 2>&1"
        fi
      fi
      PREFIX_PRINT(Installing the headers and libraries in to directory )
      if (test "$parmetis_manual_install" != "no"); then
        AUSCM_LINK_PARMETIS_302($parmetis_build_dir, $parmetis_install_dir, $parmetis_src_dir/../install_parmetis.log)
      else
        PREFIX_PRINT(   *** Installing Metis ... )
        eval "make --no-print-directory -C $parmetis_build_dir/metis install > $parmetis_src_dir/../install_metis.log 2>&1"
        PREFIX_PRINT(   *** Installing ParMetis ... )
        eval "make --no-print-directory -C $parmetis_build_dir install > $parmetis_src_dir/../install_parmetis.log 2>&1"
      fi
    fi
  fi
  # check if it worked
  if (test -f "$parmetis_install_dir/include/parmetis.h" && (test -f "$parmetis_install_dir/lib/libparmetis.a" || test -f "$parmetis_install_dir/lib/libparmetis.so" || test -f "$parmetis_install_dir/lib/libparmetis.dylib")); then
    parmetis_installed=true
  else
	  AC_MSG_ERROR([ParMetis installation was unsuccessful. Please refer to $parmetis_src_dir/../install_parmetis.log for further details.])
  fi
])


dnl ---------------------------------------------------------------------------
dnl LINK PARMETIS 3.0.2
dnl   A quick way to link the PARMETIS 3.0.2 installation into a more
dnl     traditional layout for easier management.
dnl   Arguments: [parmetis_src_dir, parmetis_install_dir, log_file]
dnl ---------------------------------------------------------------------------
AC_DEFUN([AUSCM_LINK_PARMETIS_302],
[
  ECHO_EVAL($3, "mkdir -p $2/bin $2/lib $2/include")
  # ParMetis files
  ECHO_EVAL($3, "cp $1/libparmetis.a $2/lib")
  ECHO_EVAL($3, "cp $1/parmetis.h $2/include")
  ECHO_EVAL($3, "cp $1/mtest $2/bin")
  ECHO_EVAL($3, "cp $1/ptest $2/bin")
  ECHO_EVAL($3, "cp $1/pometis $2/bin")
  ECHO_EVAL($3, "cp $1/parmetis $2/bin")
  # Metis files
  ECHO_EVAL($3, "cp $1/libmetis.a $2/lib")
  ECHO_EVAL($3, "cp $1/METISLib/defs.h $2/include")
  ECHO_EVAL($3, "cp $1/METISLib/macros.h $2/include")
  ECHO_EVAL($3, "cp $1/METISLib/metis.h $2/include")
  ECHO_EVAL($3, "cp $1/METISLib/proto.h $2/include")
  ECHO_EVAL($3, "cp $1/METISLib/rename.h $2/include")
  ECHO_EVAL($3, "cp $1/METISLib/struct.h $2/include")
])


dnl ---------------------------------------------------------------------------
dnl LINK METIS 4
dnl   A quick way to unlink or uninstall the METIS 4.0 installation
dnl   Arguments: [parmetis_install_dir, log_file]
dnl ---------------------------------------------------------------------------
AC_DEFUN([AUSCM_UNLINK_PARMETIS_302],
[
  dnl remove Metis installs first
  ECHO_EVAL($2, "rm -f $1/lib/libmetis.a $1/include/defs.h $1/include/macros.h $1/include/metis.h $1/include/proto.h $1/include/rename.h $1/include/struct.h")
  dnl now remove ParMetis installs
  ECHO_EVAL($2, "rm -f $1/lib/libparmetis.a $1/include/parmetis.h")
  ECHO_EVAL($2, "rm -f $1/bin/mtest $1/bin/ptest $1/bin/pometis $1/bin/parmetis")
])

