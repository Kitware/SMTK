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
"""
Test methods on descriptive phrases and subphrase generators.
"""

import os
import sys
import smtk
if smtk.wrappingProtocol() == 'pybind11':
    import smtk.io
    import smtk.model
import smtk.testing
from smtk.simple import *
from uuid import uuid4, UUID
import unittest

class TestModelPhrases(unittest.TestCase):

  def setUp(self):
    model_path = os.path.join(smtk.testing.DATA_DIR, 'model', '2d', 'smtk', 'pyramid.json')
    print 'Loading %s' % model_path

    status = 0
    self.mgr = smtk.model.Manager.create()
    session = self.mgr.createSession('native', smtk.model.SessionRef())
    SetActiveSession(session)
    json = None
    with open(model_path, 'r') as f:
      json = f.read()

    self.assertTrue(not json == None, 'Unable to load input file.')
    self.assertTrue(smtk.io.ImportJSON.intoModelManager(json, self.mgr), 'Unable to parse JSON input file.')

    self.mgr.assignDefaultNames()

    self.models = self.mgr.findEntitiesOfType(int(smtk.model.MODEL_ENTITY), True)
    print(type(self.models))
    print(self.models)
    # Assign imported models to current session so they have operators
    [smtk.model.Model(x).setSession(session) for x in self.models]

  def testPhrases(self):

    print 'Subphrases of {n} model(s)'.format(n=len(self.models))

    self.elist = smtk.model.EntityListPhrase.create().setup(self.models);
    self.spg = smtk.model.SimpleModelSubphrases.create();
    self.elist.setDelegate(self.spg);

    self.assertEqual(self.elist.title(), '1 volumetric model', 'Unexpected top-level title.')
    s1 = self.elist.subphrases()
    self.assertEqual(len(s1), 1, 'Unexpected phrases.')
    self.assertEqual(s1[0].title(), 'Model A', 'Unexpected model title.')
    self.assertEqual(s1[0].subtitle(), 'volumetric model', 'Unexpected model subtitle.')
    self.assertEqual(s1[0].relatedEntity(), self.models[0], 'Model phrase not related to model.')
    self.assertEqual(s1[0].relatedColor(), [0.0, 0.0, 0.0, -1.0], 'Unexpected model color.')
    self.assertEqual(s1[0].isTitleMutable(), True, 'Expected model name to be mutable.')
    self.assertEqual(s1[0].isSubtitleMutable(), False, 'Expected model subtitle to be immutable.')
    self.assertEqual(s1[0].isRelatedColorMutable(), False, 'Expected model color to be immutable.')
    self.assertEqual(s1[0].relatedPropertyName(), '',
        'Unexpected related property name "{p}".'.format(p=s1[0].relatedPropertyName()))
    self.assertEqual(s1[0].relatedAttributeId(), UUID('00000000-0000-0000-0000-000000000000'),
        'Unexpected related attribute ID {u}.'.format(u=s1[0].relatedAttributeId()))

    kname = smtk.model.NameForArrangementKind(s1[0].relatedArrangementKind())
    self.assertEqual(kname, 'invalid', 'Unexpected related arrangement kind {k}'.format(k=kname))

    s2 = s1[0].subphrases()
    # Does not work yet:
    #self.assertEqual([s1[0].argFindChild(x) for x in s2], [0, 1],
    #    'Expected to find children where they were reported.')

    # Works but unsure why:
    #self.assertEqual([s1[0].argFindChild(x.relatedEntity()) for x in s2], [0, 1],
    #    'Expected to find children where they were reported.')
    self.assertEqual([x.title() for x in s2], ['Model A, volume 0'],
        'Unexpected grandchild titles.')

    self.spg.setDirectLimit(2)
    s3 = s2[0].subphrases()
    self.assertEqual([x.title() for x in s3], ['5 faces',], 'Expected a summary phrase.')
    self.assertTrue(s2[0].areSubphrasesBuilt(), 'Expected subphrases have been built.')



    # Test that marking a phrase dirty forces its children to be rebuilt
    s2[0].markDirty(True)
    self.assertFalse(s2[0].areSubphrasesBuilt(), 'Expected subphrases need to be built after marking dirty.')

    # Test that a negative limit produces no summary phrases:
    self.spg.setDirectLimit(-1)
    s3 = s2[0].subphrases()
    self.assertEqual(
        [x.title() for x in s3],
        ['Model A, face 0', 'Model A, face 1', 'Model A, face 2', 'Model A, face 3', 'Model A, face 4'],
        'Expected 5 model-face phrases, got {s}.'.format(s=[x.title() for x in s3]))

    print 'Done'

if __name__ == '__main__':
  smtk.testing.process_arguments()
  unittest.main()
