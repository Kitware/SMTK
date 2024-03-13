//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
/// @file PublicPointerDefs.h Shared pointer typedefs for readable code.

#ifndef smtk_PublicPointerDefs_h
#define smtk_PublicPointerDefs_h

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
class Managers;
typedef std::shared_ptr<Managers> ManagersPtr;
class UUID;
class UUIDGenerator;
/// @see smtk::common::UUID
typedef std::set<smtk::common::UUID> UUIDs;
/// @see smtk::common::UUID
typedef std::vector<smtk::common::UUID> UUIDArray;
} // namespace common

namespace resource
{
class PersistentObject;
class Resource;
class Component;
class Manager;
class Set;
} // namespace resource

namespace geometry
{
class Backend;
class Geometry;
class Manager;
class Resource;
} // namespace geometry

namespace graph
{
class Component;
template<typename GraphTraits>
class Resource;
} // namespace graph

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
} // namespace attribute

namespace operation
{
class Operation;
class Manager;
} // namespace operation

namespace extension
{
class qtSelectionManager;
/// @see smtk::extension::qtSelectionManager
typedef smtk::shared_ptr<smtk::extension::qtSelectionManager> qtSelectionManagerPtr;
} // namespace extension

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
} // namespace mesh

namespace model
{
class Arrangement;
class ArrangementHelper;
class ArrangementReference;
/// @see smtk::model::ArrangementReference
typedef std::vector<smtk::model::ArrangementReference> ArrangementReferences;
class AttributeListPhrase;
/// @see smtk::model::Arrangement
typedef std::vector<smtk::model::Arrangement> Arrangements;
class AuxiliaryGeometry;
/// @see smtk::model::AuxiliaryGeometry
typedef std::vector<smtk::model::AuxiliaryGeometry> AuxiliaryGeometries;
class Resource;
class Session;
class SessionRef;
/// @see smtk::model::SessionRef
typedef std::vector<smtk::model::SessionRef> SessionRefs;
class SessionIO;
class SessionIOJSON;
class CellEntity;
/// @see smtk::model::CellEntity
typedef std::vector<smtk::model::CellEntity> Cells;
/// @see smtk::model::CellEntity
typedef std::set<smtk::model::CellEntity> CellSet;
class Chain;
/// @see smtk::model::Chain
typedef std::vector<smtk::model::Chain> Chains;
class EntityRef;
/// @see smtk::model::EntityRef
typedef std::set<smtk::model::EntityRef> EntityRefs;
/// @see smtk::model::EntityRef
typedef std::vector<smtk::model::EntityRef> EntityRefArray;
class DefaultSession;
class Edge;
/// @see smtk::model::Edge
typedef std::vector<smtk::model::Edge> Edges;
/// @see smtk::model::Edge
typedef std::set<smtk::model::Edge> EdgeSet;
class EdgeUse;
/// @see smtk::model::EdgeUse
typedef std::vector<smtk::model::EdgeUse> EdgeUses;
class EntityPhrase;
class EntityListPhrase;
class Face;
/// @see smtk::model::Face
typedef std::vector<smtk::model::Face> Faces;
/// @see smtk::model::Face
typedef std::set<smtk::model::Face> FaceSet;
class FaceUse;
/// @see smtk::model::FaceUse
typedef std::vector<smtk::model::FaceUse> FaceUses;
class GridInfo;
class GridInfo2D;
class GridInfo3D;
class Instance;
/// @see smtk::model::Instance
typedef std::vector<smtk::model::Instance> Instances;
/// @see smtk::model::Instance
typedef std::set<smtk::model::Instance> InstanceSet;
class Loop;
/// @see smtk::model::Loop
typedef std::vector<smtk::model::Loop> Loops;
class Entity;
class MeshPhrase;
class MeshListPhrase;
class Model;
/// @see smtk::model::Model
typedef std::vector<smtk::model::Model> Models;
class PropertyValuePhrase;
class PropertyListPhrase;
class Shell;
/// @see smtk::model::Shell
typedef std::vector<smtk::model::Shell> Shells;
class ShellEntity;
/// @see smtk::model::ShellEntity
typedef std::vector<smtk::model::ShellEntity> ShellEntities;
class SimpleModelSubphrases;
class Tessellation;
class UseEntity;
/// @see smtk::model::UseEntity
typedef std::vector<smtk::model::UseEntity> UseEntities;
class Vertex;
/// @see smtk::model::Vertex
typedef std::vector<smtk::model::Vertex> Vertices;
/// @see smtk::model::Vertex
typedef std::set<smtk::model::Vertex> VertexSet;
class VertexUse;
/// @see smtk::model::VertexUse
typedef std::vector<smtk::model::VertexUse> VertexUses;
class Volume;
/// @see smtk::model::Volume
typedef std::vector<smtk::model::Volume> Volumes;
class VolumeUse;
/// @see smtk::model::VolumeUse
typedef std::vector<smtk::model::VolumeUse> VolumeUses;
} // namespace model

