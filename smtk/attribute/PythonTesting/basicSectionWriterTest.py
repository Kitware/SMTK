"""
Manual port of SMTK/smtk/attribute/Testing/basicSectionWriterTest.cxx
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

if __name__ == '__main__':
    import sys
    
    status = 0
    
    manager = smtk.attribute.Manager()
    print "Manager Created"
    
    #Lets create some attribute Definitions
    funcDef = manager.createDefinition("PolyLinearFunction")
    if funcDef is None:
      print "could not create funcDef"
      sys.exit( -1 )
    materialDef = manager.createDefinition("Material")
    if materialDef is None:
      print "could not create materialDef"
      sys.exit( -1 )
    materialDef.setAssociationMask(0x40) #belongs on domains
    boundaryConditionsDef = manager.createDefinition("BoundaryCondition")
    boundaryConditionsDef.setAssociationMask(0x20); #belongs on boundaries
    specifiedHeadDef = manager.createDefinition("SpecifiedHead", "BoundaryCondition")
    if specifiedHeadDef is None:
      print "Could not create SpecifiedHead"
      sys.exit( -1 )
    specifiedFluxDef = manager.createDefinition("SpecifiedFlux", "BoundaryCondition")
    if specifiedHeadDef is None:
      print "could not create specifiedHeadDef"
      sys.exit( -1 )
    injectionWellDef = manager.createDefinition("InjectionWell", "BoundaryCondition")
    if injectionWellDef is None:
      print "could not create injectionWellDef"
      sys.exit( -1 )
    timeParamDef = manager.createDefinition("TimeParameters")
    if timeParamDef is None:
      print "could not create timeParamDef"
      sys.exit( -1 )
    globalsDef = manager.createDefinition("GlobalParameters")
    if globalsDef is None:
      print "could not create globalsDef"
      sys.exit( -1 )
    
    #Lets add some analyses
    analysis = list()
    analysis.append("Flow")
    analysis.append("General")
    analysis.append("Time")
    manager.defineAnalysis("CFD Flow", analysis)
    del analysis[:]
    
    analysis.append("Flow")
    analysis.append("Heat")
    analysis.append("General")
    analysis.append("Time")
    manager.defineAnalysis("CFD Flow with Heat Transfer", analysis)
    del analysis[:]
    
    analysis.append("Constituent")
    analysis.append("General")
    analysis.append("Time")
    manager.defineAnalysis("Constituent Transport", analysis)
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
    ddef.setMinRange(0, true)
    idef = addItemDefinition(gdef, smtk.attribute.IntItemDefinition, "Units")
    idef.addDiscreteValue(0, "Seconds")
    idef.addDiscreteValue(1, "Minutes")
    idef.addDiscreteValue(2, "Hours")
    idef.addDiscreteValue(3, "Days")
    idef.setDefaultDiscreteIndex(0)
    idef.addCategory("Time")
    gdef = addItemDefinition(timeParamDef, smtk.attribute.GroupItemDefinition, "EndTime")
    gdef.setCommonSubGroupLabel("End Time")
    ddef = gdef.addItemDefinition(gdef, smtk.attribute.DoubleItemDefinition, "Value")
    ddef.addCategory("Time")
    ddef.setDefaultValue(162)
    ddef.setMinRange(0, true)
    idef = addItemDefinition(gdef, smtk.attribute.IntItemDefinition, "Units")
    idef.addDiscreteValue(0, "Seconds")
    idef.addDiscreteValue(1, "Minutes")
    idef.addDiscreteValue(2, "Hours")
    idef.addDiscreteValue(3, "Days")
    idef.setDefaultDiscreteIndex(0)
    idef.addCategory("Time")
       
    globalsDef.addItemDefinition(ddef, smtk.attribute.DoubleItemDefinition, "Gravity")
    ddef.setDefaultValue(1.272024e08)
    ddef.setUnits("m/hr^2")
    ddef.setAdvanceLevel(1)
    ddef.setMinRange(0, false)

    ddef = addItemDefinition(globalsDef, smtk.attribute.DoubleItemDefinition, "WaterSpecificHeat")
    ddef.setDefaultValue(0.00116)
    ddef.setUnits("W hr/g-K")
    ddef.setAdvanceLevel(1)
    ddef.setMinRange(0, false)

    ddef = addItemDefinition(globalsDef, smtk.attribute.DoubleItemDefinition, "AirSpecificHeat")
    ddef.setDefaultValue(0.000278)
    ddef.setUnits("W hr/g-K")
    ddef.setAdvanceLevel(1)
    ddef.setMinRange(0, false)

    #Lets add some sections
  
    root = manager.rootSection()
    root.setTitle("SimBuilder")
    expSec = root.addSubsection(smtk.attribute.SimpleExpressionSection,"Functions")
    expSec.setDefinition(funcDef)
    attSec = root.addSubsection(smtk.attribute.AttributeSection, "Materials")
    attSec.addDefinition(materialDef)
    attSec.setModelEntityMask(0x40)
    attSec.setOkToCreateModelEntities(true)
    modSec = addSubsection(smtk.attribute.ModelEntitySection, "Domains")
    modSec.setModelEntityMask(0x40) # Look at domains only
    modSec.setDefinition(materialDef) # use tabled view focusing on Material Attributes
    attSec = root.addSubsection(smtk.attribute.AttributeSection, "BoundaryConditions")
    attSec.addDefinition(boundaryConditionsDef)
    modSec = root.addSubsection(smtk.attribute.ModelEntitySection, "Boundary View")
    modSec.setModelEntityMask(0x20) # Look at boundary entities only

    manager.updateCategories()
    att = manager.createAttribute("TimeInfomation", timeParamDef)
    iSec = root.addSubsection(smtk.attribute.InstancedSection, "Time Parameters")
    iSec.addInstance(att)
    att = manager.createAttribute("Globals", globalsDef)
    iSec = root.addSubsection(smtk.attribute.InstancedSection, "Global Parameters")
    iSec.addInstance(att)
    writer = smtk.attribute.XmlV1StringWriter(manager)
    result = writer.convertToString()
    print result
    #test(result)
    #doc = smtk.pugi.xml_document
    #doc.load(test)
    #manager1 = smtk.attribute.Manager()
    #reader = smtk.attribute.XmlDocV1Parser(manager1)
    #reader.process(doc)
    #readErrors = reader.errorStatus()
    #if readErrors is not "":
      #print "READ ERRORS ENCOUNTERED!!!"
      #print readErrors
    #writer1 = smtk.attribute.XmlV1StringWriter(manager1)
    #print << "Manager 1:"
    #result = writer1.convertToString()
    #print result

    print
    input = raw_input('Hit <Enter> to delete Managar and exit')
    del manager
    print 'Manager destroyed'

    sys.exit(status)