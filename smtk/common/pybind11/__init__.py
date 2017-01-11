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

from smtkPybindCommon import *

import datetime
def DateTimeZonePair_to_python(self, utc=False):
  '''Returns python datetime instance, or None if not set.

  This is a bare datetime instance with NO tzinfo.
  It returns local time by default.
  If the utc arugment is set, it will internally convert
    to UTC and return that as the contents of the datetime instance.
  If the SMTK DateTime is not set, returns None.
  If utc is True but TimeZone not set, returns None.
  '''
  smtk_dt = self.dateTime()
  smtk_tz = self.timeZone()

  if not smtk_dt.isSet():
    return None

  if utc and not smtk_tz.isSet():
    return None

  # (else)
  if (utc):
    smtk_tz = TimeZone()  # unset TimeZone instance
  smtk_comps = smtk_dt.components(smtk_tz)

  python_comps = list(smtk_comps)
  python_comps[-1] *= 1000  # convert smtk/msec to python/usec
  dt = datetime.datetime(*python_comps)
  return dt

DateTimeZonePair.to_python_datetime = DateTimeZonePair_to_python
