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

import os
import smtk
import smtk.environment


if __name__ == '__main__':

    print(smtk.environment.ResourceManager.instance())
    print(smtk.environment.OperationManager.instance())

    print('I know about %d resource types' %
          len(smtk.environment.ResourceManager.instance().metadata()))

    print('I know about %d operator types' %
          len(smtk.environment.OperationManager.instance().metadata()))
