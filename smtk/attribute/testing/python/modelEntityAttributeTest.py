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
Test attributes whose values are model entities and their definition.

This specifically tests the case where the definition requires
model entities of a specific type.
"""

import smtk

if __name__ == '__main__':
    import sys

    status = 0

    attribSys = smtk.attribute.System()
    modelMgr = smtk.model.Manager()
    print 'Systems created'
    def_ = attribSys.createDefinition('testDef')
    if def_ is not None:
        print 'Definition testDef created'
    else:
        print 'ERROR: Definition testDef not created'
        status = -1

    me_e_def = smtk.attribute.ModelEntityItemDefinition.New('Edges')
    me_f_def = smtk.attribute.ModelEntityItemDefinition.New('Faces')
    me_hg_def = smtk.attribute.ModelEntityItemDefinition.New('EdgeOrFaceGroups')
    # Must explicitly cast from ItemDefinition in order to add to Definition
    # Use ToItemDefinition() method that was added to typesystem
    e_def = smtk.attribute.ModelEntityItemDefinition.ToItemDefinition(me_e_def)
    f_def = smtk.attribute.ModelEntityItemDefinition.ToItemDefinition(me_f_def)
    hg_def = smtk.attribute.ModelEntityItemDefinition.ToItemDefinition(me_hg_def)
    #def_.addItemDefinition(e_def)
    #def_.addItemDefinition(f_def)
    me_e_def.setTypeMask(smtk.model.EDGE | smtk.model.GROUP_ENTITY)
    me_f_def.setTypeMask(smtk.model.FACE | smtk.model.GROUP_ENTITY | smtk.model.HOMOGENEOUS_GROUP)
    me_hg_def.setTypeMask(smtk.model.EDGE | smtk.model.FACE | smtk.model.GROUP_ENTITY | smtk.model.HOMOGENEOUS_GROUP)
    sweepCurves = attribSys.createAttribute('SweepCurves', 'Edges')
    sweepSurfaces = attribSys.createAttribute('SweepSurfaces', 'Faces')
    funkyGroup = attribSys.createAttribute('EdgesOrFacesNotBoth', 'EdgeOrFaceGroups')
    fgroup = modelMgr.addGroup(0, 'FaceGroup')
    egroup = modelMgr.addGroup(0, 'EdgeGroup')
    mgroup = modelMgr.addGroup(0, 'MixedGroup')
    faces = []
    edges = []
    for i in range(5):
      faces.append(modelMgr.addFace())
      edges.append(modelMgr.addEdge())
    mixed = faces[:2] + edges[:2]
    [egroup.addEntity(x) for x in edges[2:4]]
    [fgroup.addEntity(x) for x in faces[2:4]]
    [mgroup.addEntity(x) for x in mixed]

    sweepCurves.appendValue(edges[4])   # Should be OK
    sweepCurves.appendValue(faces[4])   # Should NOT be OK
    sweepCurves.appendValue(egroup)     # Should be OK
    sweepCurves.appendValue(fgroup)     # Should NOT be OK

    sweepSurfaces.appendValue(faces[4]) # Should be OK
    sweepSurfaces.appendValue(edges[4]) # Should NOT be OK
    sweepSurfaces.appendValue(fgroup)   # Should be OK
    sweepSurfaces.appendValue(egroup)   # Should NOT be OK

    # Now try fancy stuff
    funkyGroup.appendValue(fgroup)      # Should be OK (all entries are faces or groups of faces)
    funkyGroup.appendValue(egroup)      # Should be OK (each group has only faces or only edges)
    funkyGroup.appendValue(mgroup)      # Should NOT be OK (group has both edges and faces)

    sweepCurves.reset()
    sweepSurfaces.reset()
    # Verify that child group testing works:
    # Add a child group that does not meet requirements.
    fgroup.addEntity(mgroup)
    egroup.addEntity(mgroup)
    sweepCurves.appendValue(egroup)     # Should NOT be OK
    sweepSurfaces.appendValue(fgroup)   # Should NOT be OK

    sys.exit(status)
