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
# This Python script is part of the SMTK Cursor Tutorial.
# It should be extracted automatically from the tutorial, but isn't yet.
import sys
airfoilFile = sys.argv[1] if len(sys.argv) > 2 else 'airFoilSolidModel.json'

# --- start of tutorial ---
import smtk
from uuid import *

sm = smtk.model.Manager.create()
airfoil     = sm.addGroup(smtk.model.MODEL_DOMAIN   | smtk.model.DIMENSION_3, 'airfoil')
environment = sm.addGroup(smtk.model.MODEL_DOMAIN   | smtk.model.DIMENSION_3, 'environment')
inlet       = sm.addGroup(smtk.model.MODEL_BOUNDARY | smtk.model.DIMENSION_2, 'inlet')
skin        = sm.addGroup(smtk.model.MODEL_BOUNDARY | smtk.model.DIMENSION_2, 'skin')
outlet      = sm.addGroup(smtk.model.MODEL_BOUNDARY | smtk.model.DIMENSION_2, 'outlet')

system = smtk.attribute.System()
vdef = system.createDefinition('velocity')
for coord in ['x', 'y', 'z']:
   vi = smtk.attribute.DoubleItemDefinition.New('v' + coord)
   vi.setDefaultValue(-1.2 if coord == 'x' else 0.0) # in [m/s]
   vdef.addItemDefinition(vi);

velocity = system.createAttribute('velocity', vdef)
inlet.associateAttribute(velocity.id())

print sm.entitiesOfDimension(2)
print sm.entitiesOfDimension(3)

try:
  # If the file isn't present, just skip loading geometry.
  jsonData = file(airFoilFile, 'r').read()
  ok = smtk.io.ImportJSON.intoModelManager(jsonData, sm)
except:
  pass

# We could select faces graphically,
# but for this example we just "know" them:
sm.addToGroup(outlet.entity(), [
  UUID('12c853a5-f4d9-455d-bd61-8a73d6fe9de2'),
  UUID('49a71490-5873-45a9-ab9c-e6f89f099d46'),
  UUID('60fa3273-68cb-4016-94a5-c5ae632d45b8'),
  UUID('892382af-532f-4dc4-bbbb-56230723503f'),
  UUID('cb78cf98-b2e3-4f37-a2bf-eb823a533e5f'),
  ])

sm.addToGroup(inlet.entity(), [
  UUID('26c312cb-f3dd-47b0-8a84-d0b186035498'),
  ])

sm.addToGroup(skin.entity(), [
  UUID('7bc6818a-9028-4a43-b870-886d7a66793e'),
  UUID('e71d4246-456d-429f-815b-074e1749f888'),
  UUID('a1e7677f-40af-403b-979b-8109c730d670'),
  UUID('d99a4ae1-9a60-4da7-af26-0138f4b84685'),
  UUID('0bcc7039-19a5-4727-a449-7ab0af4e8ad6'),
  UUID('17169461-866b-4d4f-b3b6-9ab47b271184'),
  UUID('f1dcf500-571d-43ba-ba6a-581bfbb6953a'),
  UUID('e98bb9e1-583d-4e3d-81d5-d9b73af1b6de'),
  UUID('df663255-bd6e-40db-bd65-96be89328a61'),
  UUID('bcf0ed16-3f05-4043-9313-a6a3617121fb'),
  ])

json = smtk.io.ExportJSON.fromModelManager(sm)
