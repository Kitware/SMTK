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

import os

import smtk
import smtk.common
import smtk.resource
import smtk.testing

import sys
import uuid


class MyComponent(smtk.resource.Component):

    def __init__(self, resource):
        smtk.resource.Component.__init__(self)
        self._id = uuid.uuid1()
        self.resource = resource

    def __eq__(self, other):
        return self.id() == other.id()

    def __hash__(self):
        return hash(self.id())

    def resource(self):
        return self.resource

    def id(self):
        return self._id

    def setId(self, newId):
        self._id = newId
        return True

    def name(self):
        return type(self).__name__ + ' (' + str(self.id()) + ')'


class MyResource(smtk.resource.Resource):

    def __init__(self):
        smtk.resource.Resource.__init__(self)
        self.components = {}

    def newComponent(self):
        component = MyComponent(self)
        component.setId(uuid.uuid1())
        self.components[component.id()] = component
        return component

    def find(self, compId):
        return self.components[compId]

    def queryOperation(self, queryString):
        def doNothing(component):
            return True
        return doNothing

    def visit(self, visitor):
        map(visitor, self.components)

    def name(self):
        return type(self).__name__ + ' (' + str(self.id()) + ')'


def test_python_resource():

    myResource = MyResource()

    myComponent = myResource.newComponent()
    alsoMyComponent = myResource.find(myComponent.id())

    assert (myComponent == alsoMyComponent)


if __name__ == '__main__':
    smtk.testing.process_arguments()
    test_python_resource()
