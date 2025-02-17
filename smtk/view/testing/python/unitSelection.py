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
import smtk.testing
import smtk.view

import sys
import uuid

from copy import copy

eventCount = 0
eventCount2 = 0


class TestSelection(smtk.testing.TestCase):

    def setUp(self):
        self.selnMgr = smtk.view.Selection.create()

    def tearDown(self):
        self.selnMgr = None

    def loadTestData(self):
        import os
        self.mgr = smtk.model.Resource.create()
        fpath = [smtk.testing.DATA_DIR, 'model',
                 '2d', 'smtk', 'epic-trex-drummer.smtk']
        op = smtk.session.polygon.Read.create()
        op.parameters().find('filename').setValue(os.path.join(*fpath))
        res = op.operate()
        if res.find('outcome').value(0) != int(smtk.operation.Operation.SUCCEEDED):
            raise RuntimeError
        self.resource = smtk.model.Resource.CastTo(
            res.find('resourcesCreated').value())
        self.model = self.resource.findEntitiesOfType(
            int(smtk.model.MODEL_ENTITY))[0]

    def testSelectionValues(self):
        mgr = self.selnMgr
        self.assertFalse(mgr.registerSelectionValue(
            'unselected', 0), 'Should not be able to register value of 0.')
        self.assertTrue(mgr.registerSelectionValue(
            'hover', 1), 'Should be able to register non-zero value.')
        self.assertFalse(mgr.registerSelectionValue(
            'barf', 1), 'Should not be able to re-register value.')
        self.assertTrue(mgr.registerSelectionValue('barfo', 1, False),
                        'Should be able to re-register value explicitly.')
        self.assertTrue(mgr.registerSelectionValue(
            'selection', 2), 'Should be able to register multiple values.')
        self.assertTrue(mgr.registerSelectionValue('naughty', 2, False),
                        'Should be able to re-register value explicitly.')

        self.assertEqual(mgr.selectionValueFromLabel(
            'hover'), 1, 'Registered values do not match.')
        self.assertEqual(mgr.selectionValueFromLabel(
            'barfo'), 1, 'Registered values do not match.')
        self.assertEqual(mgr.selectionValueFromLabel(
            'selection'), 2, 'Registered values do not match.')
        self.assertEqual(mgr.selectionValueFromLabel(
            'naughty'), 2, 'Registered values do not match.')
        svl = {u'selection': 2, u'naughty': 2, u'hover': 1, u'barfo': 1}
        self.assertEqual(mgr.selectionValueLabels(),
                         svl, 'Unexpected selection value labels.')
        self.assertEqual(mgr.findOrCreateLabeledValue(
            'selection'), 2, 'Did not find pre-existing labeled value.')
        selnLbl = 'norkit'
        selnVal = mgr.findOrCreateLabeledValue(selnLbl)
        svl[selnLbl] = selnVal

        self.assertTrue(mgr.unregisterSelectionValue(
            'naughty'), 'Could not unregister extant selection value.')
        self.assertFalse(mgr.unregisterSelectionValue(
            'naughty'), 'Could unregister extinct selection value.')
        self.assertFalse(mgr.unregisterSelectionValue(
            'nutty'), 'Could unregister non-existent selection value.')
        del svl['naughty']
        for kk, vv in svl.items():
            self.assertEqual(mgr.selectionValueFromLabel(
                kk), vv, 'Failed to look up selection value from label.')
            self.assertTrue(mgr.unregisterSelectionValue(
                kk), 'Failed to unregister a selection value label.')

    def testSelectionSources(self):
        mgr = self.selnMgr
        self.assertTrue(mgr.registerSelectionSource(
            'foo'), 'Could not register selection source.')
        self.assertFalse(mgr.registerSelectionSource(
            'foo'), 'Could re-register selection source.')
        self.assertTrue(mgr.unregisterSelectionSource(
            'foo'), 'Could not unregister extant selection source.')
        self.assertFalse(mgr.unregisterSelectionSource(
            'foo'), 'Could unregister extinct selection.')

    def testCreateAndInstance(self):
        self.assertTrue(self.selnMgr, 'Unexpectedly null manager.')
        self.assertEqual(
            self.selnMgr, smtk.view.Selection.instance(),
            'Expected instance to match first selection manager.')
        self.selnMgr = None
        mgr = smtk.view.Selection.instance()
        self.assertTrue(
            mgr, 'Expected a default instance to be created as required.')
        other = smtk.view.Selection.create()
        self.assertNotEqual(
            mgr, other, 'Selection manager should not be a singleton.')
        self.assertEqual(mgr, smtk.view.Selection.instance(),
                         'Selection manager reported by instance should survive creation of others.')

    def testSelectionModificationFilterObserve(self):
        self.loadTestData()
        mgr = self.selnMgr

        def observer(src, smgr):
            global eventCount
            eventCount += 1

        # Test that observer is called at proper times:
        handle = mgr.observers().insert(observer, 0, True, 'Test selection observation.')
        self.assertTrue(
            handle.assigned(), 'Failed to register selection observer.')
        expectedEventCount = 1
        self.assertEqual(eventCount, expectedEventCount,
                         'Selection observer was not called immediately.')
        comp = self.resource.find(self.model.entity())
        selnVal = mgr.findOrCreateLabeledValue('selection')
        selnSrc = 'testSelectionModificationObserve'
        mgr.registerSelectionSource(selnSrc)
        mgr.registerSelectionValue('selection', selnVal)

        # Test selection modification and ensure observer is called properly:
        mgr.modifySelection(
            [comp, ], selnSrc, selnVal, smtk.view.SelectionAction.DEFAULT, False)
        expectedEventCount += 1
        self.assertEqual(eventCount, expectedEventCount,
                         'Selection observer was not called upon change.')
        mgr.modifySelection(
            [comp, ], selnSrc, selnVal, smtk.view.SelectionAction.FILTERED_ADD, False)
        self.assertEqual(eventCount, expectedEventCount,
                         'Selection observer was called when there should be no event.')
        mgr.modifySelection(
            [], selnSrc, selnVal, smtk.view.SelectionAction.DEFAULT, False)
        expectedEventCount += 1
        self.assertEqual(eventCount, expectedEventCount,
                         'Selection observer was not called upon modification.')

        # Test that filtering works
        def allPassFilter(comp, lvl, sugg):
            return True
        mgr.modifySelection(
            [comp, ], selnSrc, selnVal, smtk.view.SelectionAction.DEFAULT, False)
        expectedEventCount += 1
        mgr.setFilter(allPassFilter)
        self.assertEqual(eventCount, expectedEventCount,
                         'Selection observer was called upon no-op filter pass.')

        def nonePassFilter(comp, lvl, sugg):
            return False
        mgr.setFilter(nonePassFilter)
        expectedEventCount += 1
        self.assertEqual(eventCount, expectedEventCount,
                         'Selection observer was not called upon filter pass.')

        # We cannot test filters that suggest new values in Python
        # because pybind copies the suggestions map by value rather
        # than passing a reference:
        # def suggestFilter(comp, lvl, sugg):
        #     model = smtk.model.Model(comp.modelResource(), comp.id())
        #     for cell in model.cells():
        #         sugg[cell] = lvl
        #     return False
        # mgr.setFilter(suggestFilter)
        # mgr.modifySelection([comp,], selnSrc, selnVal, smtk.view.SelectionAction.DEFAULT, False)
        # expectedEventCount += 1
        # print(mgr.currentSelection())
        # self.assertEqual(eventCount, expectedEventCount, 'Selection observer
        # was not called upon suggestion pass.')

        self.assertTrue(mgr.observers().erase(
            handle), 'Could not unregister observer')
        self.assertFalse(
            mgr.observers().erase(handle), 'Could double-unregister observer')

        # Test enumeration of selection
        model = smtk.model.Model(comp.modelResource(), comp.id())
        cellComps = [self.resource.find(cell.entity())
                     for cell in model.cells()]
        mgr.setFilter(None)
        mgr.modifySelection(
            cellComps, selnSrc, selnVal, smtk.view.SelectionAction.DEFAULT, False)
        dd = {kk.id(): vv for kk, vv in mgr.currentSelection().items()}
        print(
            ''.join(['  ', '\n  '.join(['%s: %d' % (str(kk), vv) for kk, vv in dd.items()])]))
        ddExpected = {
            uuid.UUID('e2af5e59-6c2f-4cfe-bdd5-8e806906de44'): 1,
            uuid.UUID('6382323f-e8e4-455e-b889-fd1c0dc40be5'): 1,
            uuid.UUID('3894f798-75a9-4b74-9d53-d93ddbb513d1'): 1,
            uuid.UUID('77b2dbdd-0a70-47a9-a132-1af51353769c'): 1,
            uuid.UUID('12e0f395-8c51-4ee3-aafe-7fc108e8f48a'): 1,
            uuid.UUID('12706f3a-a528-440e-ad02-0eb907d0079a'): 1,
            uuid.UUID('36ccbcf5-240a-4f99-88ba-c5418fcfef10'): 1,
            uuid.UUID('3885d08e-c9bb-4d29-aa80-0143e50bea81'): 1,
            uuid.UUID('44c34ccc-284f-49f4-afd0-8efa59e115f9'): 1,
            uuid.UUID('e996d852-1b9a-43a5-a98a-e58bad72c207'): 1,
            uuid.UUID('04fa7479-5d03-4b7c-8fbd-1a3e2341baf6'): 1,
            uuid.UUID('5ee2ab86-3d5c-4350-82ca-83afdeb425a4'): 1,
            uuid.UUID('977e9cb5-a657-4b66-bf76-9cfeb9409171'): 1,
            uuid.UUID('8946acb8-7ad5-4a13-981d-b04881ff9248'): 1,
            uuid.UUID('fc2b03b4-8591-4796-9968-881d9461e1e6'): 1,
            uuid.UUID('69504b3f-35a3-41ca-a50d-78c31216f134'): 1
        }
        self.assertEqual(dd, ddExpected, 'Unexpected selection.')
        global visitSeln
        visitSeln = {}

        def selectionVisitor(comp, lvl):
            global visitSeln
            visitSeln[comp.id()] = lvl
        mgr.visitSelection(selectionVisitor)
        self.assertEqual(
            visitSeln, ddExpected, 'Did not visit selection properly.')

    def testBitwiseOperations(self):
        self.loadTestData()
        mgr = self.selnMgr

        def observer(src, smgr):
            global eventCount2
            eventCount2 += 1

        # Test that observer is called at proper times:
        handle = mgr.observers().insert(observer, 0, True, 'Test selection observation.')

        self.assertTrue(
            handle.assigned(), 'Failed to register selection observer.')
        expectedEventCount = 1

        comp = self.resource.find(self.model.entity())
        model = smtk.model.Model(comp.modelResource(), comp.id())
        cellComps = [self.resource.find(cell.entity())
                     for cell in model.cells()]
        lots = copy(cellComps)

        selnVal = mgr.findOrCreateLabeledValue('selection')
        selnVal2 = mgr.findOrCreateLabeledValue('selection2')
        selnSrc = 'testSelectionModificationObserve'
        mgr.registerSelectionSource(selnSrc)
        mgr.registerSelectionValue('selection', selnVal)
        mgr.registerSelectionValue('selection2', selnVal2)
        print('Selection values: %d %d' % (selnVal, selnVal2))

        # Test selection modification and ensure observer is called properly:
        mgr.modifySelection(
            lots, selnSrc, selnVal, smtk.view.SelectionAction.DEFAULT, False)
        expectedEventCount += 1

        # Add the model to the list of entities so each selection bit
        # has >= 1 component with *only* that bit present.
        lots.append(comp)

        mgr.modifySelection(
            lots[8:], selnSrc, selnVal2, smtk.view.SelectionAction.FILTERED_ADD, True)
        expectedEventCount += 1

        print('Before: %d items in selection (%d cells)' %
              (len(mgr.currentSelection()), len(cellComps)))
        self.assertEqual(eventCount2, expectedEventCount,
                         'Selection observer was not called.')
        self.assertEqual(len(mgr.currentSelection()), 17,
                         'Expected 16 cells + 1 model in selection.')

        # Remove the selnVal2 bit (this should remove comp from the selection)
        mgr.resetSelectionBits(selnSrc, selnVal2)
        expectedEventCount += 1
        print('After1: %d items in selection' % len(mgr.currentSelection()))
        self.assertEqual(len(mgr.currentSelection()), 16,
                         'Expected 16 cells in selection at this point.')
        self.assertEqual(eventCount2, expectedEventCount,
                         'Selection observer was not called upon modification.')

        # Remove the selnVal2 bit again (resulting in no changes)
        mgr.resetSelectionBits(selnSrc, selnVal2)
        print('After2: %d items in selection' % len(mgr.currentSelection()))
        self.assertEqual(len(mgr.currentSelection()), 16,
                         'Expected 16 cells in selection at this point.')
        self.assertEqual(eventCount2, expectedEventCount,
                         'Selection observer was not called upon modification.')

        # Remove the selnVal bit (this should remove lots[8:] from the
        # selection)
        mgr.resetSelectionBits(selnSrc, selnVal)
        expectedEventCount += 1
        print('After3: %d items in selection' % len(mgr.currentSelection()))
        self.assertEqual(eventCount2, expectedEventCount,
                         'Selection observer was not called upon modification.')
        self.assertEqual(len(mgr.currentSelection()), 0,
                         'Expected 0 cells in selection at this point.')

        print('')


if __name__ == '__main__':
    smtk.testing.process_arguments()
    smtk.testing.main()
