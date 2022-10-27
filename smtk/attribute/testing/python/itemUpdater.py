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
import smtk.attribute
import smtk.common
import smtk.io

# Version 1 of a simple template, plus an instance of it:
attXmlV1 = """
<SMTK_AttributeResource Version="6" TemplateType="Example" TemplateVersion="1">
  <Definitions>
    <AttDef Type="Test" Version="1">
      <ItemDefinitions>
        <Int    Version="2" Name="ObjectiveIQ"/>
        <String Version="1" Name="SubjectiveIQ" Optional="true" IsEnabledByDefault="false" />
      </ItemDefinitions>
    </AttDef>
  </Definitions>
  <Attributes>
    <Att Type="Test" Name="Test1">
      <Items>
        <Int Name="ObjectiveIQ">37</Int>
        <String Name="SubjectiveIQ" IsEnabled="true">Dumb</String>
      </Items>
    </Att>
  </Attributes>
</SMTK_AttributeResource>
"""

# Version 5 of a simple template. We want to upgrade the instance
# in attXmlV1 to this version.
attXmlV5 = """
<SMTK_AttributeResource Version="6" TemplateType="Example" TemplateVersion="5">
  <Definitions>
    <AttDef Type="Test" Version="2">
      <ItemDefinitions>
        <String Version="3" Name="SubjectiveIQ" Optional="true" IsEnabledByDefault="true" />
        <!-- The "ObjectiveIQ" item switches from an integer item to a group in version 3 -->
        <Group  Version="3" Name="ObjectiveIQ" NumberOfRequiredGroups="1">
          <ItemDefinitions>
            <Int Version="1" Name="VerbalIQ"/>
            <Int Version="1" Name="LogicIQ"/>
          </ItemDefinitions>
        </Group>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>
"""


def updateSubjectiveIQ(itemIn, itemOut, log):
    """Update the "SubjectiveIQ" item by copying a modified version of its value."""
    if not itemIn or not itemOut:
        return False
    itemOut.setValue(itemIn.value() + 'er')
    return True


def updateObjectiveIQ(itemIn, itemOut, log):
    """Use the old "ObjectiveIQ" value as a baseline for the 2 values inside the group:"""
    if not itemIn or not itemOut:
        return False
    itemOut.find(0, 'VerbalIQ').setValue(itemIn.value() + 5)
    itemOut.find(0, 'LogicIQ').setValue(itemIn.value() - 5)
    return True


if __name__ == '__main__':
    # Create an attribute manager and register two item updaters:
    updateManager = smtk.attribute.UpdateManager.create()
    updateManager.registerItemUpdater(
        "Example", "Test", "SubjectiveIQ", 1, 2, 3, updateSubjectiveIQ)
    updateManager.registerItemUpdater(
        "Example", "Test", "ObjectiveIQ", 1, 2, 3, updateObjectiveIQ)

    # Read in an old attribute resource and create a new one,
    # then migrate items from the old resource's attribute to
    # a matching attribute in the new resource.
    logger = smtk.io.Logger.instance()
    reader = smtk.io.AttributeReader()
    rsrcv1 = smtk.attribute.Resource.create()
    rsrcv5 = smtk.attribute.Resource.create()
    if not reader.readContents(rsrcv1, attXmlV1, logger):
        if not reader.readContents(rsrcv5, attXmlV5, logger):
            att1 = rsrcv1.findAttribute('Test1')
            siq1 = att1.findString('SubjectiveIQ')
            oiq1 = att1.findInt('ObjectiveIQ')
            att2 = rsrcv5.createAttribute('Test')
            siq3 = att2.findString('SubjectiveIQ')
            oiq3 = att2.findGroup('ObjectiveIQ')
            updater = updateManager.findItemUpdater(
                rsrcv1.templateType(), att1.type(),
                att1.itemPath(siq1),
                siq3.definition().version(),
                siq1.definition().version())
            if not updater(siq1, siq3, logger) or \
               siq3.value() != 'Dumber':
                raise ('Could not update SubjectiveIQ')
            updater = updateManager.findItemUpdater(
                rsrcv1.templateType(), att1.type(),
                att1.itemPath(oiq1),
                oiq3.definition().version(),
                oiq1.definition().version())
            if not updater(oiq1, oiq3, logger):
                raise ('Could not update ObjectiveIQ')
            verbal = oiq3.find('VerbalIQ')
            logic = oiq3.find('LogicIQ')
            if verbal.value() != 42 or logic.value() != 32:
                raise ('Improper update of ObjectiveIQ')
        else:
            print('Could not parse attXmlV5', logger.convertToString())
    else:
        print('Could not parse attXmlV1', logger.convertToString())
    # For debugging:
    # writer = smtk.io.AttributeWriter()
    # writer.includeDefinitions(True)
    # writer.includeInstances(True)
    # xml = writer.writeContents(rsrcv5, logger)
    # print(xml)
