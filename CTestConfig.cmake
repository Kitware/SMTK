## This file should be placed in the root directory of your project.
## Then modify the CMakeLists.txt file in the root directory of your
## project to incorporate the testing dashboard.
## # The following are required to uses Dart and the Cdash dashboard
##   ENABLE_TESTING()
##   INCLUDE(CTest)
set(CTEST_PROJECT_NAME "SuperBuild-ConceptualModelBuilder")
set(CTEST_NIGHTLY_START_TIME "21:00:00 EDT")

set(drop_sites kitware)

set(CTEST_DROP_METHOD_kitware "https")
set(CTEST_DROP_SITE_kitware "kitware.cdash.org")
set(CTEST_DROP_LOCATION_kitware "/submit.php?project=SMTK")
set(CTEST_DROP_SITE_CDASH_kitware TRUE)

if (NOT DEFINED SMTK_PUBLIC_DROP_SITE OR SMTK_PUBLIC_DROP_SITE)

  list(APPEND drop_sites open)

  set(CTEST_DROP_METHOD_open "https")
  set(CTEST_DROP_SITE_open "open.cdash.org")
  set(CTEST_DROP_LOCATION_open "/submit.php?project=SMTK")
  set(CTEST_DROP_SITE_CDASH_open TRUE)

endif ()
