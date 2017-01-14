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
from smtk import common, attribute

if __name__ == '__main__':
  import datetime
  import os
  import sys

  errcode = 0

  # Create attribute definition with 2 datetime item definiitons
  system = smtk.attribute.System()
  attdef = system.createDefinition('test-att')

  # First DateTimeItemDefinition
  itemdef1 = smtk.attribute.DateTimeItemDefinition.New('dt1')
  attdef.addItemDefinition(itemdef1)

  # Second DateTimeItemDefintion, includes default
  itemdef2 = smtk.attribute.DateTimeItemDefinition.New('dt2')
  itemdef2.setDisplayFormat("dd-MMM-yyyy  h:mm:ss.zzz AP")
  itemdef2.setUseTimeZone(False)
  itemdef2.setEnableCalendarPopup(False)

  dtz = smtk.common.DateTimeZonePair()
  tz = smtk.common.TimeZone()
  tz.setRegion('Europe/Zurich')
  dtz.setTimeZone(tz)
  dt = smtk.common.DateTime()
  dt.setComponents(tz, 1991, 8, 6, 12, 0, 0, 0)
  dtz.setDateTime(dt)
  itemdef2.setDefaultValue(dtz)
  attdef.addItemDefinition(itemdef2)

  # Create attribute and check item values
  att = system.createAttribute(attdef)
  # from smtk import io
  # writer = smtk.io.AttributeWriter()
  # logger = smtk.io.Logger()
  # filename = 'datetime-example.sbi'
  # if writer.write(system, filename, logger):
  #   print 'Error writing file', filename
  # else:
  #   print 'Wrote', filename

  item1 = att.findDateTime('dt1')
  if not item1:
    print 'First DateTimeItem not found'
    errcode = -1
  elif item1.isSet(0):
    print 'First DateTimeItem should NOT be set'
    errcode = -1

  item2 = att.findDateTime('dt2')
  if not item2:
    print 'Second DateTimeItem not found'
    errcode = -1
  else:
    if not item2.isSet(0):
      print 'Second DateTimeItem NOT set'
      errcode = -1

    dtz2 = item2.value(0)
    py_dt = dtz2.to_python_datetime()
    print 'py_dt', py_dt

    dt_string = py_dt.strftime('%Y-%m-%d %H:%M')
    expected = '1991-08-06 12:00'
    if dt_string != expected:
      print 'Wrong local datetime, should be %s not %s' % \
        (expected, dt_string)
      errcode = -1

  print 'errcode', errcode
  sys.exit(errcode)
