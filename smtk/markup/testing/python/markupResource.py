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

import smtk.graph
import smtk.markup

resource = smtk.markup.Resource.create()
resource.setName('markup')
print('Node Types\n  ' + '\n  '.join([xx.data()
      for xx in resource.nodeTypes()]))
print('Arc Types\n  ' + '\n  '.join([xx.data() for xx in resource.arcTypes()]))

expectedNodeset = set([
    'URL', 'UnstructuredData', 'Comment', 'Plane', 'Cone', 'Label', 'DiscreteGeometry',
    'Ontology', 'Field', 'AnalyticShape', 'SideSet', 'Box', 'OntologyIdentifier',
    'Subset', 'NodeSet', 'Landmark', 'SpatialData', 'ImageData', 'Component', 'Sphere',
    'Feature', 'Group'])
fqnsNodes = {smtk.string.Token('smtk::markup::' + xx)
             for xx in expectedNodeset}
if fqnsNodes != set(resource.nodeTypes()):
    print('Error: unexpected nodeset.')
    raise RuntimeError('Unexpected nodeset')

expectedArcset = set([
    'URLsToData', 'ReferencesToPrimaries', 'LabelsToSubjects', 'OntologyIdentifiersToSubtypes',
    'URLsToImportedData', 'OntologyIdentifiersToIndividuals', 'FieldsToShapes',
    'OntologyToIdentifiers', 'GroupsToMembers', 'BoundariesToShapes'])
fqnsArcs = {smtk.string.Token('smtk::markup::arcs::' + xx)
            for xx in expectedArcset}
if fqnsArcs != set(resource.arcTypes()):
    print('Error: unexpected arcset.')
    raise RuntimeError('Unexpected arcset')

# Create one node of every type
nodes = [resource.createNodeOfType(nodeTypeName)
         for nodeTypeName in resource.nodeTypes()]

# Create another node of every type, storing them in a dictionary keyed by their type.
# Note that "[14:]" removes the leading "smtk::markup::" from the type name.
moreNodes = {
    nodeTypeName.data()[14:]: resource.createNodeOfType(nodeTypeName)
    for nodeTypeName in resource.nodeTypes()
}

[smtk.markup.Component.CastTo(nodes[ii]).setName(
    'foo_' + str(ii)) for ii in range(len(nodes))]
for node in nodes:
    print(node.typeName(), node.name(), node)

for ntype, node in moreNodes.items():
    smtk.markup.Component.CastTo(node).setName('bar_' + ntype)

# Test connecting nodes with an arc
didConnect = resource.connect(
    moreNodes['Group'],
    moreNodes['UnstructuredData'],
    smtk.string.Token('smtk::markup::arcs::GroupsToMembers'))
if not didConnect:
    raise 'Should have connected group to its member.'


resource.dump('')
# print(resource.domains())
print('Domains\n  ' + '\n  '.join([xx.data()
      for xx in resource.domains().keys()]))

print('Test blanking')
nodeToBlank = resource.createNodeOfType(
    smtk.string.Token('smtk::markup::UnstructuredData'))
bbefore = nodeToBlank.isBlanked()
print('  Is blanked initially?', nodeToBlank.isBlanked())
nodeToBlank.setBlanking(True)
print('  Is blanked finally?', nodeToBlank.isBlanked())
bafter = nodeToBlank.isBlanked()
print('  Shape?', nodeToBlank.shape())
if bbefore or not bafter:
    raise ('Blanking incorrect')
