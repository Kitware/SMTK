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
import smtk
import smtk.attribute
import smtk.session.polygon
import smtk.model
import smtk.testing
import sys
import uuid


class TestEntityInstances(smtk.testing.TestCase):

    def setUp(self):
        import os
        fpath = [smtk.testing.DATA_DIR, 'model',
                 '2d', 'smtk', 'epic-trex-drummer.smtk']
        op = smtk.session.polygon.Read.create()
        op.parameters().find('filename').setValue(os.path.join(*fpath))
        res = op.operate()
        if res.find('outcome').value(0) != int(smtk.operation.Operation.SUCCEEDED):
            raise RuntimeError
        self.mgr = smtk.model.Resource.CastTo(
            res.find('resourcesCreated').value())
        self.model = self.mgr.findEntitiesOfType(
            int(smtk.model.MODEL_ENTITY))[0]
        self.faces = self.model.cells()

    def testInstanceCreation(self):
        inst = self.mgr.addInstance(self.faces[0])
        self.assertTrue(inst.isValid(), 'Could not create instance')
        self.assertEqual(
            inst.owningModel(), self.model, 'Instance not properly owned')
        inst.assignDefaultName()
        print('  Instance %s created' % inst.name())
        self.assertEqual(
            inst.name(), 'instance 0', 'Bad instance default name')
        self.assertEqual(
            len(self.faces[0].instances()), 1, 'Could not retrieve instance from prototype')

    def testInstanceRules(self):
        inst = self.mgr.addInstance(self.faces[0])
        self.assertTrue(inst.setRule('uniform random'),
                        'Setting rule should return true on initially')
        self.assertFalse(inst.setRule('uniform random'),
                         'Setting rule should return false when duplicate')

    def testInstanceSnapping(self):
        faces = self.faces
        inst = self.mgr.addInstance(faces[0])
        self.assertTrue(
            inst.setSnapEntity(faces[1]), 'Setting snap-entity should return true initially')
        self.assertFalse(
            inst.setSnapEntity(faces[1]), 'Setting snap-entity should return false when duplicate')
        self.assertTrue(
            inst.addSnapEntity(faces[2]), 'Adding snap-entity should return true initially')
        self.assertFalse(
            inst.addSnapEntity(faces[2]), 'Adding snap-entity should return false when duplicate')
        self.assertTrue(inst.removeSnapEntity(
            faces[2]), 'Removing snap-entity should return true initially')
        self.assertFalse(inst.removeSnapEntity(
            faces[2]), 'Removing snap-entity should return false when impossible')
        self.assertTrue(inst.setSnapEntities(set(faces)),
                        'Setting snap-entities should return true initially')
        self.assertFalse(inst.setSnapEntities(set(faces)),
                         'Setting snap-entities should return false when duplicate')
        self.assertTrue(inst.setSnapEntities(set()),
                        'Clearing snap-entities should return true when erasing')
        self.assertFalse(inst.setSnapEntities(set()),
                         'Clearing snap-entities should return false when already empty')

    def testInstanceUniformRandom(self):
        inst = self.mgr.addInstance(self.faces[0])
        inst.setRule('uniform random')
        tess = inst.hasTessellation()
        self.assertEqual(
            len(tess.coords()), 0, 'Expected invalid rule properties to yield no placements')

        inst.setFloatProperty('voi', [0, 1, 2, 3, 4, 5])
        inst.setIntegerProperty('sample size', 20)
        tess = inst.generateTessellation()
        self.assertTrue(inst.hasIntegerProperty('seed'),
                        'Expected a seed after forcing tessellation generation')
        crd = tess.coords()
        pts = [tuple(crd[i:i + 3]) for i in range(0, len(crd), 3)]
        self.assertEqual(len(pts), inst.integerProperty(
            'sample size')[0], 'Wrong number of placements')

        inst.generateTessellation()
        crd = tess.coords()
        pt2 = [tuple(crd[i:i + 3]) for i in range(0, len(crd), 3)]
        self.assertEqual(
            pts, pt2, 'Expected retessellation of random instance with same seed to be the same')

        inst.setIntegerProperty('seed', inst.integerProperty('seed')[0] + 42)
        inst.generateTessellation()
        crd = tess.coords()
        pt2 = [tuple(crd[i:i + 3]) for i in range(0, len(crd), 3)]
        self.assertNotEqual(
            pts, pt2, 'Expected retessellation of random instance with same seed to be the same')

    def testInstanceTabular(self):
        inst = self.mgr.addInstance(self.faces[0])
        inst.setRule('tabular')
        inst.setFloatProperty('placements', [1, 0, 0, 0, 1, 0, 0, 0, 1])
        tess = inst.hasTessellation()


if __name__ == '__main__':
    smtk.testing.process_arguments()
    smtk.testing.main()
