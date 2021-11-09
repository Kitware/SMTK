#!/usr/bin/env pvpython

# Edit this variable to hold the path to your modelbuilder or aevaCMB install.
import os
from paraview.simple import LoadPlugin
import smtk.attribute
import smtk.operation
import smtk.resource
import smtk.io
import smtk.common
import smtk.plugin
import smtk
appDir = '/path/to/superbuild/install'

# ++ 1 ++
# We need to import SMTK's python modules that expose
# python bindings to C++ objects:

# After importing, we can create manager instances.
rsrcMgr = smtk.resource.Manager.create()
opMgr = smtk.operation.Manager.create()
# The instances are initially empty:
print('ops:', len(opMgr.availableOperations()))
# -- 1 --

# ++ 2 ++
# Use ParaView's plugin infrastructure to initialize C++ functionality
# that may not be python wrapped but that can be used from Python via
# the wrapped API (such as resource types and operations).
smtkLibDir = os.path.join(appDir, 'lib', 'smtk-' +
                          smtk.common.Version.number())

# Load the plugins containing Registrars
LoadPlugin(os.path.join(smtkLibDir, 'smtkAttributePlugin/smtkAttributePlugin.so'))
LoadPlugin(os.path.join(smtkLibDir, 'smtkOperationPlugin/smtkOperationPlugin.so'))
# Have the plugins populate the operation manager with their operations.
smtk.plugin.registerPluginsTo(opMgr)
smtk.plugin.registerPluginsTo(rsrcMgr)
# Now we should see some operations. For my configuration, this prints "ops: 62".
print('ops:', len(opMgr.availableOperations()))
# -- 2 --

# ++ 3 ++
# Read an attribute resource using the resource manager:
fta = rsrcMgr.read('smtk::attribute::Resource',
                   '/path/to/fileTestAttribute.smtk')
allAttributes = fta.filter('*') if fta else []
print('Read %d attributes from file.' % len(allAttributes))
# -- 3 --

# ++ 4 ++
# It is also possible to use operations, either directly
# or via an operation group such as the Reader group.
importOp = opMgr.createOperation('smtk::attribute::Import')
importOp.parameters().findFile('filename').setValue(
    '/path/to/fileTestAttribute.sbt')
result = importOp.operate()
outcome = smtk.operation.Operation.Outcome(res.findInt('outcome').value(0))
if outcome == smtk.operation.Operation.SUCCEEDED:
    print('Imported file')
else:
    print('Operation failed with status %s' % outcome)
    print('Operation log:\n%s' % importOp.log().convertToString())
# -- 4 --