namespace view
{
class AvailableOperations;
class Badge;
class ComponentPhraseContent;
class Configuration;
class DescriptivePhrase;
class Manager;
class PhraseContent;
class PhraseListContent;
class PhraseModel;
class ResourcePhraseContent;
class Selection;
class SubphraseGenerator;
} // namespace view

namespace workflow
{
class OperationFilterSort;
}

namespace simulation
{
class ExportSpec;
class UserData;
} // namespace simulation

namespace io
{
class Logger;
/// @see smtk::io::Logger
typedef smtk::shared_ptr<smtk::io::Logger> LoggerPtr;
} // namespace io

namespace project
{
class Manager;
class Project;

namespace old
{
class Manager;
class Project;
} // namespace old
} // namespace project

namespace task
{
class Adaptor;
class Manager;
class Task;
} // namespace task

namespace resource
{
/// @see smtk::resource::Manager
typedef smtk::shared_ptr<smtk::resource::Manager> ManagerPtr;
/// @see smtk::resource::Manager
typedef smtk::weak_ptr<smtk::resource::Manager> WeakManagerPtr;
/// @see smtk::resource::PersistentObject
typedef smtk::shared_ptr<smtk::resource::PersistentObject> PersistentObjectPtr;
/// @see smtk::resource::PersistentObject
typedef smtk::shared_ptr<const smtk::resource::PersistentObject> ConstPersistentObjectPtr;
/// @see smtk::resource::PersistentObject
typedef smtk::weak_ptr<smtk::resource::PersistentObject> WeakPersistentObjectPtr;
/// @see smtk::resource::PersistentObjectPtr
typedef std::set<smtk::resource::PersistentObjectPtr> PersistentObjectSet;
/// @see smtk::resource::PersistentObject
typedef smtk::shared_ptr<smtk::resource::PersistentObject> PersistentObjectPtr;
/// @see smtk::resource::Resource
typedef smtk::shared_ptr<smtk::resource::Resource> ResourcePtr;
/// @see smtk::resource::Component
typedef smtk::shared_ptr<smtk::resource::Component> ComponentPtr;
/// @see smtk::resource::Resource
typedef smtk::weak_ptr<smtk::resource::Resource> WeakResourcePtr;
/// @see smtk::resource::Set
typedef smtk::shared_ptr<smtk::resource::Set> SetPtr;
/// @see smtk::resource::Component
typedef smtk::shared_ptr<const smtk::resource::Component> ConstComponentPtr;
/// @see smtk::resource::Resource
typedef smtk::shared_ptr<const smtk::resource::Resource> ConstResourcePtr;
/// @see smtk::resource::Set
typedef smtk::shared_ptr<const smtk::resource::Set> ConstSetPtr;
/// @see PersistentObjectPtr
typedef std::vector<smtk::resource::PersistentObjectPtr> PersistentObjectArray;
/// @see ResourcePtr
typedef std::vector<smtk::resource::ResourcePtr> ResourceArray;
/// @see ComponentPtr
typedef std::vector<smtk::resource::ComponentPtr> ComponentArray;
/// @see PersistentObjectPtr
typedef std::set<smtk::resource::PersistentObjectPtr> PersistentObjectSet;
/// @see ResourcePtr
typedef std::set<smtk::resource::ResourcePtr> ResourceSet;
/// @see ComponentPtr
typedef std::set<smtk::resource::ComponentPtr> ComponentSet;
} // namespace resource

namespace geometry
{
/// @see smtk::geometry::Geometry
typedef std::unique_ptr<Geometry> GeometryPtr;
/// @see smtk::geometry::Manager
typedef std::shared_ptr<Manager> ManagerPtr;
typedef std::shared_ptr<const Manager> ConstManagerPtr;
/// @see smtk::geometry::Resource
typedef std::shared_ptr<Resource> ResourcePtr;
typedef std::shared_ptr<const Resource> ConstResourcePtr;
} // namespace geometry

