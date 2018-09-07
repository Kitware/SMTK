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

# For Windows builds, we must initialize our python interpreter with
# "Py_NoSiteFlag" enabled. We therefore import it here instead. Since this
# module is usually imported at python's initialization, this should have no
# effect when another python implementation imports smtk.
import site

__all__ = ('common', 'attribute', 'extension', 'model',
           'mesh', 'io', 'session', 'simulation')


def wrappingProtocol():
    return 'pybind11'
