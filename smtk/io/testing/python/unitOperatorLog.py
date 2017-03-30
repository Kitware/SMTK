#!/usr/bin/python
import sys
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
import smtk
if smtk.wrappingProtocol() == 'pybind11':
    import smtk.io
    import smtk.model
import smtk.testing
from smtk.simple import *


class LogOperatorNames(smtk.io.OperatorLog):
    """Inherit the pure virtual C++ OperatorLog class.

       This Python class implements the pure virtual methods
       so that some members of the Python instance record
       operator information (just the name and outcome at
       this point) which we compare to a known good list.
    """

    def __init__(self, mgr):
        # Invoke the C++ superclass constructor
        super(LogOperatorNames, self).__init__(mgr)

        # Keep a list of operations we have performed in the
        # "history" member and a stack of the currently-running
        # operators in the "active" member:
        self.history = []
        self.active = []

    def recordInvocation(self, evt, op):
        """Add the current operation to the active stack when invoked."""
        self.active.append([op.name()])
        # print 'Record Invocation of {nm}!'.format(nm=op.name())
        return 0

    def recordResult(self, evt, op, res):
        """Move the current operation from the active stack
           to the history upon completion."""
        outcome = smtk.model.OperatorOutcome(
            res.findInt('outcome').value(0))
        # print 'Record Result of {nm} as {stat}'.format(nm=op.name(), outcome)
        if len(self.active) and self.active[-1][0] == op.name():
            self.history.append(self.active[-1] + [outcome, ])
            self.active = self.active[:-1]
        else:
            self.history.append(
                ['***ERROR*** {nm}'.format(op.name()), outcome])
        return 0


class TestOperatorLog(smtk.testing.TestCase):

    def setUp(self):
        self.mgr = smtk.model.Manager.create()
        sref = self.mgr.createSession('cgm')
        if not sref.isValid():
            self.skipTest('CGM session unavailable')
        sref.assignDefaultName()
        SetActiveSession(sref)

    def testSimpleLogging(self):
        # Create an operator log.
        # It will immediately start logging all operations on the manager
        # (and only those on the specified model manager).
        recorder = LogOperatorNames(self.mgr)

        # Create a thick spherical shell and a sphere:
        sph = CreateSphere(radius=1.0, inner_radius=0.2,
                           center=[0.2, 0.2, 0.2])
        sph2 = CreateSphere(radius=0.5, center=[0.9, 0., 0.])

        # Note that su should have same UUID as sph2:
        su = Union([sph, sph2])

        # Create a cylinder:
        cyl = CreateCylinder(top_radius=1.0)

        # Now close the session, kill the manager and attempt to replay
        self.mgr.closeSession(GetActiveSession())
        self.assertEqual(
            GetActiveSession().name(),
            'invalid id {uid}'.format(uid=str(GetActiveSession().entity())),
            'Expected invalid session name after closing, got "{s}"'.format(
                s=GetActiveSession().name()))

        # See what we got in the log
        self.assertEqual(
            recorder.history,
            [
                ['create sphere', smtk.model.OperatorOutcome.OPERATION_SUCCEEDED],
                ['create sphere', smtk.model.OperatorOutcome.OPERATION_SUCCEEDED],
                ['union', smtk.model.OperatorOutcome.OPERATION_SUCCEEDED],
                ['create cylinder', smtk.model.OperatorOutcome.OPERATION_SUCCEEDED]
            ])


if __name__ == '__main__':
    smtk.testing.process_arguments()
    smtk.testing.main()