namespace graph
{
typedef std::shared_ptr<Component> ComponentPtr;
template<typename GraphTraits>
using ResourcePtr = std::shared_ptr<Resource<GraphTraits>>;
} // namespace graph

namespace operation
{
/// @see smtk::operation::Operation
typedef smtk::shared_ptr<smtk::operation::Operation> OperationPtr;
/// @see smtk::operation::Operation
typedef smtk::weak_ptr<smtk::operation::Operation> WeakOperationPtr;
/// @see smtk::operation::Manager
typedef smtk::shared_ptr<smtk::operation::Manager> ManagerPtr;
/// @see smtk::operation::Manager
typedef smtk::weak_ptr<smtk::operation::Manager> WeakManagerPtr;
} // namespace operation

namespace mesh
{
/// @see smtk::mesh::Resource
typedef smtk::shared_ptr<smtk::mesh::Resource> ResourcePtr;
/// @see smtk::mesh::Resource
typedef smtk::shared_ptr<const smtk::mesh::Resource> ConstResourcePtr;
/// @see smtk::mesh::Component
typedef smtk::shared_ptr<smtk::mesh::Component> ComponentPtr;
/// @see smtk::mesh::Interface
typedef smtk::shared_ptr<smtk::mesh::Interface> InterfacePtr;
/// @see smtk::mesh::Allocator
typedef smtk::shared_ptr<smtk::mesh::Allocator> AllocatorPtr;
/// @see smtk::mesh::BufferedCellAllocator
typedef smtk::shared_ptr<smtk::mesh::BufferedCellAllocator> BufferedCellAllocatorPtr;
/// @see smtk::mesh::IncrementalAllocator
typedef smtk::shared_ptr<smtk::mesh::IncrementalAllocator> IncrementalAllocatorPtr;
/// @see smtk::mesh::ConnectivityStorage
typedef smtk::shared_ptr<smtk::mesh::ConnectivityStorage> ConnectivityStoragePtr;
/// @see smtk::mesh::PointLocatorImpl
typedef smtk::shared_ptr<smtk::mesh::PointLocatorImpl> PointLocatorImplPtr;

namespace moab
{
/// @see smtk::mesh::moab::Interface
typedef smtk::shared_ptr<smtk::mesh::moab::Interface> InterfacePtr;
} // namespace moab

namespace json
{
/// @see smtk::mesh::json::Interface
typedef smtk::shared_ptr<smtk::mesh::json::Interface> InterfacePtr;
} // namespace json
} // namespace mesh

