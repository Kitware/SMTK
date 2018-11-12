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

""" export_to_pyarc.py:

Export an RGG model as PyARC geometry.

"""
import sys

import export_to_pyarc_xml

import os
import PyArc
import PyArc.PyARCModel
import smtk
import smtk.io
import smtk.model
import smtk.bridge.rgg
from export_to_pyarcHelper import *


class export_to_pyarc(smtk.operation.Operation):

    def __init__(self):
        smtk.model.Operator.__init__(self)

    def name(self):
        return "export to pyarc"

    def operateInternal(self):

        # Access the PyARC SON file
        filename = self.specification().findFile('filename').value(0)

        # Access the associated model
        associatedEntities = self.associatedEntities()
        model = smtk.model.Model(next(iter(associatedEntities)))
        isHex = model.integerProperty("hex")[0]
        if not isHex:
            smtk.InfoMessage(
                self.log(), 'Cannot export a non hex nuclear core!')
            return self.createResult(smtk.operation.Operation.Outcome.FAILED)

        mgr = model.manager()
        # Awesome print message
        smtk.InfoMessage(self.log(), 'Executing export_to_pyarc')

        materials = model.stringProperty('materials')
        materialDescriptions = model.stringProperty('material_descriptions')
        # TODO: export materials

        tabNum = 0

        surfaces = dict()
        # (name, orientation, normal, pitch)
        surfaces["hexagon"] = set()
        # (name, axis, radius)
        surfaces["cylinder"] = set()
        # (name, z)
        surfaces["plane"] = set()

        content = "=arc\ngeometry{\n"
        materialsS, surfacesS, coreS, calculationsS = "", "", "", ""
        for group in model.groups():
            rggType = group.stringProperty('rggType')[0]
            if (rggType == '_rgg_core'):
                smtk.InfoMessage(self.log(), "processing core " + group.name())
                core = Core(group)
                coreS = core.exportCore(tabNum + 1, surfaces)
                break
        surfacesS += surfacesToString(tabNum + 1, surfaces)

        content += materialsS
        content += surfacesS
        content += coreS
        content += calculationsS
        content += "}"

        writeFile(filename, content)

        # Return with success
        result = self.createResult(smtk.operation.Operation.Outcome.SUCCEEDED)
        return result

    def createSpecification(self):
        spec = smtk.attribute.Resource.create()
        reader = smtk.io.AttributeReader()
        reader.readContents(
            spec, export_to_pyarc_xml.description, self.log())
        return spec
