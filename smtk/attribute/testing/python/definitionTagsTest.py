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
from smtk import attribute
from smtk import io

import smtk.testing


class TestDefinitionTags(smtk.testing.TestCase):

    def setUp(self):
        testInput = (
            "<?xml version=\"1.0\" encoding=\"utf-8\" ?>                                   "
            "<SMTK_AttributeResource Version=\"3\">                                        "
            "  <Definitions>                                                               "
            "    <AttDef Type=\"att1\" BaseType=\"\">                                      "
            "      <Tags>                                                                  "
            "        <Tag Name=\"My Tag\" />                                               "
            "        <Tag Name=\"My Tag with Values\">value1,value2,value3</Tag>           "
            "      </Tags>                                                                 "
            "      <ItemDefinitions>                                                       "
            "	  <String Name=\"normalString\" Extensible=\"0\"                           "
            "               NumberOfRequiredValues=\"1\">                                  "
            "         <DefaultValue>normal</DefaultValue>                                  "
            "	  </String>                                                                "
            "      </ItemDefinitions>                                                      "
            "    </AttDef>                                                                 "
            "  </Definitions>                                                              "
            "  <Attributes>                                                                "
            "    <Att Name=\"att\" Type=\"att1\"/>                                         "
            "  </Attributes>                                                               "
            "</SMTK_AttributeResource>                                                     ")

        self.resource = smtk.attribute.Resource.create()

        logger = smtk.io.Logger()
        reader = smtk.io.AttributeReader()
        status = reader.readContents(self.resource, testInput, logger)
        self.assertFalse(status, 'Could not read input')

    def testDefinitionTags(self):
        atts = self.resource.attributes()
        self.assertEqual(len(atts), 1, 'Incorrect number of attributes')

        att = atts[0]
        defn = att.definition()

        self.assertEqual(len(defn.tags()), 2, 'Incorrect number of tags')

        tag = defn.tag('My Tag')
        self.assertFalse(tag == None, 'Could not access tag')

        tag = defn.tag('My Tag with Values')
        self.assertFalse(tag == None, 'Could not access tag')
        self.assertEqual(len(tag.values()), 3,
                         'Incorrect number of tag values')
        value = ['value1', 'value2', 'value3']
        for v in value:
            self.assertTrue(tag.contains(v), 'Could not find tag value')

        tag = defn.tag('My Nonexistent Tag')
        self.assertTrue(tag == None, 'Found tag that does not exist')


if __name__ == '__main__':
    smtk.testing.process_arguments()
    smtk.testing.main()
