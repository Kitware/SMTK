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
import smtk.attribute
import smtk.graph
import smtk.string
import smtk.io
import smtk.testing
import sys

try:
    import smtk.markup
    haveMarkup = True
except:
    print('Skipping TestCloneResources due to missing markup support.')
    haveMarkup = False
    sys.exit(smtk.testing.SKIP_ENTIRE)


def printLogOnFail(ok, opts):
    if not ok:
        print('Log output:\n', opts.log().convertToString(True), '\n')
    return ok


class TestCloneResources(smtk.testing.TestCase):

    def setUp(self):
        # Reset the smtk.common.Managers object used by smtk.read() to eliminate
        # previously-read resources from memory:
        if smtk.appContext:
            rsrcMgr = smtk.appContext.get('smtk.resource.Manager')
            for rsrc in rsrcMgr.resources():
                rsrcMgr.remove(rsrc)

        # Read in the original attribute resource
        attFilename = os.path.join(
            smtk.testing.DATA_DIR, 'attribute', 'attribute_collection', 'cloneTest.smtk')
        self.origAttResource = smtk.read(attFilename)[0]
        self.origAttResource.setName('attResource')
        # Read in the original markup resource
        markupFilename = os.path.join(
            smtk.testing.DATA_DIR, 'model', '3d', 'smtk', 'coarse-knee.smtk')
        self.origMarkupResource = smtk.read(markupFilename)[0]
        # add a resource property to xx to test that it gets copied
        # even when component properties are not.
        self.origMarkupResource.stringProperties().set('foo', 'bar')
        self.origAttResource.stringProperties().set('foo', 'bar')

        # Let's associate the markup resource to the attribute resource
        self.assertTrue(self.origAttResource.associate(self.origMarkupResource),
                        'Could not associatte attribute resource to markup resource.')

        compset = self.origMarkupResource.filter(
            'smtk::markup::UnstructuredData')
        att = self.origAttResource.findAttribute('Test Attribute')
        refitem = att.associations()
        refitem.setValue(0, compset.pop())
        # The original attribute did not have any active categories or analyses so let's add some
        self.origAttResource.setActiveCategories({'A', 'B'})
        self.origAttResource.setActiveCategoriesEnabled(True)
        analysis = self.origAttResource.analyses().create('foo')
        analysis.setLocalCategories({'A'})

        # Create a "visual link" between an attribute and a markup component.
        self.originalMarkupComp = compset.pop()
        mcomp = self.originalMarkupComp
        att.links().addLinkTo(mcomp, smtk.resource.Resource.visuallyLinkedRole())
        self.assertTrue(att.links().isLinkedTo(mcomp, smtk.resource.Resource.visuallyLinkedRole()),
                        'Could not visually link attribute to markup component.')

        print('------- Setup Results --------')
        print(self.origMarkupResource.id(), self.origMarkupResource.name())
        print(self.originalMarkupComp.id(), self.originalMarkupComp.name())
        print(self.origAttResource.id(), self.origAttResource.name())
        print(att.id(), att.name())
        print('Visually linked', mcomp.name(), 'to', att.name())
        self.printAttInfo(self.origAttResource)

    def printResource(self, resource):
        print(resource, resource.id())
        # Print all of its components:
        resource.visit(lambda comp: print(
            '  ', comp.id(), comp.typeName(), comp.name()))
        # Print the property names segregated by property storage-type:
        for prop in resource.properties().types():
            print(prop.data())
            print('  ', '\n  '.join(
                [ww.data() for ww in resource.properties().namesForType(prop)]))

        # Attribute Resource Specifics
        if resource.matchesType(smtk.string.Token('smtk::attribute::Resource')):
            self.printAttInfo(resource)

    def printAttInfo(self, resource):
        # Print the number of resources directly associated with it
        print('  number of associated resources:',
              len(resource.associations()))
        # Print the number of analyses
        print('  number of analyses:', resource.analyses().size())
        # print the active categories
        print('  active categories:', resource.activeCategories())
        print('  active categories enabled:',
              resource.activeCategoriesEnabled())
        # Find the test attribute
        att = resource.findAttribute('Test Attribute')
        refitem = att.associations()
        print('  association: ', refitem.value(
            0).name(), refitem.value(0).id(), '\n')
        d0 = att.findDouble('d0')
        d1 = att.findDouble('d1')
        print('  d0 = ', d0.value(0),
              ' d1 = {', d1.expression().name(), d1.expression().id(), '}\n')
        view = resource.findView('AttributeResourceCloneTest')
        val = view.details().child(0).child(0).attributeAsString('ID')
        if view.details().child(0).child(0).attribute('ID'):
            print('  View: ', view.name(), ' has Attribute ID: ', val)
        else:
            print('  View: ', view.name(), ' does not have an ID Attribute')

    def testCloneAttributeResourceOnly(self):
        print('-------- Testing Copying only the Attribute Resource --------')
        opts = smtk.resource.CopyOptions(smtk.io.Logger.instance())
        opts.setCopyLocation(True)  # Normally this is false

        clonedAtts = self.origAttResource.clone(opts)

        self.assertTrue(
            printLogOnFail(clonedAtts.copyInitialize(
                self.origAttResource, opts), opts),
            'copyInitialize Failed for attribute resource')
        self.assertTrue(
            printLogOnFail(clonedAtts.copyFinalize(
                self.origAttResource, opts), opts),
            'copyFinalize Failed for attribute resource')

        # print('  +++++ Cloned Attribute Resource ++++++')
        # self.printResource(clonedAtts)
        self.printAttInfo(clonedAtts)
        self.compareClonedAttResource(clonedAtts, True, opts)

    def testCloneAllResource(self):
        print('-------- Testing Copying All Resources --------')
        opts = smtk.resource.CopyOptions(smtk.io.Logger.instance())
        opts.setCopyLocation(True)  # Normally this is false

        clonedAtts = self.origAttResource.clone(opts)
        clonedMarkup = self.origMarkupResource.clone(opts)

        self.assertTrue(
            printLogOnFail(clonedAtts.copyInitialize(
                self.origAttResource, opts), opts),
            'copyInitialize failed for attribute resource')
        self.assertTrue(
            printLogOnFail(clonedMarkup.copyInitialize(
                self.origMarkupResource, opts), opts),
            'copyInitialize failed for markup resource')

        self.assertTrue(
            printLogOnFail(clonedAtts.copyFinalize(
                self.origAttResource, opts), opts),
            'copyFinalize failed for attribute resource')
        self.assertTrue(
            printLogOnFail(clonedMarkup.copyFinalize(
                self.origMarkupResource, opts), opts),
            'copyFinalize failed for markup resource')

        # print('  +++++ Cloned Attribute Resource ++++++')
        # self.printResource(clonedAtts)
        self.printAttInfo(clonedAtts)
        # print('  +++++ Cloned Markup Resource ++++++')
        # self.printResource(clonedMarkup)
        self.compareClonedAttResource(clonedAtts, False, opts)

    def compareClonedAttResource(self, attRes, sameMarkup, opts):
        self.assertTrue(attRes.stringProperties().contains(
            'foo'), 'Missing "foo" property.')
        self.assertEqual(attRes.stringProperties().at('foo'), 'bar',
                         'Copied attribute resource does not have a string property foo equal to bar.')

        self.assertEqual(attRes.activeCategoriesEnabled(), self.origAttResource.activeCategoriesEnabled(),
                         'Attribute resources do not have the same active categories enabled.')
        self.assertEqual(attRes.activeCategories(), self.origAttResource.activeCategories(),
                         'Attribute Resources do not have the same active categories.')
        self.assertEqual(attRes.analyses().size(), self.origAttResource.analyses().size(),
                         'Attribute Resources do not have the same number of analyses.')
        self.assertNotEqual(attRes.id(), self.origAttResource.id(),
                            'Attribute Resources have same ID')

        origAtt = self.origAttResource.findAttribute('Test Attribute')
        att = attRes.findAttribute('Test Attribute')
        self.assertNotEqual(att.id(), origAtt.id(), 'Attributes have same ID.')

        d0 = att.findDouble('d0')
        d1 = att.findDouble('d1')
        origD0 = origAtt.findDouble('d0')
        origD1 = origAtt.findDouble('d1')
        self.assertEqual(d0.value(), origD0.value(),
                         'Items d0 do not have the same value.')

        self.assertNotEqual(d1.expression().id(), origD1.expression().id(),
                            'Items d1 have the same expression attribute, but it should be distinct.')

        attResAsso = attRes.associations()
        origAttResAsso = self.origAttResource.associations()
        print('attRes assocs:')
        for ar in attResAsso:
            print('  ', ar.name(), ar.id())
        print('origAttRes assocs:')
        for ar in origAttResAsso:
            print('  ', ar.name(), ar.id())
        print('--')
        # The copy should have the same number of associates resources as the source
        self.assertEqual(len(attResAsso), len(origAttResAsso),
                         'Copied Attribute Resources has incorrect number of associated resources.')

        attAsso = att.associations()
        origAttAsso = origAtt.associations()

        if sameMarkup:
            if len(attResAsso):
                self.assertTrue(self.origMarkupResource in attResAsso,
                                'Copied attribute resource is not associated to the original markup resource and should be.')
            self.assertEqual(attAsso.value(0).id(), origAttAsso.value(0).id(),
                             'Attributes are not associated with the same component and should be.')
        else:
            if len(attResAsso):
                self.assertFalse(self.origMarkupResource in attResAsso,
                                 'Copied attribute resource is associated to the original markup resource and should not be.')
            self.assertNotEqual(attAsso.value(0).id(), origAttAsso.value(0).id(),
                                'Attributes are associated with the same component and should not be.')

        # Lets compare the views - the Attribute IDs should be different if they exist
        view = attRes.findView('AttributeResourceCloneTest')
        val = view.details().child(0).child(0).attributeAsString('ID')

        origView = self.origAttResource.findView('AttributeResourceCloneTest')
        origVal = origView.details().child(0).child(0).attributeAsString('ID')
        self.assertNotEqual(
            val, origVal, 'View contains IDs that point to the same attribute which is wrong.')

        mcomp = self.originalMarkupComp
        if sameMarkup:
            self.assertTrue(att.links().isLinkedTo(mcomp, smtk.resource.Resource.visuallyLinkedRole()),
                            'Visual link not copied.')
        else:
            self.assertFalse(att.links().isLinkedTo(mcomp, smtk.resource.Resource.visuallyLinkedRole()),
                             'Visually linked to original (not new) component.')
            ncomp = opts.targetComponentFromSourceId(mcomp.id())
            self.assertTrue(att.links().isLinkedTo(ncomp, smtk.resource.Resource.VisuallyLinkedRole),
                            'Visual link not preserved in cloned resource.')

        print('Comparing Attribute Resources Succeeded!')


if __name__ == '__main__' and haveMarkup:
    smtk.testing.process_arguments()
    smtk.testing.main()
