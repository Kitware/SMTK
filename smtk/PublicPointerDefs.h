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
#include <map>
#include <set>
#include <vector>

//forward declare moab Interface class
namespace moab
{
  class Interface;
}

namespace smtk
{
  namespace common
  {
    class Resource;
    class ResourceSet;
    class UUID;
    class UUIDGenerator;
    class View;
    typedef std::set<UUID> UUIDs;
    typedef std::vector<UUID> UUIDArray;
  }

  namespace attribute
  {
    class Attribute;
    class RefItem;
    class RefItemDefinition;
    class Definition;
    class DirectoryItem;
    class DirectoryItemDefinition;
    class DoubleItem;
    class DoubleItemDefinition;
    class FileItem;
    class FileItemDefinition;
    class GroupItem;
    class GroupItemDefinition;
    class IntItem;
    class IntItemDefinition;
    class Item;
    class ItemDefinition;
    class System;
    class MeshSelectionItem;
    class MeshSelectionItemDefinition;
    class ModelEntityItem;
    class ModelEntityItemDefinition;
    class StringItem;
    class StringItemDefinition;
    class ValueItem;
    class ValueItemDefinition;
    class VoidItem;
    class VoidItemDefinition;
  }

  namespace mesh
  {
    class Manager;
    class Collection;
    namespace moab
    {
      //make the our moab interface class be the same as the moab::interface
      //We don't inherit from moab::interface since it is an abstract class
      typedef ::moab::Interface Interface;
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
    class Manager;
    class Session;
    class SessionRef;
    typedef std::vector<smtk::model::SessionRef> SessionRefs;
    class SessionIO;
    class SessionIOJSON;
    class CellEntity;
    class Chain;
    typedef std::vector<smtk::model::Chain> Chains;
    class EntityRef;
    typedef std::set<smtk::model::EntityRef> EntityRefs;
    typedef std::vector<smtk::model::EntityRef> EntityRefArray;
    class DefaultSession;
    class DescriptivePhrase;
    class Edge;
    typedef std::vector<smtk::model::Edge> Edges;
    class EdgeUse;
    typedef std::vector<smtk::model::EdgeUse> EdgeUses;
    class EntityPhrase;
    class EntityListPhrase;
    class Face;
    typedef std::vector<smtk::model::Face> Faces;
    class FaceUse;
    typedef std::vector<smtk::model::FaceUse> FaceUses;
    class GridInfo;
    class GridInfo2D;
    class GridInfo3D;
    class Group;
    class Instance;
    class Loop;
    typedef std::vector<smtk::model::Loop> Loops;
    class Entity;
    class Model;
    class Operator;
    class PropertyValuePhrase;
    class PropertyListPhrase;
    class RemoteOperator;
    class Shell;
    typedef std::vector<smtk::model::Shell> Shells;
    class ShellEntity;
    typedef std::vector<smtk::model::ShellEntity> ShellEntities;
    class Manager;
    class SimpleModelSubphrases;
    class SubphraseGenerator;
    class Tessellation;
    class UseEntity;
    typedef std::vector<smtk::model::UseEntity> UseEntities;
    class Vertex;
    typedef std::vector<smtk::model::Vertex> Vertices;
    class VertexUse;
    typedef std::vector<smtk::model::VertexUse> VertexUses;
    class Volume;
    typedef std::vector<smtk::model::Volume> Volumes;
    class VolumeUse;
    typedef std::vector<smtk::model::VolumeUse> VolumeUses;
  }

  namespace bridge
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
    class ExportJSON;
    class ImportJSON;
    class OperatorLog;
  }

  namespace common
  {
    typedef smtk::shared_ptr< smtk::common::Resource > ResourcePtr;
    typedef smtk::shared_ptr< smtk::common::View > ViewPtr;
  }

  namespace mesh
  {
    typedef smtk::shared_ptr< smtk::mesh::Manager >               ManagerPtr;
    typedef smtk::shared_ptr< smtk::mesh::Collection >            CollectionPtr;
    namespace moab
    {
      typedef smtk::shared_ptr< smtk::mesh::moab::Interface >     InterfacePtr;
    }

  }