namespace model
{
// Model Related Pointer Classes
/// @see smtk::model::Session
typedef smtk::shared_ptr<smtk::model::Session> SessionPtr;
/// @see smtk::model::Session
typedef smtk::weak_ptr<smtk::model::Session> WeakSessionPtr;
/// @see smtk::common::UUID, smtk::model::Session
typedef std::map<smtk::common::UUID, smtk::shared_ptr<smtk::model::Session>> UUIDsToSessions;
/// @see smtk::model::DefaultSession
typedef smtk::shared_ptr<smtk::model::DefaultSession> DefaultSessionPtr;
/// @see smtk::model::SessionIO
typedef smtk::shared_ptr<smtk::model::SessionIO> SessionIOPtr;
/// @see smtk::model::SessionIOJSON
typedef smtk::shared_ptr<smtk::model::SessionIOJSON> SessionIOJSONPtr;
/// @see smtk::model::EntityPhrase
// typedef smtk::shared_ptr<smtk::model::EntityPhrase> EntityPhrasePtr;
/// @see smtk::model::EntityListPhrase
// typedef smtk::shared_ptr<smtk::model::EntityListPhrase> EntityListPhrasePtr;
/// @see smtk::model::MeshPhrase
// typedef smtk::shared_ptr<smtk::model::MeshPhrase> MeshPhrasePtr;
/// @see smtk::model::MeshListPhrase
// typedef smtk::shared_ptr<smtk::model::MeshListPhrase> MeshListPhrasePtr;
/// @see smtk::model::PropertyValuePhrase
// typedef smtk::shared_ptr<smtk::model::PropertyValuePhrase> PropertyValuePhrasePtr;
/// @see smtk::model::PropertyListPhrase
// typedef smtk::shared_ptr<smtk::model::PropertyListPhrase> PropertyListPhrasePtr;
/// @see smtk::model::SimpleModelSubphrases
// typedef smtk::shared_ptr<smtk::model::SimpleModelSubphrases> SimpleModelSubphrasesPtr;
/// @see smtk::model::SubphraseGenerator
// typedef smtk::shared_ptr<smtk::model::SubphraseGenerator> SubphraseGeneratorPtr;
/// @see smtk::model::Resource
typedef smtk::shared_ptr<smtk::model::Resource> ResourcePtr;
/// @see smtk::model::Resource
typedef smtk::weak_ptr<smtk::model::Resource> WeakResourcePtr;
/// @see smtk::model::Entity
typedef smtk::shared_ptr<smtk::model::Entity> EntityPtr;
/// @see smtk::model::Entity
typedef smtk::weak_ptr<smtk::model::Entity> WeakEntityPtr;
/// @see smtk::model::EntityPtr
typedef std::vector<smtk::model::EntityPtr> EntityArray;
/// @see smtk::model::Arrangement
typedef smtk::shared_ptr<smtk::model::Arrangement> ArrangementPtr;
/// @see smtk::model::Arrangement
typedef smtk::weak_ptr<smtk::model::Arrangement> WeakArrangementPtr;
/// @see smtk::model::Tessellation
typedef smtk::shared_ptr<smtk::model::Tessellation> TessellationPtr;
/// @see smtk::model::Tessellation
typedef smtk::weak_ptr<smtk::model::Tessellation> WeakTessellationPtr;

// class for making the analysis grid information available in SMTK
/// @see smtk::model::GridInfo
typedef smtk::shared_ptr<smtk::model::GridInfo> GridInfoPtr;
/// @see smtk::model::GridInfo2D
// typedef smtk::shared_ptr<smtk::model::GridInfo2D> GridInfo2DPtr;
/// @see smtk::model::GridInfo3D
// typedef smtk::shared_ptr<smtk::model::GridInfo3D> GridInfo3DPtr;
} // namespace model

