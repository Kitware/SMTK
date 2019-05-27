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

from ._smtkPybindOperation import *

"""
Operation parameters are simply instances of smtk.attribute.Attribute. We wrap
smtk.operation.Operation.parameters() in a python <_create_api> that constructs
a more user-friendly API to the python interface for operation parameters.
"""


def _create_api(parameters):
    for i in range(parameters.numberOfItems()):
        item = parameters.item(i)
        name = item.name()
        import re
        nameList = re.split(r"[^a-zA-Z0-9]", item.name().title())
        if hasattr(item, 'value'):
            set_attr = 'set' + ''.join(nameList)
            get_attr = nameList[0].lower()
            if len(nameList) > 1:
                get_attr += ''.join(nameList[1:])

            def setValue(self, *argv):
                index = 0
                if len(argv) > 1:
                    index = argv[0]
                return self.find(name).setValue(index, argv[-1])
            setattr(parameters, set_attr, setValue)

            def value(self, index=0):
                return self.find(name).value(index)
            setattr(parameters, get_attr, value)
        if item.isOptional():
            enable_attr = 'enable' + ''.join(nameList)
            enabled_attr = nameList[0].lower()
            if len(nameList) > 1:
                enabled_attr += ''.join(nameList[1:])
            enabled_attr += 'Enabled'

            def enable(self, choice):
                return self.find(name).setIsEnabled(choice)
            setattr(parameters, enable_attr, enable)

            def isEnabled(self):
                return self.find(name).isEnabled()
            setattr(parameters, enabled_attr, isEnabled)

        if hasattr(item, 'isDiscrete'):
            setindex_attr = 'set' + ''.join(nameList) + 'Index'

            def setDiscreteIndex(self, index, value):
                return self.find(name).setDiscreteIndex(index, value)
            setattr(parameters, setindex_attr, setDiscreteIndex)
            getindex_attr = nameList[0].lower()
            if len(nameList) > 1:
                getindex_attr += ''.join(nameList[1:])
            getindex_attr += "Index"

            def discreteIndex(self, index):
                return self.find(name).discreteIndex(index)
            setattr(parameters, getindex_attr, discreteIndex)


def _params(self):
    params = self._parameters()
    _create_api(params)
    return params

setattr(Operation, 'parameters', _params)

del _params