  //Shiboken requires that we use fully qualified namespaces for all
  //types that these shared_ptr and weak_ptr are holding
  namespace model
  {
    // Model Related Pointer Classes
    typedef smtk::shared_ptr< smtk::model::Session >                SessionPtr;
    typedef std::map<smtk::common::UUID, smtk::shared_ptr< smtk::model::Session > > UUIDsToSessions;
    typedef smtk::shared_ptr< smtk::model::DefaultSession >         DefaultSessionPtr;
    typedef smtk::shared_ptr< smtk::model::SessionIO >              SessionIOPtr;
    typedef smtk::shared_ptr< smtk::model::SessionIOJSON >          SessionIOJSONPtr;
    typedef smtk::shared_ptr< smtk::model::DescriptivePhrase >     DescriptivePhrasePtr;
    typedef smtk::weak_ptr< smtk::model::DescriptivePhrase >       WeakDescriptivePhrasePtr;
    typedef smtk::shared_ptr< smtk::model::EntityPhrase >          EntityPhrasePtr;
    typedef smtk::shared_ptr< smtk::model::EntityListPhrase >      EntityListPhrasePtr;
    typedef smtk::shared_ptr< smtk::model::PropertyValuePhrase >   PropertyValuePhrasePtr;
    typedef smtk::shared_ptr< smtk::model::PropertyListPhrase >    PropertyListPhrasePtr;
    typedef smtk::shared_ptr< smtk::model::SimpleModelSubphrases > SimpleModelSubphrasesPtr;
    typedef smtk::shared_ptr< smtk::model::SubphraseGenerator >    SubphraseGeneratorPtr;
    typedef smtk::shared_ptr< smtk::model::Manager >               ManagerPtr;
    typedef smtk::weak_ptr< smtk::model::Manager >                 WeakManagerPtr;
    typedef smtk::shared_ptr< smtk::model::Operator >              OperatorPtr;
    typedef smtk::weak_ptr< smtk::model::Operator >                WeakOperatorPtr;
    typedef std::set< smtk::model::OperatorPtr >                   Operators;
    typedef smtk::shared_ptr< smtk::model::RemoteOperator >        RemoteOperatorPtr;
#ifndef SHIBOKEN_SKIP
    typedef smtk::model::OperatorPtr                             (*OperatorConstructor)();
    typedef std::pair<std::string,OperatorConstructor>             StaticOperatorInfo;
    typedef std::map<std::string,StaticOperatorInfo>               OperatorConstructors;
#endif
    typedef smtk::shared_ptr< smtk::model::Entity >                EntityPtr;
    typedef smtk::weak_ptr< smtk::model::Entity >                  WeakEntityPtr;
    typedef smtk::shared_ptr< smtk::model::Arrangement >           ArrangementPtr;
    typedef smtk::weak_ptr< smtk::model::Arrangement >             WeakArrangementPtr;
    typedef smtk::shared_ptr< smtk::model::Tessellation >          TessellationPtr;
    typedef smtk::weak_ptr< smtk::model::Tessellation >            WeakTessellationPtr;

    // class for making the analysis grid information available in SMTK
    typedef smtk::shared_ptr< smtk::model::GridInfo >          GridInfoPtr;
    typedef smtk::shared_ptr< smtk::model::GridInfo2D >        GridInfo2DPtr;
    typedef smtk::shared_ptr< smtk::model::GridInfo3D >        GridInfo3DPtr;

    // Model-related typedefs (dependent on attribute classes)
    typedef smtk::shared_ptr< smtk::attribute::Definition >    OperatorDefinition;
    typedef smtk::shared_ptr< smtk::attribute::Attribute >     OperatorSpecification;
    typedef smtk::shared_ptr< smtk::attribute::Attribute >     OperatorResult;
  }

  namespace attribute
  {
    // Attribute Related Pointer Classes
    typedef smtk::shared_ptr< smtk::attribute::Definition >       DefinitionPtr;
    typedef smtk::shared_ptr< const smtk::attribute::Definition > ConstDefinitionPtr;
    typedef smtk::weak_ptr< smtk::attribute::Definition >         WeakDefinitionPtr;
    typedef smtk::shared_ptr< smtk::attribute::Attribute >        AttributePtr;
    typedef smtk::weak_ptr< smtk::attribute::Attribute >          WeakAttributePtr;

