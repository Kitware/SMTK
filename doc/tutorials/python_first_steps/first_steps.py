#!/usr/bin/env python

from smtk import model
from smtk import io
from smtk import attribute
import smtk
import sys
# print('This test has been disabled until tutorials are updated.\n')
# sys.exit(125)

modelFileName = sys.argv[-1]

# I. First, demonstrate basic setup of importing module
#    and creating managers.
# ++ 1 ++

ares = smtk.attribute.Resource.create()
mmgr = smtk.model.Resource.create()

# -- 1 --

# II. Basic attribute definitions
# ++ 2 ++
bcDef = ares.createDefinition('boundary condition')
icDef = ares.createDefinition('initial condition')
matDef = ares.createDefinition('material properties')

# Our material is defined by a viscosity
viscosity = smtk.attribute.DoubleItemDefinition.New('viscosity')
matDef.addItemDefinition(viscosity)
# -- 2 --

# III. Attribute definitions may inherit other definitions:
# ++ 3 ++
# We have one initial condition for the interior of the domain:
fluidICDef = ares.createDefinition('fluid ic', icDef)

# The specific types of BCDefs our example must define:
# At the outlet, pressure must be given.
# At the inlet, fluid velocity and temperature must be given.
# On the wall, heat flux must be specified.
outletBCDef = ares.createDefinition('outlet bc', bcDef)
inletBCDef = ares.createDefinition('inlet bc', bcDef)
wallBCDef = ares.createDefinition('wall bc', bcDef)

# Each ICDef/BCDef holds a different type of value:
temperature = smtk.attribute.DoubleItemDefinition.New('temperature')
pressure = smtk.attribute.DoubleItemDefinition.New('pressure')
velocity = smtk.attribute.DoubleItemDefinition.New('velocity')
heatflux = smtk.attribute.DoubleItemDefinition.New('heatflux')

# Add the values to the appropriate BCDef:
fluidICDef.addItemDefinition(temperature)
fluidICDef.addItemDefinition(velocity)

outletBCDef.addItemDefinition(pressure)

inletBCDef.addItemDefinition(velocity)
inletBCDef.addItemDefinition(temperature)

wallBCDef.addItemDefinition(heatflux)
# -- 3 --

# IV. Put limits on some of the values
# ++ 4 ++
viscosity.setMinRange(0, True)
viscosity.setUnits('Pa * s')  # or Poise [P]

temperature.setMinRange(-273.15, True)
temperature.setUnits('deg C')

pressure.setMinRange(0, True)
pressure.setUnits('Pa')

heatflux.setUnits('W / m^2 / K')

velocity.setNumberOfRequiredValues(2)
velocity.setUnits('m / s')
velocity.setDefaultValue(0.)
# -- 4 --

# V. Create attributes from the definitions
# ++ 5 ++
fluidIC = ares.createAttribute('fluidIC', fluidICDef)
wallBC = ares.createAttribute('wallBC', wallBCDef)
inletBC = ares.createAttribute('inletBC', inletBCDef)
outletBC = ares.createAttribute('outletBC', outletBCDef)
matProp = ares.createAttribute('fluid', matDef)
# -- 5 --

# VI. Now tie these to the model
# ++ 6 ++
# Read in an SMTK-native B-Rep model:
# TODO: Replace with resource.readModel()
jsonFile = open(modelFileName, 'r')
json = jsonFile.read()
smtk.model.SessionIOJSON.loadModelRecords(json, mmgr)

# Now find groups corresponding to IC/BCs:
models = mmgr.findEntitiesByProperty('name', 'Test Model')
model = smtk.model.Model(models[0])
groups = model.groups()
if groups and len(groups):
    wallGroup = next((g for g in groups if g.name() == 'wall'))
    inletGroup = next((g for g in groups if g.name() == 'inlet'))
    outletGroup = next((g for g in groups if g.name() == 'outlet'))
    fluidGroup = next((g for g in groups if g.name() == 'fluid'))

    fluidIC.associateEntity(fluidGroup)
    outletBC.associateEntity(outletGroup)
    inletBC.associateEntity(inletGroup)
    wallBC.associateEntity(wallGroup)
# -- 6 --

# VII. Loop over conditions of interest and create
#      an input deck for each.
# ++ 7 ++
# FIXME: Actually put this inside a loop that exports input decks.
matProp.findDouble('viscosity').setValue(1.002e-3)  # [Pa * s]

fluidIC.findDouble('temperature').setValue(25)  # [C]
fluidIC.findDouble('velocity').setValue(0, 0.)  # [m / s]
fluidIC.findDouble('velocity').setValue(1, 0.)  # [m / s]

outletBC.findDouble('pressure').setValue(101300)  # [Pa]

inletBC.findDouble('velocity').setValue(0, 0.25)  # [m / s]
inletBC.findDouble('velocity').setValue(1, 0.00)  # [m / s]
inletBC.findDouble('temperature').setValue(50)  # [C]

wallBC.findDouble('heatflux').setValue(1.0)  # [W / m^2 / K]
# -- 7 --

# ++ xxx ++
# -- xxx --
