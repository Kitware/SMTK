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
#BEGIN REMOVE ME FROM PACKAGE

def __bootstrap_smtk__():
  import sys, os

  shiboken_lib = "@SHIBOKEN_LIBRARY@"
  shiboken_path = os.path.dirname(shiboken_lib)
  smtk_path = "@LIBRARY_OUTPUT_PATH@"

  extra_paths = [shiboken_path, smtk_path]
  for (path, dirs, files) in os.walk(shiboken_path, topdown=False):
    #walk shiboken path and find the python/site-packages sub folders
    #add this to sys path
    x = [os.path.join(path,dir) for dir in dirs if "site-packages" in dir]
    if len(x) > 0:
      extra_paths = extra_paths + x
  sys.path = sys.path + extra_paths

__bootstrap_smtk__()

#END REMOVE ME FROM PACKAGE

def __import_shared_ptrs__():
  import re
  extract_name = re.compile("\W+")
  def __make_clean_name(name,obj):

    #known classes with the same name between the modules
    known_clashes = ("Manager", "Item")

    #split the name so we will have
    #[shared_ptr, smtk, model/atribute, name]
    split_name = [i for i in re.split(extract_name, name)]

    #assign the name
    new_name = split_name[3]
    #if the name is a known clashable name, extract the namespace
    #and make it a title
    if(new_name in known_clashes):
      new_name = split_name[2].title()+new_name

    #assign ptr to the end of the name
    return new_name+"Ptr"

  for i in _temp.__dict__:
    #I can't seem to find the type Shiboken.ObjectType, so we will hope
    #the inverse allways works
    if not isinstance(_temp.__dict__[i],(str)):
      try:
        name = __make_clean_name(i,_temp.__dict__[i])
        #now we are also going to
        globals()[name]  = _temp.__dict__[i]
      except:
        pass

#import the modules information we need.
#We are using _import__ since shiboken doesn't create proper python modules
#We can't just import smtkCorePython.smtk.model as model, so instead
#we have to use _import__ so that we get a nice interface.

import shiboken
_temp = __import__('smtkCorePython', globals(), locals(), [], -1)
__import_shared_ptrs__()

common = _temp.smtk.common
attribute = _temp.smtk.attribute
model = _temp.smtk.model
mesh = _temp.smtk.mesh
simulation = _temp.smtk.simulation
io = _temp.smtk.io

# Try loading optional extensions.
# Do not complain if they are not present.
try:
  import smtkRemusExtPy
except:
  pass

# Try importing bridge session modules. They may not be built, so don't complain on failure.
try:
  from collections import namedtuple
  btuple = []
  failed = []
  _tempmain = _temp
  try:
    _tempcgm = __import__('smtkCGMSessionPython', globals(), locals(), [], -1)
    _temp = _tempcgm
    __import_shared_ptrs__()
    btuple.append(('cgm', _tempcgm.cgm))
  except:
    failed += ['cgm']
  finally:
    _temp = _tempmain

  try:
    _tempexo = __import__('smtkExodusSessionPython', globals(), locals(), [], -1)
    _temp = _tempexo
    __import_shared_ptrs__()
    btuple.append(('exodus', _tempexo.exodus))
  except:
    failed += ['exodus']
  finally:
    _temp = _tempmain

  try:
    _tempdis = __import__('smtkDiscreteSessionPython', globals(), locals(), [], -1)
    _temp = _tempdis
    __import_shared_ptrs__()
    btuple.append(('discrete', _tempdis.discrete))
  except:
    failed += ['discrete']
  finally:
    _temp = _tempmain

  try:
    _tempdis = __import__('smtkPolygonSessionPython', globals(), locals(), [], -1)
    _temp = _tempdis
    __import_shared_ptrs__()
    btuple.append(('polygon', _tempdis.polygon))
  except:
    failed += ['polygon']
  finally:
    _temp = _tempmain

  try:
    _tempremote = __import__('smtkRemoteSessionPython', globals(), locals(), [], -1)
    _temp = _tempremote
    __import_shared_ptrs__()
    btuple.append(('remote', _tempremote.remote))
  except:
    failed += ['remote']
  finally:
    _temp = _tempmain

  if len(btuple) > 0:
    bridgeModule = namedtuple('bridgeModule', ' '.join([x for x,y in btuple]))
    bridge = bridgeModule(*[y for x,y in btuple])
  else:
    bridge = None
except:
  bridge = None


