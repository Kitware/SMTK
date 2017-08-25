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

from _smtkPybindIO import *
import _smtkPybindIO as _io

_tmp = [x for x in dir(_io) if not x.startswith('_')]
if "@SMTK_ENABLE_VTK_SUPPORT@" is "ON":
    _tmp.append('vtk')

__all__ = (_tmp)

del _tmp


""" Error/Warning/Debug/InfoMessage:

These are the equivalent to SMTK's C++ smtkError/Warning/Debug/InfoMacro. They
are placed at smtk.* scope to parallel the C++ macro's lack of scope, but are
defined in the IO module because they use smtk.Logger.
"""

from inspect import currentframe, getframeinfo


def _message(severity, logger, message):
    frameinfo = getframeinfo(currentframe().f_back)
    logger.addRecord(severity, message, frameinfo.filename, frameinfo.lineno)


def _infoMessage(logger, message):
    logger.addRecord(Logger.INFO, message)

import functools
import smtk

smtk.ErrorMessage = functools.partial(_message, Logger.ERROR)
smtk.WarningMessage = functools.partial(_message, Logger.WARNING)
smtk.DebugMessage = functools.partial(_message, Logger.DEBUG)
smtk.InfoMessage = _infoMessage

# For backward compatibility with simulation workflow export
# scripts written for CMB V4.


def _addError(logger, message):
    smtk.ErrorMessage(logger, message)


def _addWarning(logger, message):
    smtk.WarningMessage(logger, message)


def _addDebug(logger, message):
    smtk.DebugMessage(logger, message)

Logger.addError = _addError
Logger.addWarning = _addWarning
Logger.addDebug = _addDebug

del _addError
del _addWarning
del _addDebug

del _message
del _infoMessage
