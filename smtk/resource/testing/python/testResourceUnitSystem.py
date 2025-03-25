# =============================================================================
#
#  Copyright (c) Kitware, Inc.
#  All rights reserved.
#  See LICENSE.txt for details.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
# =============================================================================

import smtk
import smtk.common
import smtk.resource
import smtk.model

import math

# Create a resource; by default it has no unit system.
rsrc = smtk.model.Resource.create()
# Test that conversion fails with no unit system.
result = rsrc.unitConversion("1 ft", "m")
if not math.isnan(result):
    raise RuntimeError('Unit conversion with no unit system should fail.')

rsrc.createDefaultUnitSystem()
result = rsrc.unitConversion("1 ft", "m")
if math.fabs(result - 0.3048) > 1e-7:
    raise RuntimeError('Unit conversion failed.')

result = rsrc.unitConversion("1 ft", "second")
if not math.isnan(result):
    raise RuntimeError(
        'Unit conversion between incompatible units should fail.')