namespace attribute
{
// Attribute Related Pointer Classes
/// @see smtk::attribute::Definition
typedef smtk::shared_ptr<smtk::attribute::Definition> DefinitionPtr;
/// @see smtk::attribute::Definition
typedef smtk::shared_ptr<const smtk::attribute::Definition> ConstDefinitionPtr;
/// @see smtk::attribute::Definition
typedef smtk::weak_ptr<smtk::attribute::Definition> WeakDefinitionPtr;
/// @see smtk::attribute::Attribute
typedef smtk::shared_ptr<smtk::attribute::Attribute> AttributePtr;
/// @see smtk::attribute::Attribute
typedef smtk::shared_ptr<const smtk::attribute::Attribute> ConstAttributePtr;
/// @see smtk::attribute::Attribute
typedef smtk::weak_ptr<smtk::attribute::Attribute> WeakAttributePtr;
/// @see smtk::attribute::Attribute
typedef smtk::weak_ptr<const smtk::attribute::Attribute> ConstWeakAttributePtr;
/// @see smtk::attribute::AttributePtr
typedef std::vector<smtk::attribute::AttributePtr> Attributes;

/// @see smtk::attribute::Item
typedef smtk::shared_ptr<smtk::attribute::Item> ItemPtr;
/// @see smtk::attribute::Item
typedef smtk::shared_ptr<const smtk::attribute::Item> ConstItemPtr;
/// @see smtk::attribute::Item
typedef smtk::weak_ptr<smtk::attribute::Item> WeakItemPtr;
/// @see smtk::attribute::ItemDefinition
typedef smtk::shared_ptr<smtk::attribute::ItemDefinition> ItemDefinitionPtr;
/// @see smtk::attribute::ItemDefinition
typedef smtk::shared_ptr<const smtk::attribute::ItemDefinition> ConstItemDefinitionPtr;
/// @see smtk::attribute::ItemDefinition
typedef smtk::weak_ptr<smtk::attribute::ItemDefinition> WeakItemDefinitionPtr;

/// @see smtk::attribute::ValueItem
typedef smtk::shared_ptr<smtk::attribute::ValueItem> ValueItemPtr;
/// @see smtk::attribute::ValueItem
typedef smtk::shared_ptr<const smtk::attribute::ValueItem> ConstValueItemPtr;
/// @see smtk::attribute::ValueItemDefinition
typedef smtk::shared_ptr<smtk::attribute::ValueItemDefinition> ValueItemDefinitionPtr;

/// @see smtk::attribute::DateTimeItem
typedef smtk::shared_ptr<smtk::attribute::DateTimeItem> DateTimeItemPtr;
/// @see smtk::attribute::DateTimeItemDefinition
typedef smtk::shared_ptr<smtk::attribute::DateTimeItemDefinition> DateTimeItemDefinitionPtr;
/// @see smtk::attribute::DirectoryItem
typedef smtk::shared_ptr<smtk::attribute::DirectoryItem> DirectoryItemPtr;
/// @see smtk::attribute::DirectoryItemDefinition
typedef smtk::shared_ptr<smtk::attribute::DirectoryItemDefinition> DirectoryItemDefinitionPtr;
/// @see smtk::attribute::DoubleItem
typedef smtk::shared_ptr<smtk::attribute::DoubleItem> DoubleItemPtr;
/// @see smtk::attribute::DoubleItemDefinition
typedef smtk::shared_ptr<smtk::attribute::DoubleItemDefinition> DoubleItemDefinitionPtr;
/// @see smtk::attribute::FileItem
typedef smtk::shared_ptr<smtk::attribute::FileItem> FileItemPtr;
/// @see smtk::attribute::FileItemDefinition
typedef smtk::shared_ptr<smtk::attribute::FileItemDefinition> FileItemDefinitionPtr;
/// @see smtk::attribute::FileSystemItem
typedef smtk::shared_ptr<smtk::attribute::FileSystemItem> FileSystemItemPtr;
/// @see smtk::attribute::FileSystemItemDefinition
typedef smtk::shared_ptr<smtk::attribute::FileSystemItemDefinition> FileSystemItemDefinitionPtr;
/// @see smtk::attribute::GroupItem
typedef smtk::shared_ptr<smtk::attribute::GroupItem> GroupItemPtr;
typedef smtk::weak_ptr<smtk::attribute::GroupItem> WeakGroupItemPtr;
/// @see smtk::attribute::GroupItemDefinition
typedef smtk::shared_ptr<smtk::attribute::GroupItemDefinition> GroupItemDefinitionPtr;
/// @see smtk::attribute::IntItem
typedef smtk::shared_ptr<smtk::attribute::IntItem> IntItemPtr;
/// @see smtk::attribute::IntItemDefinition
typedef smtk::shared_ptr<smtk::attribute::IntItemDefinition> IntItemDefinitionPtr;
/// @see smtk::attribute::StringItem
typedef smtk::shared_ptr<smtk::attribute::StringItem> StringItemPtr;
/// @see smtk::attribute::StringItemDefinition
typedef smtk::shared_ptr<smtk::attribute::StringItemDefinition> StringItemDefinitionPtr;
/// @see smtk::attribute::ModelEntityItem
typedef smtk::shared_ptr<smtk::attribute::ModelEntityItem> ModelEntityItemPtr;
/// @see smtk::attribute::ModelEntityItemDefinition
typedef smtk::shared_ptr<const smtk::attribute::ModelEntityItemDefinition>
  ConstModelEntityItemDefinitionPtr;
/// @see smtk::attribute::ModelEntityItemDefinition
typedef smtk::shared_ptr<smtk::attribute::ModelEntityItemDefinition> ModelEntityItemDefinitionPtr;
/// @see smtk::attribute::VoidItem
typedef smtk::shared_ptr<smtk::attribute::VoidItem> VoidItemPtr;
/// @see smtk::attribute::VoidItemDefinition
typedef smtk::shared_ptr<smtk::attribute::VoidItemDefinition> VoidItemDefinitionPtr;
/// @see smtk::attribute::ReferenceItem
typedef smtk::shared_ptr<smtk::attribute::ReferenceItem> ReferenceItemPtr;
/// @see smtk::attribute::ReferenceItemDefinition
typedef smtk::shared_ptr<smtk::attribute::ReferenceItemDefinition> ReferenceItemDefinitionPtr;
/// @see smtk::attribute::ResourceItem
typedef smtk::shared_ptr<smtk::attribute::ResourceItem> ResourceItemPtr;
/// @see smtk::attribute::ResourceItemDefinition
typedef smtk::shared_ptr<smtk::attribute::ResourceItemDefinition> ResourceItemDefinitionPtr;
/// @see smtk::attribute::ComponentItem
typedef smtk::shared_ptr<smtk::attribute::ComponentItem> ComponentItemPtr;
/// @see smtk::attribute::ComponentItemDefinition
typedef smtk::shared_ptr<smtk::attribute::ComponentItemDefinition> ComponentItemDefinitionPtr;

/// @see smtk::attribute::DateTimeItem
typedef smtk::shared_ptr<const smtk::attribute::DateTimeItem> ConstDateTimeItemPtr;
/// @see smtk::attribute::DateTimeItemDefinition
typedef smtk::shared_ptr<const smtk::attribute::DateTimeItemDefinition>
  ConstDateTimeItemDefinitionPtr;
/// @see smtk::attribute::DirectoryItem
typedef smtk::shared_ptr<const smtk::attribute::DirectoryItem> ConstDirectoryItemPtr;
/// @see smtk::attribute::DirectoryItemDefinition
typedef smtk::shared_ptr<const smtk::attribute::DirectoryItemDefinition>
  ConstDirectoryItemDefinitionPtr;
/// @see smtk::attribute::DoubleItem
typedef smtk::shared_ptr<const smtk::attribute::DoubleItem> ConstDoubleItemPtr;
/// @see smtk::attribute::DoubleItemDefinition
typedef smtk::shared_ptr<const smtk::attribute::DoubleItemDefinition> ConstDoubleItemDefinitionPtr;
/// @see smtk::attribute::FileItem
typedef smtk::shared_ptr<const smtk::attribute::FileItem> ConstFileItemPtr;
/// @see smtk::attribute::FileItemDefinition
typedef smtk::shared_ptr<const smtk::attribute::FileItemDefinition> ConstFileItemDefinitionPtr;
/// @see smtk::attribute::FileSystemItem
typedef smtk::shared_ptr<const smtk::attribute::FileSystemItem> ConstFileSystemItemPtr;
/// @see smtk::attribute::FileSystemItemDefinition
typedef smtk::shared_ptr<const smtk::attribute::FileSystemItemDefinition>
  ConstFileSystemItemDefinitionPtr;
/// @see smtk::attribute::GroupItem
typedef smtk::shared_ptr<const smtk::attribute::GroupItem> ConstGroupItemPtr;
/// @see smtk::attribute::GroupItemDefinition
typedef smtk::shared_ptr<const smtk::attribute::GroupItemDefinition> ConstGroupItemDefinitionPtr;
/// @see smtk::attribute::IntItem
typedef smtk::shared_ptr<const smtk::attribute::IntItem> ConstIntItemPtr;
/// @see smtk::attribute::IntItemDefinition
typedef smtk::shared_ptr<const smtk::attribute::IntItemDefinition> ConstIntItemDefinitionPtr;
/// @see smtk::attribute::StringItem
typedef smtk::shared_ptr<const smtk::attribute::StringItem> ConstStringItemPtr;
/// @see smtk::attribute::StringItemDefinition
typedef smtk::shared_ptr<const smtk::attribute::StringItemDefinition> ConstStringItemDefinitionPtr;
/// @see smtk::attribute::ModelEntityItem
typedef smtk::shared_ptr<const smtk::attribute::ModelEntityItem> ConstModelEntityItemPtr;
/// @see smtk::attribute::ModelEntityItemDefinition
typedef smtk::shared_ptr<const smtk::attribute::ModelEntityItemDefinition>
  ConstModelEntityItemDefinitionPtr;
/// @see smtk::attribute::ReferenceItem
typedef smtk::shared_ptr<const smtk::attribute::ReferenceItem> ConstReferenceItemPtr;
/// @see smtk::attribute::ReferenceItemDefinition
typedef smtk::shared_ptr<const smtk::attribute::ReferenceItemDefinition>
  ConstReferenceItemDefinitionPtr;
/// @see smtk::attribute::ResourceItem
typedef smtk::shared_ptr<const smtk::attribute::ResourceItem> ConstResourceItemPtr;
/// @see smtk::attribute::ResourceItemDefinition
typedef smtk::shared_ptr<const smtk::attribute::ResourceItemDefinition>
  ConstResourceItemDefinitionPtr;
/// @see smtk::attribute::ComponentItem
typedef smtk::shared_ptr<const smtk::attribute::ComponentItem> ConstComponentItemPtr;
/// @see smtk::attribute::ComponentItemDefinition
typedef smtk::shared_ptr<const smtk::attribute::ComponentItemDefinition>
  ConstComponentItemDefinitionPtr;
/// @see smtk::attribute::VoidItem
typedef smtk::shared_ptr<const smtk::attribute::VoidItem> ConstVoidItemPtr;
/// @see smtk::attribute::VoidItemDefinition
typedef smtk::shared_ptr<const smtk::attribute::VoidItemDefinition> ConstVoidItemDefinitionPtr;

/// @see smtk::attribute::Resource
typedef smtk::shared_ptr<smtk::attribute::Resource> ResourcePtr;
/// @see smtk::attribute::Resource
typedef smtk::shared_ptr<const smtk::attribute::Resource> ConstResourcePtr;
/// @see smtk::attribute::Resource
typedef smtk::weak_ptr<smtk::attribute::Resource> WeakResourcePtr;
} // namespace attribute