attribute.type_dict = { attribute.Item.ATTRIBUTE_REF: (attribute.RefItem, attribute.RefItemDefinition),
                        attribute.Item.DOUBLE: (attribute.DoubleItem, attribute.DoubleItemDefinition),
                        attribute.Item.GROUP: (attribute.GroupItem, attribute.GroupItemDefinition),
                        attribute.Item.INT: (attribute.IntItem, attribute.IntItemDefinition),
                        attribute.Item.STRING: (attribute.StringItem, attribute.StringItemDefinition),
                        attribute.Item.VOID: (attribute.VoidItem, attribute.VoidItemDefinition),
                        attribute.Item.FILE: (attribute.FileItem, attribute.FileItemDefinition),
                        attribute.Item.DIRECTORY: (attribute.DirectoryItem, attribute.DirectoryItemDefinition),
                        attribute.Item.COLOR: (None, None),
                        attribute.Item.MODEL_ENTITY: (attribute.ModelEntityItem, attribute.ModelEntityItemDefinition),
                        attribute.Item.MESH_SELECTION: (attribute.MeshSelectionItem, attribute.MeshSelectionItemDefinition)
                      }

@staticmethod
def __to_concrete__(item):
  '''
    Returns concrete (leaf) object for input, which is smtk.attribute.Item
  '''
  def fun(i):
    concrete_item = None
    for item_type,klass in attribute.type_dict.items():
      if i.type() == item_type:
        try:
          concrete_item = klass[0].CastTo(i)
          break
        except TypeError:
          concrete_item = klass[1].CastTo(i)
          break
    if concrete_item is None:
      print 'WARNING - unsupported type %s, item %s' % \
        (i.type(), i.name())
    return concrete_item
  if isinstance(item, list):
      return [ fun(x) for x in item ]
  elif isinstance(item, tuple):
      return tuple([ fun(x) for x in item ])
  elif isinstance(item, set):
      return set([fun(x) for x in item])
  return fun(item)

attribute.to_concrete = __to_concrete__
del __to_concrete__

def addItemDefinition( self, data_type, name):
  def_ = data_type.New(name)
  if def_ is None:
    print "could not create"
    return None
  idef = data_type.ToItemDefinition(def_)
  if idef is None:
    print "could not convert"
    return None
  if not self.addItemDefinition(idef):
    print "could not add"
    return None
  return def_

DefinitionPtr.addItemDefinitionStr = addItemDefinition
DirectoryItemDefinitionPtr.addItemDefinitionStr = addItemDefinition
DoubleItemDefinitionPtr.addItemDefinitionStr = addItemDefinition
FileItemDefinitionPtr.addItemDefinitionStr = addItemDefinition
GroupItemDefinitionPtr.addItemDefinitionStr = addItemDefinition
IntItemDefinitionPtr.addItemDefinitionStr = addItemDefinition
ItemDefinitionPtr.addItemDefinitionStr = addItemDefinition
RefItemDefinitionPtr.addItemDefinitionStr = addItemDefinition
StringItemDefinitionPtr.addItemDefinitionStr = addItemDefinition
MeshSelectionItemDefinitionPtr.addItemDefinitionStr = addItemDefinition
ModelEntityItemDefinitionPtr.addItemDefinitionStr = addItemDefinition
ValueItemDefinitionPtr.addItemDefinitionStr = addItemDefinition
VoidItemDefinitionPtr.addItemDefinitionStr = addItemDefinition

import inspect

def _Debug( self, message ):
  cs = inspect.stack()
  at = 1
  if len(cs) < 1:
    at = 0
  self.addRecord(io.Logger.DEBUG, str(message), cs[at][1],  cs[at][2])

def _Error( self, message ):
  cs = inspect.stack()
  at = 1
  if len(cs) < 1:
    at = 0
  self.addRecord(io.Logger.ERROR, str(message), cs[at][1],  cs[at][2])

def _Warn( self, message ):
  cs = inspect.stack()
  at = 1
  if len(cs) < 1:
    at = 0
  self.addRecord(io.Logger.WARNING, str(message), cs[at][1],  cs[at][2])

io.Logger.addDebug = _Debug
io.Logger.addWarning = _Warn
io.Logger.addError = _Error

del _Debug
del _Warn
del _Error


def to_concrete(instance):
  '''General type converter for smtk objects
  '''
  # Use current smtk.attribute.to_concrete() for its types
  if instance.__class__.__module__.endswith('smtk.attribute') or \
    str(instance.__class__).find('smtk::attribute::') > 0:
      return attribute.to_concrete(instance)
  # (else)
  print 'no converter available for class %s, cannot convert' % \
      str(instance.__class__)
  print 'module', instance.__class__.__module__
  print 'class', instance.__class__
  return None