    typedef smtk::shared_ptr< smtk::attribute::RefItem >           RefItemPtr;
    typedef smtk::shared_ptr< smtk::attribute::RefItemDefinition > RefItemDefinitionPtr;

    typedef smtk::shared_ptr< smtk::attribute::Item >                 ItemPtr;
    typedef smtk::shared_ptr< const smtk::attribute::Item >           ConstItemPtr;
    typedef smtk::weak_ptr< smtk::attribute::Item >                   WeakItemPtr;
    typedef smtk::shared_ptr< smtk::attribute::ItemDefinition >       ItemDefinitionPtr;
    typedef smtk::shared_ptr< const smtk::attribute::ItemDefinition > ConstItemDefinitionPtr;
    typedef smtk::weak_ptr< smtk::attribute::ItemDefinition >         WeakItemDefinitionPtr;

    typedef smtk::shared_ptr< smtk::attribute::ValueItem >            ValueItemPtr;
    typedef smtk::shared_ptr< const smtk::attribute::ValueItem >      ConstValueItemPtr;
    typedef smtk::shared_ptr< smtk::attribute::ValueItemDefinition >  ValueItemDefinitionPtr;

    typedef smtk::shared_ptr< smtk::attribute::DirectoryItem >            DirectoryItemPtr;
    typedef smtk::shared_ptr< smtk::attribute::DirectoryItemDefinition >  DirectoryItemDefinitionPtr;
    typedef smtk::shared_ptr< smtk::attribute::DoubleItem >               DoubleItemPtr;
    typedef smtk::shared_ptr< smtk::attribute::DoubleItemDefinition >     DoubleItemDefinitionPtr;
    typedef smtk::shared_ptr< smtk::attribute::FileItem >                 FileItemPtr;
    typedef smtk::shared_ptr< smtk::attribute::FileItemDefinition >       FileItemDefinitionPtr;
    typedef smtk::shared_ptr< smtk::attribute::GroupItem >                GroupItemPtr;
    typedef smtk::shared_ptr< smtk::attribute::GroupItemDefinition >      GroupItemDefinitionPtr;
    typedef smtk::shared_ptr< smtk::attribute::IntItem >                  IntItemPtr;
    typedef smtk::shared_ptr< smtk::attribute::IntItemDefinition >        IntItemDefinitionPtr;
    typedef smtk::shared_ptr< smtk::attribute::StringItem >               StringItemPtr;
    typedef smtk::shared_ptr< smtk::attribute::StringItemDefinition >     StringItemDefinitionPtr;
    typedef smtk::shared_ptr< smtk::attribute::MeshSelectionItem >           MeshSelectionItemPtr;
    typedef smtk::shared_ptr< smtk::attribute::MeshSelectionItemDefinition > MeshSelectionItemDefinitionPtr;
    typedef smtk::shared_ptr< smtk::attribute::ModelEntityItem >          ModelEntityItemPtr;
    typedef smtk::shared_ptr< smtk::attribute::ModelEntityItemDefinition > ModelEntityItemDefinitionPtr;
    typedef smtk::shared_ptr< smtk::attribute::VoidItem >                 VoidItemPtr;
    typedef smtk::shared_ptr< smtk::attribute::VoidItemDefinition >       VoidItemDefinitionPtr;