namespace view
{
/// @see smtk::view::AvailableOperations
typedef smtk::shared_ptr<smtk::view::AvailableOperations> AvailableOperationsPtr;
/// @see smtk::view::AvailableOperations
typedef smtk::weak_ptr<smtk::view::AvailableOperations> WeakAvailableOperationsPtr;
/// @see smtk::view::Badge
typedef smtk::shared_ptr<smtk::view::Badge> BadgePtr;
/// @see smtk::view::DescriptivePhrase
typedef smtk::shared_ptr<smtk::view::DescriptivePhrase> DescriptivePhrasePtr;
/// @see smtk::view::DescriptivePhrase
typedef smtk::weak_ptr<smtk::view::DescriptivePhrase> WeakDescriptivePhrasePtr;
/// @see smtk::view::DescriptivePhrasePtr
typedef std::vector<smtk::view::DescriptivePhrasePtr> DescriptivePhrases;
/// @see smtk::view::Manager
typedef smtk::shared_ptr<smtk::view::Manager> ManagerPtr;
/// @see smtk::view::Manager
typedef smtk::weak_ptr<smtk::view::Manager> WeakManagerPtr;
/// @see smtk::view::PhraseModel
typedef smtk::shared_ptr<smtk::view::PhraseModel> PhraseModelPtr;
/// @see smtk::view::PhraseModel
typedef smtk::weak_ptr<smtk::view::PhraseModel> WeakPhraseModelPtr;
/// @see smtk::view::Selection
typedef smtk::shared_ptr<smtk::view::Selection> SelectionPtr;
/// @see smtk::view::Selection
typedef smtk::weak_ptr<smtk::view::Selection> WeakSelectionPtr;
/// @see smtk::view::SubphraseGenerator
typedef smtk::shared_ptr<smtk::view::SubphraseGenerator> SubphraseGeneratorPtr;
/// @see smtk::view::SubphraseGenerator
typedef smtk::weak_ptr<smtk::view::SubphraseGenerator> WeakSubphraseGeneratorPtr;
/// @see smtk::view::Configuration
typedef smtk::shared_ptr<smtk::view::Configuration> ConfigurationPtr;
/// @see smtk::view::Configuration
typedef smtk::weak_ptr<smtk::view::Configuration> WeakConfigurationPtr;
/// @see smtk::view::PhraseContent
typedef smtk::shared_ptr<smtk::view::PhraseContent> PhraseContentPtr;
/// @see smtk::view::PhraseContent
typedef smtk::shared_ptr<const smtk::view::PhraseContent> ConstPhraseContentPtr;
/// @see smtk::view::PhraseListContent
typedef smtk::shared_ptr<smtk::view::PhraseListContent> PhraseListContentPtr;
/// @see smtk::view::ComponentPhraseContent
typedef smtk::shared_ptr<smtk::view::ComponentPhraseContent> ComponentPhraseContentPtr;
/// @see smtk::view::ResourcePhraseContent
typedef smtk::shared_ptr<smtk::view::ResourcePhraseContent> ResourcePhraseContentPtr;
} // namespace view

