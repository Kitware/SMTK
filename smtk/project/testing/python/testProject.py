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
import smtk.project
import smtk.testing

import sys
import uuid


class MyProject(smtk.project.Project):

    def __init__(self):
        smtk.project.Project.__init__(self)

    def name(self):
        return type(self).__name__ + ' (' + str(self.id()) + ')'


def test_python_project():
    myProject = MyProject()


if __name__ == '__main__':
    smtk.testing.process_arguments()
    test_python_project()
