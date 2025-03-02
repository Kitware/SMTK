#=============================================================================
#
#  Copyright (c) Kitware, Inc.
#  All rights reserved.
#  See LICENSE.txt for details.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
#=============================================================================

set(unit_tests
  UnitTestRedirectOutput.cxx
  )

set(unit_test_libs)

smtk_unit_tests(LABELS "VTK"
                SOURCES ${unit_tests}
                LIBRARIES smtkIOVTK smtkCoreModelTesting
                VTK::CommonComputationalGeometry
                VTK::CommonCore
                VTK::CommonDataModel
                VTK::FiltersCore
                VTK::FiltersSources
                VTK::IOXML
                VTK::vtksys
                ${unit_test_libs}
                # TODO: Use Boost:: imported targets
                ${Boost_LIBRARIES})