namespace workflow
{
/// @see smtk::workflow::OperationFilterSort
typedef smtk::shared_ptr<smtk::workflow::OperationFilterSort> OperationFilterSortPtr;
/// @see smtk::workflow::OperationFilterSort
typedef smtk::weak_ptr<smtk::workflow::OperationFilterSort> WeakOperationFilterSortPtr;
} // namespace workflow

namespace simulation
{
//custom user data classes
/// @see smtk::simulation::ExportSpec
typedef smtk::shared_ptr<smtk::simulation::ExportSpec> ExportSpecPtr;
/// @see smtk::simulation::UserData
typedef smtk::shared_ptr<smtk::simulation::UserData> UserDataPtr;
} // namespace simulation

//special map and set typedefs for better safety with sets of weak pointers
//since sets of weak pointers can be dangerous.
namespace attribute
{
/// @see attribute::WeakAttributePtr
typedef std::set<attribute::WeakAttributePtr, smtk::owner_less<attribute::WeakAttributePtr>>
  WeakAttributePtrSet;
/// @see attribute::WeakDefinitionPtr
typedef std::set<attribute::WeakDefinitionPtr, smtk::owner_less<attribute::WeakDefinitionPtr>>
  WeakDefinitionPtrSet;
/// @see attribute::WeakItemDefinitionPtr
typedef std::
  set<attribute::WeakItemDefinitionPtr, smtk::owner_less<attribute::WeakItemDefinitionPtr>>
    WeakItemDefinitionPtrSet;
/// @see attribute::WeakItemPtr
typedef std::set<attribute::WeakItemPtr, smtk::owner_less<attribute::WeakItemPtr>> WeakItemPtrSet;
} // namespace attribute

