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
Demonstrate model construction from within Python.

This construction technique is similar to how Session subclasses will create models.
Sessions assign UUIDs to model entities they transcribe as SMTK requests access to
the model entities. These UUIDs may be saved from previous sessions, so allowing
SMTK to assign a new, random UUID to a model entity being translated is not an option.
Instead, the Session mandates the UUID.

This example uses insertXXX() and setXXX() methods to create SMTK-model
entries with externally-provided UUIDs.
"""

import os
import smtk

if __name__ == '__main__':
  import sys
  status = 0
  try:
    store = smtk.model.Manager.create()
    # Add some models to the model manager:
    model = store.addModel(3, 3, 'Test Model')
    # Create a model as if we were importing it (i.e., UUIDs already assigned).
    # This tests the methods created for use by Session subclasses.
    ugen = smtk.common.UUIDGenerator()
    uids = [ugen.random() for x in range(50)]; # Generate 10 UUIDs

    # Cells
    volume = store.insertVolume(uids[0])
    face1 = store.insertFace(uids[1])
    face2 = store.insertFace(uids[2])
    edge1 = store.insertEdge(uids[3])
    edge2 = store.insertEdge(uids[4])
    vert1 = store.insertVertex(uids[5])
    vert2 = store.insertVertex(uids[6])
    model.addCell(volume)

    # Uses
    # The volume is used once by the model
    voluse = store.setVolumeUse(uids[7], volume)
    # Each face is used once by the volume. Exterior face-uses are not included.
    face1use = store.setFaceUse(uids[8], face1, 0, smtk.model.POSITIVE)
    face2use = store.setFaceUse(uids[9], face2, 0, smtk.model.POSITIVE)
    # Each edge is used twice (once by each face)
    edge1use1 = store.setEdgeUse(uids[10], edge1, 0, smtk.model.POSITIVE)
    edge2use1 = store.setEdgeUse(uids[11], edge2, 0, smtk.model.POSITIVE)
    edge1use2 = store.setEdgeUse(uids[12], edge1, 0, smtk.model.NEGATIVE)
    edge2use2 = store.setEdgeUse(uids[13], edge2, 0, smtk.model.NEGATIVE)
    # Each vertex has a single connected component attached to it, and thus has a single sense.
    vert1use = store.setVertexUse(uids[14], vert1, 0)
    vert2use = store.setVertexUse(uids[15], vert2, 0)

    # Shells
    # First, we create the shells (indicating their parent cell-use)
    shell1 = store.setShell(uids[16], voluse)
    loop1 = store.setLoop(uids[17], face1use)
    loop2 = store.setLoop(uids[18], face2use)
    # One can call store.setXXX(shellEntityId, parentUse) or equivalently
    # call parentUse.setBoundingShellEntity(store.insertXXX(shellEntityId)).
    # We demonstrate/test both below:
    chain1 = store.setChain(uids[19], edge1use1)
    chain2 = store.setChain(uids[20], edge2use1)
    chain3 = store.insertChain(uids[21])
    chain4 = store.insertChain(uids[22])
    edge1use2.setBoundingShellEntity(chain3)
    edge2use2.setBoundingShellEntity(chain4)
    # Now we add child cell-uses to each shell:
    shell1.addUse(face1use).addUse(face2use)
    loop1.addUse(edge1use1).addUse(edge2use1)
    loop2.addUse(edge1use2).addUse(edge2use2)
    chain1.addUse(vert1use).addUse(vert2use)
    chain2.addUse(vert2use).addUse(vert1use)
    chain3.addUse(vert2use).addUse(vert1use)
    chain4.addUse(vert1use).addUse(vert2use)

    # Add "unoriented" cell-cell relationships
    # Without these, boundaryEntities/bordantEntities will not work.
    # assignDefaultNames needs bordantEntities in order to determine
    # the owningModel for naming...
    store.findEntity(uids[0], True).pushRelation(uids[1]).pushRelation(uids[2])
    store.findEntity(uids[1], True).pushRelation(uids[0]).pushRelation(uids[3])
    store.findEntity(uids[2], True).pushRelation(uids[0]).pushRelation(uids[4])
    store.findEntity(uids[3], True).pushRelation(uids[1]).pushRelation(uids[5])
    store.findEntity(uids[4], True).pushRelation(uids[2]).pushRelation(uids[6])
    store.findEntity(uids[5], True).pushRelation(uids[3]).pushRelation(uids[4])
    store.findEntity(uids[6], True).pushRelation(uids[4]).pushRelation(uids[3])

    store.assignDefaultNames()
    print smtk.io.ExportJSON.fromModelManager(store)

    status = \
        len(vert1.edges()) != 2 or \
        any([vert not in vert1.edges() for vert in vert2.edges()]) or \
        edge1use1.boundingShellEntity().entity() != loop1.entity() or \
        edge1use2.boundingShellEntity().entity() != loop2.entity() or \
        edge2use1.boundingShellEntity().entity() != loop1.entity() or \
        edge2use2.boundingShellEntity().entity() != loop2.entity() or \
        loop1.face().entity() != face1.entity() or \
        loop2.face().entity() != face2.entity()

  except Exception, ex:
    print 'Exception:'

    exc_type, exc_obj, exc_tb = sys.exc_info()
    fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
    print
    print 'Exception: ', exc_type, fname, 'line', exc_tb.tb_lineno
    print
    print ex
    print
    status = True

  sys.exit(0 if not status else 1)
