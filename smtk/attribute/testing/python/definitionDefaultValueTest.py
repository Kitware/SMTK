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
"""
Verify that vector-valued defaults for value items are written and read
properly to and from XML files.
"""
import smtk
from smtk import attribute
from smtk import io

import sys
import os
err = 0

# Force a non-comma separator:
stringTest = ['Oh, no!', 'Yes, please.', 'Bleh.']
doubleTest = [3.,2.,1.]

# Create an attribute definition whose items have vector default values
asys = smtk.attribute.System()
foo = asys.createDefinition('foo')

bar = smtk.attribute.DoubleItemDefinition.New('bar')
bar.setNumberOfRequiredValues(3)
bar.setDefaultValue(doubleTest)
foo.addItemDefinition(bar)
baz = smtk.attribute.StringItemDefinition.New('baz')
baz.setNumberOfRequiredValues(3)
baz.setDefaultValue(stringTest)
foo.addItemDefinition(baz)

log = smtk.io.Logger()
wri = smtk.io.AttributeWriter()
res = wri.write(asys, sys.argv[1], log)
#xml = ''
#print wri.writeContents(asys, xml, log, False)

# Read in the generated XML
from xml.dom import minidom
doc = minidom.parse(sys.argv[1])
ddef = doc.getElementsByTagName('Double')[0].getElementsByTagName('DefaultValue')[0]
sdef = doc.getElementsByTagName('String')[0].getElementsByTagName('DefaultValue')[0]
if not ddef or 'Sep' in ddef.attributes.keys():
  print 'Invalid default value in DoubleItemDefinition: %s' % ddef.toxml()
  err |= 1
if not sdef or 'Sep' not in sdef.attributes.keys() or sdef.attributes['Sep'].value != ';':
  print 'Invalid default value in StringItemDefinition: %s' % sdef.toxml()
  err |= 2

asys = smtk.attribute.System()
rdr = smtk.io.AttributeReader()
rdr.read(asys, sys.argv[1], log)
att = asys.createAttribute('foobly', asys.findDefinition('foo'))
baritem = att.findDouble('bar')
xx = [baritem.value(i) for i in range(baritem.numberOfValues())]
if xx != doubleTest:
  print 'Unexpected double default value ', xx
  err |= 4
bazitem = att.findString('baz')
yy = [bazitem.value(i) for i in range(bazitem.numberOfValues())]
if yy != stringTest:
  print 'Unexpected string default value ', yy
  err |= 8

sys.exit(err)
