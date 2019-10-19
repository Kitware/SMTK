//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME PublicPointerDefs.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_PublicPointerDefs_h
#define __smtk_PublicPointerDefs_h

#include "smtk/SharedPtr.h"
#include "smtk/SystemConfig.h"
#include <functional>
#include <map>
#include <set>
#include <string>
#include <typeindex>
#include <vector>

namespace smtk
{
namespace common
{
class UUID;
class UUIDGenerator;
typedef std::set<UUID> UUIDs;
typedef std::vector<UUID> UUIDArray;
}

namespace resource
{
class PersistentObject;
class Resource;
class Component;
class Manager;
class Set;
}

namespace attribute
{
class Attribute;
class Resource;
class ComponentItem;
class ComponentItemDefinition;
class DateTimeItem;
class DateTimeItemDefinition;
class Definition;
class DirectoryItem;
class DirectoryItemDefinition;
class DoubleItem;
class DoubleItemDefinition;
class FileItem;
class FileItemDefinition;
class FileSystemItem;
class FileSystemItemDefinition;
class GroupItem;
class GroupItemDefinition;
class IntItem;
class IntItemDefinition;
class Item;
class ItemDefinition;
class ModelEntityItem;
class ModelEntityItemDefinition;
class ReferenceItem;
class ReferenceItemDefinition;
class ResourceItem;
class ResourceItemDefinition;
class StringItem;
class StringItemDefinition;
class ValueItem;
class ValueItemDefinition;
class VoidItem;
class VoidItemDefinition;
}

namespace operation
{
class Operation;
class Manager;
}

namespace extension
{
class qtSelectionManager;
typedef smtk::shared_ptr<smtk::extension::qtSelectionManager> qtSelectionManagerPtr;
}

namespace mesh
{
class Resource;
class Component;
class Interface;
class Allocator;
class BufferedCellAllocator;
class IncrementalAllocator;
class ConnectivityStorage;
class PointLocatorImpl;

namespace moab
{
class Interface;
}

namespace json
{
class Interface;
}
}

namespace model
{
class Arrangement;
class ArrangementHelper;
class ArrangementReference;
typedef std::vector<smtk::model::ArrangementReference> ArrangementReferences;
class AttributeListPhrase;
typedef std::vector<smtk::model::Arrangement> Arrangements;
class AuxiliaryGeometry;
typedef std::vector<smtk::model::AuxiliaryGeometry> AuxiliaryGeometries;
class Resource;
class Session;
class SessionRef;
typedef std::vector<smtk::model::SessionRef> SessionRefs;
class SessionIO;
class SessionIOJSON;
class CellEntity;
typedef std::vector<smtk::model::CellEntity> Cells;
typedef std::set<smtk::model::CellEntity> CellSet;
class Chain;
typedef std::vector<smtk::model::Chain> Chains;
class EntityRef;
typedef std::set<smtk::model::EntityRef> EntityRefs;
typedef std::vector<smtk::model::EntityRef> EntityRefArray;
class DefaultSession;
class DescriptivePhrase;
class Edge;
typedef std::vector<smtk::model::Edge> Edges;
typedef std::set<smtk::model::Edge> EdgeSet;
class EdgeUse;
typedef std::vector<smtk::model::EdgeUse> EdgeUses;
class EntityPhrase;
class EntityListPhrase;
class Face;
typedef std::vector<smtk::model::Face> Faces;
typedef std::set<smtk::model::Face> FaceSet;
class FaceUse;
typedef std::vector<smtk::model::FaceUse> FaceUses;
class GridInfo;
class GridInfo2D;
class GridInfo3D;
class Group;
class Instance;
typedef std::vector<smtk::model::Instance> Instances;
typedef std::set<smtk::model::Instance> InstanceSet;
class Loop;
typedef std::vector<smtk::model::Loop> Loops;
class Entity;
class MeshPhrase;
class MeshListPhrase;
class Model;
typedef std::vector<Model> Models;
class PropertyValuePhrase;
class PropertyListPhrase;
class Shell;
typedef std::vector<smtk::model::Shell> Shells;
class ShellEntity;
typedef std::vector<smtk::model::ShellEntity> ShellEntities;
class SimpleModelSubphrases;
class SubphraseGenerator;
class Tessellation;
class UseEntity;
typedef std::vector<smtk::model::UseEntity> UseEntities;
class Vertex;
typedef std::vector<smtk::model::Vertex> Vertices;
typedef std::set<smtk::model::Vertex> VertexSet;
class VertexUse;
typedef std::vector<smtk::model::VertexUse> VertexUses;
class Volume;
typedef std::vector<smtk::model::Volume> Volumes;
class VolumeUse;
typedef std::vector<smtk::model::VolumeUse> VolumeUses;
}

namespace view
{
class AvailableOperations;
class ComponentPhrase;
class DescriptivePhrase;
class PhraseContent;
class PhraseListContent;
class ComponentPhraseContent;
class ResourcePhraseContent;
class PhraseList;
class PhraseModel;
class ResourcePhrase;
class Selection;
class SubphraseGenerator;
class View;
class VisibilityContent;
}

namespace workflow
{
class OperationFilterSort;
}

namespace session
{
// These classes are in the SMTKRemote library, which
// is only built when SMTK_ENABLE_REMUS_SUPPORT is ON.
// However, we do not #ifdef these declarations since
// that would introduce a dependency on a generated
// header that could cause frequent recompilation.
namespace remote
{
class Session;
class RemusConnection;
class RemusConnections;
class RemusRPCWorker;
}
}

namespace simulation
{
class ExportSpec;
class UserData;
}

namespace io
{
class Logger;
typedef smtk::shared_ptr<smtk::io::Logger> LoggerPtr;
}

namespace project
{
class Manager;
class Project;
}

namespace resource
{
typedef smtk::shared_ptr<smtk::resource::Manager> ManagerPtr;
typedef smtk::weak_ptr<smtk::resource::Manager> WeakManagerPtr;
typedef smtk::shared_ptr<smtk::resource::PersistentObject> PersistentObjectPtr;
typedef smtk::shared_ptr<const smtk::resource::PersistentObject> ConstPersistentObjectPtr;
typedef smtk::weak_ptr<smtk::resource::PersistentObject> WeakPersistentObjectPtr;
typedef std::set<smtk::resource::PersistentObjectPtr> PersistentObjectSet;
typedef smtk::shared_ptr<smtk::resource::PersistentObject> PersistentObjectPtr;
typedef smtk::shared_ptr<smtk::resource::Resource> ResourcePtr;
typedef smtk::shared_ptr<smtk::resource::Component> ComponentPtr;
typedef smtk::weak_ptr<smtk::resource::Resource> WeakResourcePtr;
typedef smtk::shared_ptr<smtk::resource::Set> SetPtr;
typedef smtk::shared_ptr<const smtk::resource::Component> ConstComponentPtr;
typedef smtk::shared_ptr<const smtk::resource::Resource> ConstResourcePtr;
typedef smtk::shared_ptr<const smtk::resource::Set> ConstSetPtr;
typedef std::vector<PersistentObjectPtr> PersistentObjectArray;
typedef std::vector<ResourcePtr> ResourceArray;
typedef std::vector<ComponentPtr> ComponentArray;
typedef std::set<PersistentObjectPtr> PersistentObjectSet;
typedef std::set<ResourcePtr> ResourceSet;
typedef std::set<ComponentPtr> ComponentSet;
}

namespace operation
{
typedef smtk::shared_ptr<smtk::operation::Operation> OperationPtr;
typedef smtk::weak_ptr<smtk::operation::Operation> WeakOperationPtr;
typedef smtk::shared_ptr<smtk::operation::Manager> ManagerPtr;
typedef smtk::weak_ptr<smtk::operation::Manager> WeakManagerPtr;
}

namespace mesh
{
typedef smtk::shared_ptr<smtk::mesh::Resource> ResourcePtr;
typedef smtk::shared_ptr<const smtk::mesh::Resource> ConstResourcePtr;
typedef smtk::shared_ptr<smtk::mesh::Component> ComponentPtr;
typedef smtk::shared_ptr<smtk::mesh::Interface> InterfacePtr;
typedef smtk::shared_ptr<smtk::mesh::Allocator> AllocatorPtr;
typedef smtk::shared_ptr<smtk::mesh::BufferedCellAllocator> BufferedCellAllocatorPtr;
typedef smtk::shared_ptr<smtk::mesh::IncrementalAllocator> IncrementalAllocatorPtr;
typedef smtk::shared_ptr<smtk::mesh::ConnectivityStorage> ConnectivityStoragePtr;
typedef smtk::shared_ptr<smtk::mesh::PointLocatorImpl> PointLocatorImplPtr;

namespace moab
{
typedef smtk::shared_ptr<smtk::mesh::moab::Interface> InterfacePtr;
}

namespace json
{
typedef smtk::shared_ptr<smtk::mesh::json::Interface> InterfacePtr;
}
}

namespace model
{
// Model Related Pointer Classes
typedef smtk::shared_ptr<smtk::model::Session> SessionPtr;
typedef smtk::weak_ptr<smtk::model::Session> WeakSessionPtr;
typedef std::map<smtk::common::UUID, smtk::shared_ptr<smtk::model::Session> > UUIDsToSessions;
typedef smtk::shared_ptr<smtk::model::DefaultSession> DefaultSessionPtr;
typedef smtk::shared_ptr<smtk::model::SessionIO> SessionIOPtr;
typedef smtk::shared_ptr<smtk::model::SessionIOJSON> SessionIOJSONPtr;
typedef smtk::shared_ptr<smtk::model::DescriptivePhrase> DescriptivePhrasePtr;
typedef smtk::weak_ptr<smtk::model::DescriptivePhrase> WeakDescriptivePhrasePtr;
typedef smtk::shared_ptr<smtk::model::EntityPhrase> EntityPhrasePtr;
typedef smtk::shared_ptr<smtk::model::EntityListPhrase> EntityListPhrasePtr;
typedef smtk::shared_ptr<smtk::model::MeshPhrase> MeshPhrasePtr;
typedef smtk::shared_ptr<smtk::model::MeshListPhrase> MeshListPhrasePtr;
typedef smtk::shared_ptr<smtk::model::PropertyValuePhrase> PropertyValuePhrasePtr;
typedef smtk::shared_ptr<smtk::model::PropertyListPhrase> PropertyListPhrasePtr;
typedef smtk::shared_ptr<smtk::model::SimpleModelSubphrases> SimpleModelSubphrasesPtr;
typedef smtk::shared_ptr<smtk::model::SubphraseGenerator> SubphraseGeneratorPtr;
typedef smtk::shared_ptr<smtk::model::Resource> ResourcePtr;
typedef smtk::weak_ptr<smtk::model::Resource> WeakResourcePtr;
typedef smtk::shared_ptr<smtk::model::Entity> EntityPtr;
typedef smtk::weak_ptr<smtk::model::Entity> WeakEntityPtr;
typedef std::vector<smtk::model::EntityPtr> EntityArray;
typedef smtk::shared_ptr<smtk::model::Arrangement> ArrangementPtr;
typedef smtk::weak_ptr<smtk::model::Arrangement> WeakArrangementPtr;
typedef smtk::shared_ptr<smtk::model::Tessellation> TessellationPtr;
typedef smtk::weak_ptr<smtk::model::Tessellation> WeakTessellationPtr;

// class for making the analysis grid information available in SMTK
typedef smtk::shared_ptr<smtk::model::GridInfo> GridInfoPtr;
typedef smtk::shared_ptr<smtk::model::GridInfo2D> GridInfo2DPtr;
typedef smtk::shared_ptr<smtk::model::GridInfo3D> GridInfo3DPtr;
}

namespace attribute
{
// Attribute Related Pointer Classes
typedef smtk::shared_ptr<smtk::attribute::Definition> DefinitionPtr;
typedef smtk::shared_ptr<const smtk::attribute::Definition> ConstDefinitionPtr;
typedef smtk::weak_ptr<smtk::attribute::Definition> WeakDefinitionPtr;
typedef smtk::shared_ptr<smtk::attribute::Attribute> AttributePtr;
typedef smtk::shared_ptr<const smtk::attribute::Attribute> ConstAttributePtr;
typedef smtk::weak_ptr<smtk::attribute::Attribute> WeakAttributePtr;
typedef std::vector<smtk::attribute::AttributePtr> Attributes;

typedef smtk::shared_ptr<smtk::attribute::Item> ItemPtr;
typedef smtk::shared_ptr<const smtk::attribute::Item> ConstItemPtr;
typedef smtk::weak_ptr<smtk::attribute::Item> WeakItemPtr;
typedef smtk::shared_ptr<smtk::attribute::ItemDefinition> ItemDefinitionPtr;
typedef smtk::shared_ptr<const smtk::attribute::ItemDefinition> ConstItemDefinitionPtr;
typedef smtk::weak_ptr<smtk::attribute::ItemDefinition> WeakItemDefinitionPtr;

typedef smtk::shared_ptr<smtk::attribute::ValueItem> ValueItemPtr;
typedef smtk::shared_ptr<const smtk::attribute::ValueItem> ConstValueItemPtr;
typedef smtk::shared_ptr<smtk::attribute::ValueItemDefinition> ValueItemDefinitionPtr;

typedef smtk::shared_ptr<smtk::attribute::DateTimeItem> DateTimeItemPtr;
typedef smtk::shared_ptr<smtk::attribute::DateTimeItemDefinition> DateTimeItemDefinitionPtr;
typedef smtk::shared_ptr<smtk::attribute::DirectoryItem> DirectoryItemPtr;
typedef smtk::shared_ptr<smtk::attribute::DirectoryItemDefinition> DirectoryItemDefinitionPtr;
typedef smtk::shared_ptr<smtk::attribute::DoubleItem> DoubleItemPtr;
typedef smtk::shared_ptr<smtk::attribute::DoubleItemDefinition> DoubleItemDefinitionPtr;
typedef smtk::shared_ptr<smtk::attribute::FileItem> FileItemPtr;
typedef smtk::shared_ptr<smtk::attribute::FileItemDefinition> FileItemDefinitionPtr;
typedef smtk::shared_ptr<smtk::attribute::FileSystemItem> FileSystemItemPtr;
typedef smtk::shared_ptr<smtk::attribute::FileSystemItemDefinition> FileSystemItemDefinitionPtr;
typedef smtk::shared_ptr<smtk::attribute::GroupItem> GroupItemPtr;
typedef smtk::shared_ptr<smtk::attribute::GroupItemDefinition> GroupItemDefinitionPtr;
typedef smtk::shared_ptr<smtk::attribute::IntItem> IntItemPtr;
typedef smtk::shared_ptr<smtk::attribute::IntItemDefinition> IntItemDefinitionPtr;
typedef smtk::shared_ptr<smtk::attribute::StringItem> StringItemPtr;
typedef smtk::shared_ptr<smtk::attribute::StringItemDefinition> StringItemDefinitionPtr;
typedef smtk::shared_ptr<smtk::attribute::ModelEntityItem> ModelEntityItemPtr;
typedef smtk::shared_ptr<const smtk::attribute::ModelEntityItemDefinition>
  ConstModelEntityItemDefinitionPtr;
typedef smtk::shared_ptr<smtk::attribute::ModelEntityItemDefinition> ModelEntityItemDefinitionPtr;
typedef smtk::shared_ptr<smtk::attribute::VoidItem> VoidItemPtr;
typedef smtk::shared_ptr<smtk::attribute::VoidItemDefinition> VoidItemDefinitionPtr;
typedef smtk::shared_ptr<smtk::attribute::ReferenceItem> ReferenceItemPtr;
typedef smtk::shared_ptr<smtk::attribute::ReferenceItemDefinition> ReferenceItemDefinitionPtr;
typedef smtk::shared_ptr<smtk::attribute::ResourceItem> ResourceItemPtr;
typedef smtk::shared_ptr<smtk::attribute::ResourceItemDefinition> ResourceItemDefinitionPtr;
typedef smtk::shared_ptr<smtk::attribute::ComponentItem> ComponentItemPtr;
typedef smtk::shared_ptr<smtk::attribute::ComponentItemDefinition> ComponentItemDefinitionPtr;

typedef smtk::shared_ptr<const smtk::attribute::DateTimeItem> ConstDateTimeItemPtr;
typedef smtk::shared_ptr<const smtk::attribute::DateTimeItemDefinition>
  ConstDateTimeItemDefinitionPtr;
typedef smtk::shared_ptr<const smtk::attribute::DirectoryItem> ConstDirectoryItemPtr;
typedef smtk::shared_ptr<const smtk::attribute::DirectoryItemDefinition>
  ConstDirectoryItemDefinitionPtr;
typedef smtk::shared_ptr<const smtk::attribute::DoubleItem> ConstDoubleItemPtr;
typedef smtk::shared_ptr<const smtk::attribute::DoubleItemDefinition> ConstDoubleItemDefinitionPtr;
typedef smtk::shared_ptr<const smtk::attribute::FileItem> ConstFileItemPtr;
typedef smtk::shared_ptr<const smtk::attribute::FileItemDefinition> ConstFileItemDefinitionPtr;
typedef smtk::shared_ptr<const smtk::attribute::FileSystemItem> ConstFileSystemItemPtr;
typedef smtk::shared_ptr<const smtk::attribute::FileSystemItemDefinition>
  ConstFileSystemItemDefinitionPtr;
typedef smtk::shared_ptr<const smtk::attribute::GroupItem> ConstGroupItemPtr;
typedef smtk::shared_ptr<const smtk::attribute::GroupItemDefinition> ConstGroupItemDefinitionPtr;
typedef smtk::shared_ptr<const smtk::attribute::IntItem> ConstIntItemPtr;
typedef smtk::shared_ptr<const smtk::attribute::IntItemDefinition> ConstIntItemDefinitionPtr;
typedef smtk::shared_ptr<const smtk::attribute::StringItem> ConstStringItemPtr;
typedef smtk::shared_ptr<const smtk::attribute::StringItemDefinition> ConstStringItemDefinitionPtr;
typedef smtk::shared_ptr<const smtk::attribute::ModelEntityItem> ConstModelEntityItemPtr;
typedef smtk::shared_ptr<const smtk::attribute::ModelEntityItemDefinition>
  ConstModelEntityItemDefinitionPtr;
typedef smtk::shared_ptr<const smtk::attribute::ReferenceItem> ConstReferenceItemPtr;
typedef smtk::shared_ptr<const smtk::attribute::ReferenceItemDefinition>
  ConstReferenceItemDefinitionPtr;
typedef smtk::shared_ptr<const smtk::attribute::ResourceItem> ConstResourceItemPtr;
typedef smtk::shared_ptr<const smtk::attribute::ResourceItemDefinition>
  ConstResourceItemDefinitionPtr;
typedef smtk::shared_ptr<const smtk::attribute::ComponentItem> ConstComponentItemPtr;
typedef smtk::shared_ptr<const smtk::attribute::ComponentItemDefinition>
  ConstComponentItemDefinitionPtr;
typedef smtk::shared_ptr<const smtk::attribute::VoidItem> ConstVoidItemPtr;
typedef smtk::shared_ptr<const smtk::attribute::VoidItemDefinition> ConstVoidItemDefinitionPtr;

typedef smtk::shared_ptr<smtk::attribute::Resource> ResourcePtr;
typedef smtk::shared_ptr<const smtk::attribute::Resource> ConstResourcePtr;
typedef smtk::weak_ptr<smtk::attribute::Resource> WeakResourcePtr;
}

namespace view
{
typedef smtk::shared_ptr<smtk::view::AvailableOperations> AvailableOperationsPtr;
typedef smtk::weak_ptr<smtk::view::AvailableOperations> WeakAvailableOperationsPtr;
typedef smtk::shared_ptr<smtk::view::ComponentPhrase> ComponentPhrasePtr;
typedef smtk::weak_ptr<smtk::view::ComponentPhrase> WeakComponentPhrasePtr;
typedef smtk::shared_ptr<smtk::view::DescriptivePhrase> DescriptivePhrasePtr;
typedef smtk::weak_ptr<smtk::view::DescriptivePhrase> WeakDescriptivePhrasePtr;
typedef std::vector<smtk::view::DescriptivePhrasePtr> DescriptivePhrases;
typedef smtk::shared_ptr<smtk::view::PhraseList> PhraseListPtr;
typedef smtk::shared_ptr<smtk::view::PhraseModel> PhraseModelPtr;
typedef smtk::weak_ptr<smtk::view::PhraseModel> WeakPhraseModelPtr;
typedef smtk::shared_ptr<smtk::view::ResourcePhrase> ResourcePhrasePtr;
typedef smtk::weak_ptr<smtk::view::ResourcePhrase> WeakResourcePhrasePtr;
typedef smtk::shared_ptr<smtk::view::Selection> SelectionPtr;
typedef smtk::weak_ptr<smtk::view::Selection> WeakSelectionPtr;
typedef smtk::shared_ptr<smtk::view::SubphraseGenerator> SubphraseGeneratorPtr;
typedef smtk::weak_ptr<smtk::view::SubphraseGenerator> WeakSubphraseGeneratorPtr;
typedef smtk::shared_ptr<smtk::view::View> ViewPtr;
typedef smtk::weak_ptr<smtk::view::View> WeakViewPtr;
typedef smtk::shared_ptr<smtk::view::PhraseContent> PhraseContentPtr;
typedef smtk::shared_ptr<const smtk::view::PhraseContent> ConstPhraseContentPtr;
typedef smtk::shared_ptr<smtk::view::PhraseListContent> PhraseListContentPtr;
typedef smtk::shared_ptr<smtk::view::ComponentPhraseContent> ComponentPhraseContentPtr;
typedef smtk::shared_ptr<smtk::view::ResourcePhraseContent> ResourcePhraseContentPtr;
typedef smtk::shared_ptr<smtk::view::VisibilityContent> VisibilityContentPtr;
}

namespace workflow
{
typedef smtk::shared_ptr<smtk::workflow::OperationFilterSort> OperationFilterSortPtr;
typedef smtk::weak_ptr<smtk::workflow::OperationFilterSort> WeakOperationFilterSortPtr;
}

namespace session
{
namespace remote
{
typedef smtk::shared_ptr<Session> SessionPtr;
typedef smtk::shared_ptr<RemusConnection> RemusConnectionPtr;
typedef smtk::shared_ptr<RemusConnections> RemusConnectionsPtr;
typedef smtk::shared_ptr<RemusRPCWorker> RemusRPCWorkerPtr;
}
}
namespace simulation
{
//custom user data classes
typedef smtk::shared_ptr<smtk::simulation::ExportSpec> ExportSpecPtr;
typedef smtk::shared_ptr<smtk::simulation::UserData> UserDataPtr;
}

//special map and set typedefs for better safety with sets of weak pointers
//since sets of weak pointers can be dangerous.
namespace attribute
{
typedef std::set<attribute::WeakAttributePtr, smtk::owner_less<attribute::WeakAttributePtr> >
  WeakAttributePtrSet;
typedef std::set<attribute::WeakDefinitionPtr, smtk::owner_less<attribute::WeakDefinitionPtr> >
  WeakDefinitionPtrSet;
typedef std::set<attribute::WeakItemDefinitionPtr,
  smtk::owner_less<attribute::WeakItemDefinitionPtr> >
  WeakItemDefinitionPtrSet;
typedef std::set<attribute::WeakItemPtr, smtk::owner_less<attribute::WeakItemPtr> > WeakItemPtrSet;
}

namespace project
{
typedef smtk::shared_ptr<smtk::project::Manager> ManagerPtr;
typedef smtk::shared_ptr<smtk::project::Project> ProjectPtr;
}

// These are used internally by SMTK
namespace internal
{
template <typename T>
struct is_shared_ptr
{
  enum
  {
    type = false
  };
};
template <typename T>
struct is_shared_ptr<smtk::shared_ptr<T> >
{
  enum
  {
    type = true
  };
};

template <typename T, int Enabled = is_shared_ptr<T>::type>
struct shared_ptr_type
{
  typedef smtk::shared_ptr<T> SharedPointerType;
  typedef T RawPointerType;
};

template <typename T>
struct shared_ptr_type<T, true>
{
  typedef T SharedPointerType;
  typedef typename T::element_type RawPointerType;
};
}
}
#endif /* __smtk_PublicPointerDefs_h */
