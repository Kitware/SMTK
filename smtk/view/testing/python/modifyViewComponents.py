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
import smtk.testing
import smtk.view


class testModifyViewComponents(smtk.testing.TestCase):

    def setUp(self):
        self.attRsc = smtk.attribute.Resource.create()

    def testAddViewComponents(self):
        # Create an empty view
        testView = smtk.view.View.New('Instanced', 'testView')
        self.attRsc.addView(testView)
        # Test addChild() and details()
        attComp = testView.details().addChild(
            'InstancedAttributes').addChild('Att')
        # Make sure attcomp and SameAttComp is a reference
        SameAttComp = attComp.setAttribute('Name', 'Foo')
        SameAttComp.setAttribute('Title', 'Bar')
        instanceIndex = testView.details().findChild('InstancedAttributes')
        if instanceIndex == -1:
            raise Exception('smtk.view.details() failed!')
        attIndex = testView.details().child(instanceIndex).findChild('Att')
        if attIndex == -1:
            raise Exception('smtk.view.component.addChild() failed!')
        # Grab the component again
        attCompAgain = testView.details().child(instanceIndex).child(attIndex)
        success = attCompAgain.attribute('Name')
        if not success:
            raise Exception('smtk.view.component.addChild() failed!')
        success = attCompAgain.attribute('Title')
        if not success:
            raise Exception('smtk.view.component.setAttribute() failed!')
        # Test unsetAttribute
        attCompJr = attComp.unsetAttribute('Title')
        Found = attComp.attribute('Title')
        if Found:
            raise Exception('smtk.view.component.unsetAttribute() failed!')
        attCompJr.setAttribute('Title', 'Foo')
        success = attComp.attribute('Title')
        if not success:
            raise Exception('smtk.view.component.unsetAttribute() failed!')

        # Test Child()
        attCompAgain.setAttribute('Type', 'Baz')
        attCompFinal = testView.details().child(instanceIndex).child(attIndex)
        success = attCompFinal.attribute('Type')
        if not success:
            raise Exception('smtk.view.component.Child() failed!')

        # Print all the attributes. Note pybind drops its const-ness so we do
        # not want it to return the attributes map by reference
        compAtts = attComp.attributes()
        print('Attributes in the component:')
        for att, value in compAtts.items():
            print('Attribute: {}, Value: {}'.format(att, value))


if __name__ == '__main__':
    smtk.testing.process_arguments()
    smtk.testing.main()