    typedef smtk::shared_ptr< const smtk::attribute::DirectoryItem >             ConstDirectoryItemPtr;
    typedef smtk::shared_ptr< const smtk::attribute::DirectoryItemDefinition >   ConstDirectoryItemDefinitionPtr;
    typedef smtk::shared_ptr< const smtk::attribute::DoubleItem >                ConstDoubleItemPtr;
    typedef smtk::shared_ptr< const smtk::attribute::DoubleItemDefinition >      ConstDoubleItemDefinitionPtr;
    typedef smtk::shared_ptr< const smtk::attribute::FileItem >                  ConstFileItemPtr;
    typedef smtk::shared_ptr< const smtk::attribute::FileItemDefinition >        ConstFileItemDefinitionPtr;
    typedef smtk::shared_ptr< const smtk::attribute::GroupItem >                 ConstGroupItemPtr;
    typedef smtk::shared_ptr< const smtk::attribute::GroupItemDefinition >       ConstGroupItemDefinitionPtr;
    typedef smtk::shared_ptr< const smtk::attribute::IntItem >                   ConstIntItemPtr;
    typedef smtk::shared_ptr< const smtk::attribute::IntItemDefinition >         ConstIntItemDefinitionPtr;
    typedef smtk::shared_ptr< const smtk::attribute::StringItem >                ConstStringItemPtr;
    typedef smtk::shared_ptr< const smtk::attribute::StringItemDefinition >      ConstStringItemDefinitionPtr;
    typedef smtk::shared_ptr< const smtk::attribute::MeshSelectionItem >            ConstMeshSelectionItemPtr;
    typedef smtk::shared_ptr< const smtk::attribute::MeshSelectionItemDefinition >  ConstMeshSelectionItemDefinitionPtr;
    typedef smtk::shared_ptr< const smtk::attribute::ModelEntityItem >           ConstModelEntityItemPtr;
    typedef smtk::shared_ptr< const smtk::attribute::ModelEntityItemDefinition > ConstModelEntityItemDefinitionPtr;
    typedef smtk::shared_ptr< const smtk::attribute::VoidItem >                  ConstVoidItemPtr;
    typedef smtk::shared_ptr< const smtk::attribute::VoidItemDefinition >        ConstVoidItemDefinitionPtr;
    typedef smtk::shared_ptr< const smtk::attribute::RefItem >                   ConstRefItemPtr;
    typedef smtk::shared_ptr< const smtk::attribute::RefItemDefinition >         ConstRefItemDefinitionPtr;

    // Note used by SMTK but added for completeness
    typedef smtk::shared_ptr< smtk::attribute::System >   SystemPtr;
  }

  namespace bridge
  {
    namespace remote
    {
      typedef smtk::shared_ptr< Session >              SessionPtr;
      typedef smtk::shared_ptr< RemusConnection >      RemusConnectionPtr;
      typedef smtk::shared_ptr< RemusConnections >     RemusConnectionsPtr;
      typedef smtk::shared_ptr< RemusRPCWorker >       RemusRPCWorkerPtr;
    }
  }
  namespace simulation
  {
    //custom user data classes
    typedef smtk::shared_ptr< smtk::simulation::ExportSpec > ExportSpecPtr;
    typedef smtk::shared_ptr< smtk::simulation::UserData > UserDataPtr;
  }

#ifdef smtk_has_owner_less
  //special map and set typedefs for better safety with sets of weak pointers
  //since sets of weak pointers can be dangerous.
  namespace attribute
  {
    typedef std::set< attribute::WeakAttributePtr,
      smtk::owner_less<attribute::WeakAttributePtr > >        WeakAttributePtrSet;
    typedef std::set< attribute::WeakDefinitionPtr,
      smtk::owner_less< attribute::WeakDefinitionPtr > >      WeakDefinitionPtrSet;
    typedef std::set< attribute::WeakItemDefinitionPtr,
      smtk::owner_less< attribute::WeakItemDefinitionPtr > >  WeakItemDefinitionPtrSet;
    typedef std::set< attribute::WeakItemPtr,
      smtk::owner_less< attribute::WeakItemPtr > >   WeakItemPtrSet;
  }

#else
  //we can use less than operator
  namespace attribute
  {
    typedef std::set< attribute::WeakAttributePtr  >      WeakAttributePtrSet;
    typedef std::set< attribute::WeakDefinitionPtr  >     WeakDefinitionPtrSet;
    typedef std::set< attribute::WeakItemDefinitionPtr >  WeakItemDefinitionPtrSet;
    typedef std::set< attribute::WeakItemPtr  >           WeakItemPtrSet;
  }

#endif

  // These are used internally by SMTK
  namespace internal
  {
    template <typename T >
    struct is_shared_ptr
    {
      enum {type=false};
    };
    template<typename T >
    struct is_shared_ptr< smtk::shared_ptr< T >  >
    {
      enum{type=true};
    };

    template<typename T, int Enabled = is_shared_ptr< T >::type  >
    struct shared_ptr_type
    {
      typedef smtk::shared_ptr< T > SharedPointerType;
      typedef T RawPointerType;
    };

    template<typename T >
    struct shared_ptr_type<T,true >
    {
      typedef T SharedPointerType;
      typedef typename T::element_type RawPointerType;
    };

  }
}
#endif /* __smtk_PublicPointerDefs_h */
