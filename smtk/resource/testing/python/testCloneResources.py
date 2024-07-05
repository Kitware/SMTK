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
import smtk.attribute
import smtk.graph
import smtk.string
import smtk.io
import smtk.resource
import smtk.testing
import sys

try:
    import smtk.markup
    haveMarkup = True
except:
    print('Skipping TestCloneResources due to missing markup support.')
    haveMarkup = False
    sys.exit(smtk.testing.SKIP_ENTIRE)


class TestCloneResources(smtk.testing.TestCase):

    def setUp(self):
        # Read in the original attribute resource
        attFilename = os.path.join(
            smtk.testing.DATA_DIR, 'attribute', 'attribute_collection', 'cloneTest.smtk')
        rr = smtk.read(attFilename)
        self.origAttResource = rr[0]
        self.origAttResource.setName('attResource')
        # Read in the original markup resource
        markupFilename = os.path.join(
            smtk.testing.DATA_DIR, 'model', '3d', 'smtk', 'coarse-knee.smtk')
        rr = smtk.read(markupFilename)
        self.origMarkupResource = rr[0]
        # add a resource property to xx to test that it gets copied
        # even when component properties are not.
        self.origMarkupResource.stringProperties().set('foo', 'bar')
        self.origAttResource.stringProperties().set('foo', 'bar')

        # Let's associate the markup resource to the attribute resource
        self.origAttResource.associate(self.origMarkupResource)

        compset = self.origMarkupResource.filter(
            'smtk::markup::UnstructuredData')
        att = self.origAttResource.findAttribute("Test Attribute")
        refitem = att.associations()
        refitem.setValue(0, compset.pop())
        # The original attribute did not have any active categories or analyses so let's add some
        self.origAttResource.setActiveCategories({"A", "B"})
        self.origAttResource.setActiveCategoriesEnabled(True)
        analysis = self.origAttResource.analyses().create("foo")
        analysis.setLocalCategories({"A"})
        print('------- Setup Results --------')
        print('  +++++ Original Attribute Resource ++++++')
        self.printResource(self.origAttResource)
        print('  +++++ Original Markup Resource ++++++')
        self.printResource(self.origMarkupResource)

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
        att = resource.findAttribute("Test Attribute")
        refitem = att.associations()
        print('  association: ', refitem.value(
            0).name(), refitem.value(0).id(), '\n')
        d0 = att.findDouble("d0")
        d1 = att.findDouble("d1")
        print('  d0 = ', d0.value(0),
              ' d1 = {', d1.expression().name(), d1.expression().id(), '}\n')
        view = resource.findView("AttributeResourceCloneTest")
        val = view.details().child(0).child(0).attributeAsString("ID")
        if view.details().child(0).child(0).attribute("ID"):
            print('  View: ', view.name(), ' has Attribute ID: ', val)
        else:
            print('  View: ', view.name(), ' does not have an ID Attribute')

    def testCloneAttributeResourceOnly(self):
        print('-------- Testing Copying only the Attribute Resource --------')
        opts = smtk.resource.CopyOptions(smtk.io.Logger.instance())
        opts.setCopyLocation(True)  # Normally this is false

        clonedAtts = self.origAttResource.clone(opts)

        if not clonedAtts.copyInitialize(self.origAttResource, opts):
            raise RuntimeError(
                'Copy Initialized Failed for Attribute Resource')

        print('Copy Initialized for Attributes Worked.')
        if not clonedAtts.copyFinalize(self.origAttResource, opts):
            raise RuntimeError(
                'Copy Finalized Failed for Attribute Resource')

        print('Copy Finalized for Attributes Worked.')
        print('  +++++ Cloned Attribute Resource ++++++')
        self.printResource(clonedAtts)
        self.compareClonedAttResource(clonedAtts, 1)

    def testCloneAllResource(self):
        print('-------- Testing Copying All Resources --------')
        opts = smtk.resource.CopyOptions(smtk.io.Logger.instance())
        opts.setCopyLocation(True)  # Normally this is false

        clonedAtts = self.origAttResource.clone(opts)
        clonedMarkup = self.origMarkupResource.clone(opts)
        if not clonedAtts.copyInitialize(self.origAttResource, opts):
            raise RuntimeError(
                'Copy Initialized Failed for Attribute Resource')

        if not clonedMarkup.copyInitialize(self.origMarkupResource, opts):
            raise RuntimeError('Copy Initialized Failed for Markup Resource')

        print('Copy Initialized for Resources Worked.')

        if not clonedAtts.copyFinalize(self.origAttResource, opts):
            raise RuntimeError('Copy Finalized Failed for Attribute Resource')

        if not clonedMarkup.copyFinalize(self.origMarkupResource, opts):
            raise RuntimeError('Copy Finalized Failed for Markup Resource')

        print('Copy Finalized for Resources Worked.')
        print('  +++++ Cloned Attribute Resource ++++++')
        self.printResource(clonedAtts)
        print('  +++++ Cloned Markup Resource ++++++')
        self.printResource(clonedMarkup)
        self.compareClonedAttResource(clonedAtts, 0)

    def compareClonedAttResource(self, attRes, sameMarkup):
        if attRes.stringProperties().contains('foo'):
            if attRes.stringProperties().at('foo') != 'bar':
                raise RuntimeError(
                    'Copied Attribute Resource do not have a string property foo equal to bar')
        else:
            raise RuntimeError(
                'Copied Attribute Resource do not have a string property foo')
        if attRes.activeCategoriesEnabled() != self.origAttResource.activeCategoriesEnabled():
            raise RuntimeError(
                'Attribute Resources do not have the same active categories enabled option')
        if attRes.activeCategories() != self.origAttResource.activeCategories():
            raise RuntimeError(
                'Attribute Resources do not have the same active categories')
        if attRes.analyses().size() != self.origAttResource.analyses().size():
            raise RuntimeError(
                'Attribute Resources do not have the same number of analyses')
        if attRes.id() == self.origAttResource.id():
            raise RuntimeError('Attribute Resources have same ID')

        origAtt = self.origAttResource.findAttribute("Test Attribute")
        att = attRes.findAttribute("Test Attribute")
        if att.id() == origAtt.id():
            raise RuntimeError('Attributes have same ID')

        d0 = att.findDouble("d0")
        d1 = att.findDouble("d1")
        origD0 = origAtt.findDouble("d0")
        origD1 = origAtt.findDouble("d1")
        if d0.value() != origD0.value():
            raise RuntimeError('Items d0 do not have the same value')

        if d1.expression().id() == origD1.expression().id():
            raise RuntimeError('Items d1 do the same expression attribute')

        attResAsso = attRes.associations()
        origAttResAsso = self.origAttResource.associations()
        # The copy should have the same number of associates resources as the source
        if len(attResAsso) != len(origAttResAsso):
            raise RuntimeError(
                'Copied Attribute Resources has incorrect number of associated resources')

        attAsso = att.associations()
        origAttAsso = origAtt.associations()

        if bool(sameMarkup):
            if len(attResAsso):
                if not (self.origMarkupResource in attResAsso):
                    raise RuntimeError(
                        'Copied Attribute Resource is not associated to the origin MarkUp Resource and should be')
            if attAsso.value(0).id() != origAttAsso.value(0).id():
                raise RuntimeError(
                    'Attributes are not associated with the same component and should be')
        else:
            if len(attResAsso):
                if self.origMarkupResource in attResAsso:
                    raise RuntimeError(
                        'Copied Attribute Resource is  associated to the origin MarkUp Resource and not should be')
            if attAsso.value(0).id() == origAttAsso.value(0).id():
                raise RuntimeError(
                    'Attributes are associated with the same component and should not be')

        # Lets compare the views - the Attribute IDs should be different if they exist
        view = attRes.findView("AttributeResourceCloneTest")
        val = view.details().child(0).child(0).attributeAsString("ID")

        origView = self.origAttResource.findView("AttributeResourceCloneTest")
        origVal = origView.details().child(0).child(0).attributeAsString("ID")
        if val == origVal:
            raise RuntimeError(
                'View contains IDs that point to the same attribute which is wrong')

        print('Comparing Attribute Resources Succeeded!')


if __name__ == '__main__' and haveMarkup:
    smtk.testing.process_arguments()
    smtk.testing.main()
