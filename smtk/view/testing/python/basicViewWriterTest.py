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
Manual port of SMTK/smtk/attribute/Testing/basicViewWriterTest.cxx
For verifying python-shiboken wrappers

Requires SMTKCorePython.so to be in module path
"""

import smtk

def addItemDefinition( ato, data_type, name):
  def_ = data_type.New(name)
  if def_ is None:
    print "could not create"
    return None
  idef = data_type.ToItemDefinition(def_)
  if idef is None:
    print "could not convert"
    return None
  if not ato.addItemDefinition(idef):
    print "could not add"
    return None
  return def_

def addSubView( ato, data_type, name ):
  sec = data_type.New(name)
  if sec is None:
    print "could not create subview"
    return None
  print "converting to Base View"
  isec = data_type.ToBase(sec)
  if isec is None:
    print "could not convert"
    return None
  if not ato.addSubView(isec):
    print "could not add"
    return None
  return sec

if __name__ == '__main__':
    import sys

    status = 0

    system = smtk.attribute.System()
    print "System Created"

    #Lets create some attribute Definitions
    funcDef = system.createDefinition("PolyLinearFunction")
    if funcDef is None:
      print "could not create funcDef"
      sys.exit( -1 )
    materialDef = system.createDefinition("Material")
    if materialDef is None:
      print "could not create materialDef"
      sys.exit( -1 )
    materialDef.setAssociationMask(smtk.model.VOLUME) #belongs on 3D domains
    boundaryConditionsDef = system.createDefinition("BoundaryCondition")
    boundaryConditionsDef.setAssociationMask(smtk.model.FACE); #belongs on 3D boundaries
    specifiedHeadDef = system.createDefinition("SpecifiedHead", "BoundaryCondition")
    if specifiedHeadDef is None:
      print "Could not create SpecifiedHead"
      sys.exit( -1 )
    specifiedFluxDef = system.createDefinition("SpecifiedFlux", "BoundaryCondition")
    if specifiedHeadDef is None:
      print "could not create specifiedHeadDef"
      sys.exit( -1 )
    injectionWellDef = system.createDefinition("InjectionWell", "BoundaryCondition")
    if injectionWellDef is None:
      print "could not create injectionWellDef"
      sys.exit( -1 )
    timeParamDef = system.createDefinition("TimeParameters")
    if timeParamDef is None:
      print "could not create timeParamDef"
      sys.exit( -1 )
    globalsDef = system.createDefinition("GlobalParameters")
    if globalsDef is None:
      print "could not create globalsDef"
      sys.exit( -1 )

    #Lets add some analyses
    analysis = list()
    analysis.append("Flow")
    analysis.append("General")
    analysis.append("Time")
    system.defineAnalysis("CFD Flow", analysis)
    del analysis[:]

    analysis.append("Flow")
    analysis.append("Heat")
    analysis.append("General")
    analysis.append("Time")
    system.defineAnalysis("CFD Flow with Heat Transfer", analysis)
    del analysis[:]

    analysis.append("Constituent")
    analysis.append("General")
    analysis.append("Time")
    system.defineAnalysis("Constituent Transport", analysis)
    del analysis[:]

    #Lets complete the definition for some boundary conditions
    ddef = addItemDefinition(specifiedHeadDef, smtk.attribute.DoubleItemDefinition, "Value")
    ddef.setExpressionDefinition(funcDef)
    ddef = addItemDefinition(specifiedFluxDef, smtk.attribute.DoubleItemDefinition, "Value")
    ddef.setExpressionDefinition(funcDef)

    gdef = addItemDefinition(timeParamDef, smtk.attribute.GroupItemDefinition, "StartTime")
    gdef.setCommonSubGroupLabel("Start Time")
    ddef = addItemDefinition(gdef, smtk.attribute.DoubleItemDefinition, "Value")
    ddef.addCategory("Time")
    ddef.setDefaultValue(0)
    ddef.setMinRange(0, True)
    idef = addItemDefinition(gdef, smtk.attribute.IntItemDefinition, "Units")
    idef.addDiscreteValue(0, "Seconds")
    idef.addDiscreteValue(1, "Minutes")
    idef.addDiscreteValue(2, "Hours")
    idef.addDiscreteValue(3, "Days")
    idef.setDefaultDiscreteIndex(0)
    idef.addCategory("Time")
    gdef = addItemDefinition(timeParamDef, smtk.attribute.GroupItemDefinition, "EndTime")
    gdef.setCommonSubGroupLabel("End Time")
    ddef = addItemDefinition(gdef, smtk.attribute.DoubleItemDefinition, "Value")
    ddef.addCategory("Time")
    ddef.setDefaultValue(162)
    ddef.setMinRange(0, True)
    idef = addItemDefinition(gdef, smtk.attribute.IntItemDefinition, "Units")
    idef.addDiscreteValue(0, "Seconds")
    idef.addDiscreteValue(1, "Minutes")
    idef.addDiscreteValue(2, "Hours")
    idef.addDiscreteValue(3, "Days")
    idef.setDefaultDiscreteIndex(0)
    idef.addCategory("Time")

    ddef = addItemDefinition(globalsDef, smtk.attribute.DoubleItemDefinition, "Gravity")
    ddef.setDefaultValue(1.272024e08)
    ddef.setUnits("m/hr^2")
    ddef.setAdvanceLevel(1)
    ddef.setMinRange(0, False)

    ddef = addItemDefinition(globalsDef, smtk.attribute.DoubleItemDefinition, "WaterSpecificHeat")
    ddef.setDefaultValue(0.00116)
    ddef.setUnits("W hr/g-K")
    ddef.setAdvanceLevel(1)
    ddef.setMinRange(0, False)

    ddef = addItemDefinition(globalsDef, smtk.attribute.DoubleItemDefinition, "AirSpecificHeat")
    ddef.setDefaultValue(0.000278)
    ddef.setUnits("W hr/g-K")
    ddef.setAdvanceLevel(1)
    ddef.setMinRange(0, False)

    #Lets add some views

    root = system.rootView()
    root.setTitle("SimBuilder")
    expSec = addSubView(root, smtk.view.SimpleExpression,"Functions")
    expSec.setDefinition(funcDef)
    attSec = addSubView( root, smtk.view.Attribute, "Materials")
    attSec.addDefinition(materialDef)
    attSec.setModelEntityMask(smtk.model.VOLUME)
    attSec.setOkToCreateModelEntities(True)
    modSec = addSubView(root, smtk.view.ModelEntity, "Domains")
    modSec.setModelEntityMask(smtk.model.VOLUME) # Look at 3D domains only
    modSec.setDefinition(materialDef) # use tabled view focusing on Material Attributes
    attSec = addSubView(root, smtk.view.Attribute, "BoundaryConditions")
    attSec.addDefinition(boundaryConditionsDef)
    modSec = addSubView(root, smtk.view.ModelEntity, "Boundary View")
    modSec.setModelEntityMask(smtk.model.FACE) # Look at 3d boundary entities only

    system.updateCategories()
    att = system.createAttribute("TimeInformation", timeParamDef)
    iSec = addSubView(root, smtk.view.Instanced, "Time Parameters")
    iSec.addInstance(att)
    att = system.createAttribute("Globals", globalsDef)
    iSec = addSubView(root, smtk.view.Instanced, "Global Parameters")
    iSec.addInstance(att)
#    writer = smtk.attribute.XmlV2StringWriter(system)
#    result = writer.convertToString()
#    print result
    #test(result)
    #doc = smtk.pugi.xml_document
    #doc.load(test)
    #system1 = smtk.attribute.System()
    #reader = smtk.attribute.XmlDocV2Parser(system1)
    #reader.process(doc)
    #readErrors = reader.errorStatus()
    #if readErrors is not "":
      #print "READ ERRORS ENCOUNTERED!!!"
      #print readErrors
    #writer1 = smtk.attribute.XmlV2StringWriter(system1)
    #print << "System 1:"
    #result = writer1.convertToString()
    #print result

    del system
    print 'System destroyed'

    sys.exit(status)
