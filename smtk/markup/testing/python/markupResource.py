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
print(resource.domains())
print(resource.domains().keys())
