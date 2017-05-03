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
import sys
import unittest
import smtk
if smtk.wrappingProtocol() == 'pybind11':
    import smtk.bridge.cgm
    import smtk.bridge.discrete
    import smtk.bridge.exodus
    import smtk.model
import smtk.testing
from smtk.simple import *


class MultiSessionDescriptivePhrase(unittest.TestCase):

    def setUp(self):
        self.session_files = {
            'cgm': ['model', '3d', 'solidmodel', 'occ', 'pyramid.brep'],
            'discrete': ['model', '2d', 'cmb', 'test2D.cmb'],
            'exodus': ['model', '3d', 'exodus', 'disk_out_ref.ex2']
        }
        for required in self.session_files.keys():
            if required not in smtk.model.Manager.sessionTypeNames():
                print 'ERROR: %s not available.' % required

        self.mgr = smtk.model.Manager.create()
        self.sessions = {}
        for (session_type, path) in self.session_files.items():
            sref = self.mgr.createSession(session_type)
            sref.setName(sref.flagSummary(0))
            SetActiveSession(sref)
            self.sessions[session_type] = {'session_ref': sref}

            filename = os.path.join(*([smtk.testing.DATA_DIR, ] + path))
            self.sessions[session_type]['entities'] = Read(filename)
            print 'Session ', session_type, ' entities ', self.sessions[session_type]['entities']
        self.mgr.assignDefaultNames()
        self.models = []
        baseline = (
            smtk.testing.DATA_DIR,
            'baseline',
            'smtk',
            'model',
            'integrationMultiSessionDescriptivePhrases.json')

        # Load the test results we expect
        import json
        jsdata = open(os.path.join(*baseline), 'r')
        self.correct = json.loads(jsdata.read())
        jsdata.close()

    def recursePhrase(self, phrase, depth):
        record = '{:1} ({:2})'.format(phrase.title(), phrase.subtitle())
        if phrase.title() == 'url':
            record = 'url ({:1})'.format(os.path.split(phrase.subtitle())[-1])

        if depth > 3:
            return [record, ]
        subs = [self.recursePhrase(x, depth + 1) for x in phrase.subphrases()]
        return [record, subs]

    def printPhrases(self, indent, top):
        for entry in top:
            if type(entry) == str or type(entry) == unicode:
                print indent + entry
            else:
                self.printPhrases(indent + '  ', entry)

    def testPhrase(self):
        # sessions = self.mgr.findEntitiesOfType(smtk.model.SESSION, True)
        phrase = smtk.model.EntityListPhrase.create().setup(
            [x[1]['session_ref'] for x in self.sessions.items()])
        spg = smtk.model.SimpleModelSubphrases.create()
        spg.setDirectLimit(-1)
        phrase.setDelegate(spg)
        # set active model
        self.models = self.mgr.findEntitiesOfType(
            int(smtk.model.MODEL_ENTITY), True)
        for model in self.models:
            #  set cgm session's model as active model
            if model.owningSession().name() == "cgm session":
                spg.setActiveModel(model)

        allPhrases = self.recursePhrase(phrase, 0)

        print "allPhrases: "
        print allPhrases
        print "\n self.correct:"
        print self.correct
        self.printPhrases('', allPhrases)
        self.assertEqual(allPhrases, self.correct, "Phrases mismatched.")

    def tearDown(self):
        # Release all references to sessions.
        # This should cause a clean shutdown of each session.
        self.sessions = {}


if __name__ == '__main__':
    smtk.testing.process_arguments()
    unittest.main()