namespace project
{
/// @see smtk::project::Manager
typedef smtk::shared_ptr<smtk::project::Manager> ManagerPtr;
typedef smtk::weak_ptr<smtk::project::Manager> WeakManagerPtr;
/// @see smtk::project::Project
typedef smtk::shared_ptr<smtk::project::Project> ProjectPtr;
typedef smtk::shared_ptr<const smtk::project::Project> ConstProjectPtr;
typedef smtk::weak_ptr<smtk::project::Project> WeakProjectPtr;
typedef smtk::weak_ptr<const smtk::project::Project> ConstWeakProjectPtr;
namespace old
{
/// @see smtk::project::Manager
typedef smtk::shared_ptr<smtk::project::old::Manager> ManagerPtr;
/// @see smtk::project::Project
typedef smtk::shared_ptr<smtk::project::old::Project> ProjectPtr;
} // namespace old
} // namespace project

namespace task
{
/// @see smtk::task::Adaptor
typedef smtk::shared_ptr<smtk::task::Adaptor> AdaptorPtr;
typedef smtk::shared_ptr<const smtk::task::Adaptor> ConstAdaptorPtr;
typedef smtk::weak_ptr<smtk::task::Adaptor> WeakAdaptorPtr;
typedef smtk::weak_ptr<const smtk::task::Adaptor> ConstWeakAdaptorPtr;
/// @see smtk::yask::Manager
typedef smtk::shared_ptr<smtk::task::Manager> ManagerPtr;
typedef smtk::weak_ptr<smtk::task::Manager> WeakManagerPtr;
/// @see smtk::task::Task
typedef smtk::shared_ptr<smtk::task::Task> TaskPtr;
typedef smtk::shared_ptr<const smtk::task::Task> ConstTaskPtr;
typedef smtk::weak_ptr<smtk::task::Task> WeakTaskPtr;
typedef smtk::weak_ptr<const smtk::task::Task> ConstWeakTaskPtr;
} // namespace task

// These are used internally by SMTK
namespace internal
{
template<typename T>
struct is_shared_ptr
{
  enum
  {
    type = false
  };
};
template<typename T>
struct is_shared_ptr<smtk::shared_ptr<T>>
{
  enum
  {
    type = true
  };
};

template<typename T, int Enabled = is_shared_ptr<T>::type>
struct shared_ptr_type
{
  typedef smtk::shared_ptr<T> SharedPointerType;
  typedef T RawPointerType;
};

template<typename T>
struct shared_ptr_type<T, true>
{
  typedef T SharedPointerType;
  typedef typename T::element_type RawPointerType;
};
} // namespace internal
} // namespace smtk
#endif /* smtk_PublicPointerDefs_h */
