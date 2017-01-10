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

import smtk
from smtk import attribute

if __name__ == '__main__':
  import datetime
  import os
  import sys

  errcode = 0

  # SMTK DateTime instance
  smtk_dt = smtk.common.DateTime()

  # SMTK TimeZone instance
  smtk_zone = smtk.common.TimeZone()
  smtk_zone.setPosixString('EST-5')

  # SMTK DateTimeZonePair
  sp = smtk.common.DateTimeZonePair()
  sp.setDateTime(smtk_dt)
  sp.setTimeZone(smtk_zone)

  # Convert to python datetime - should be "None"
  dt1 = sp.to_python_datetime()
  if not dt1 is None:
    print 'Empty DateTimeZonePair should convert to None'
    errcode = -1

  # Set datetime with time zone
  smtk_dt.setComponents(smtk_zone, 2016, 11, 16, 16, 46, 22, 33)
  sp.setDateTime(smtk_dt)

  # Check python datetime
  dt2 = sp.to_python_datetime()
  dt_string2 = dt2.strftime('%Y-%m-%d %H:%M:%S .%f')
  expected2 = '2016-11-16 16:46:22 .033000'
  if dt_string2 != expected2:
    print 'Wrong local datetime, should be %s not %s' % \
      (expected2, dt_string2)
    errcode = -1

  # Check python datetime with utc option
  dt3 = sp.to_python_datetime(True)
  dt_string3 = dt3.strftime('%Y-%m-%d %H:%M:%S .%f')
  expected3 = '2016-11-16 21:46:22 .033000'
  if dt_string3 != expected3:
    print 'Wrong UTC datetime, should be %s not %s' % \
      (expected3, dt_string3)
    errcode = -1

  sys.exit(errcode)
