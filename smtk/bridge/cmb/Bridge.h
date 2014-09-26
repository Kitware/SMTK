#ifndef __smtk_bridge_cmb_Bridge_h
#define __smtk_bridge_cmb_Bridge_h

#include "vtkCmbDiscreteModelModule.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/model/Bridge.h"

#include "smtk/util/UUID.h"
#include "smtk/util/UUIDGenerator.h"

#include "vtkSmartPointer.h"

#include <map>

class vtkAbstractArray;
class vtkDiscreteModel;
class vtkDiscreteModelEdge;
class vtkDiscreteModelEntityGroup;
class vtkDiscreteModelFace;
class vtkDiscreteModelGeometricEntity;
class vtkDiscreteModelRegion;
class vtkDiscreteModelVertex;
class vtkDiscreteModelWrapper;
class vtkInformation;
class vtkModel;
class vtkModelEdge;
class vtkModelEdgeUse;
class vtkModelEntity;
class vtkModelFace;
class vtkModelFaceUse;
class vtkModelGeometricEntity;
class vtkModelItem;
class vtkModelItemIterator;
class vtkModelLoopUse;
class vtkModelMaterial;
class vtkModelRegion;
class vtkModelShellUse;
class vtkModelVertex;
class vtkModelVertexUse;
class vtkUnsignedIntArray;

namespace smtk {
  namespace model {
    class GroupEntity;
  }
}

namespace smtk {
  namespace bridge {

  namespace cmb {

class vtkItemWatcherCommand;

/**\brief A class that handles translation between CMB and SMTK instances.
  *
  * How it works:
  * (1) A bridge entity is created and ImportEntitiesFromFileNameIntoManager
  *     is called to load a CMB model. A CMBModelReadOperator is used to load
  *     a vtkDiscreteModel (placed inside a vtkDiscreteModelWrapper by the
  *     CMB operator). The bridge instance is associated to the model wrapper
  *     in the s_modelsToBridges variable.
  * (2) ImportEntitiesFromFileNameIntoManager calls trackModel with the
  *     vtkDiscreteModelWrapper obtained using CMB's "read" model
  *     operator. Since vtkDiscreteModelWrapper is a subclass of vtkObject,
  *     we can keep it from being destroyed by holding a smart-pointer to
  *     it (in s_modelIdsToRefs and s_modelRefsToIds).
  * (2) The model and, upon demand, entities contained in the model
  *     are assigned UUIDs if not present already. The UUIDs are kept
  *     in the vtkInformation object every vtkModelItem owns (Properties).
  * (3) ExportSolid accepts a list of UUIDs for vtkModel instances.
  *     For each top-level vtkDiscreteModelWrapper in the list, it calls
  *     CMB's "write" operator on the model.
  */
class SMTKCMBBRIDGE_EXPORT Bridge : public smtk::model::Bridge
{
public:
  smtkTypeMacro(Bridge);
  smtkCreateMacro(smtk::model::Bridge);
  smtkSharedFromThisMacro(smtk::model::Bridge);
  smtkDeclareModelingKernel();

  typedef smtk::weak_ptr<Bridge> WeakPtr;
  typedef smtk::model::BridgedInfoBits BridgedInfoBits;
  virtual ~Bridge();

  virtual BridgedInfoBits allSupportedInformation() const;

  smtk::model::Cursor addCMBEntityToManager(
    const smtk::util::UUID& entity, smtk::model::ManagerPtr storage, int relDepth = 1);

  void assignUUIDs(const std::vector<vtkModelItem*>& ents, vtkAbstractArray* uuidArray);

  static vtkUnsignedIntArray* retrieveUUIDs(
    vtkDiscreteModel* model, const std::vector<vtkModelItem*>& ents);

  smtk::util::UUID ImportEntitiesFromFileNameIntoManager(
    const std::string& filename,
    const std::string& filetype,
    smtk::model::ManagerPtr storage);

  int ExportEntitiesToFileOfNameAndType(
    const std::vector<smtk::model::Cursor>& entities,
    const std::string& filename,
    const std::string& filetype);

  vtkDiscreteModelWrapper* findModel(const smtk::util::UUID& uid) const;

protected:
  friend class vtkItemWatcherCommand;
  friend class MergeOperator;
  friend class ReadOperator;
  friend class SplitFaceOperator;

  Bridge();

  virtual BridgedInfoBits transcribeInternal(
        const smtk::model::Cursor& entity, BridgedInfoBits requestedInfo);

  smtk::util::UUID trackModel(
    vtkDiscreteModelWrapper* mod, const std::string& url,
    smtk::model::ManagerPtr storage);
  bool assignUUIDToEntity(
    const smtk::util::UUID& itemId, vtkModelItem* item);
  smtk::util::UUID findOrSetEntityUUID(vtkModelItem* item);
  smtk::util::UUID findOrSetEntityUUID(vtkInformation* itemProperties);

  //static vtkDiscreteModel* owningModel(vtkModelItem* e);
  vtkModelItem* entityForUUID(const smtk::util::UUID& uid);

  smtk::model::Cursor addCMBEntityToManager(
    const smtk::util::UUID& entity, vtkModelItem* refEnt, smtk::model::ManagerPtr storage, int relDepth = 1);

  smtk::model::ModelEntity addBodyToManager(const smtk::util::UUID&, vtkModel*, smtk::model::ManagerPtr, int relDepth = 1);
  smtk::model::GroupEntity addGroupToManager(const smtk::util::UUID&, vtkDiscreteModelEntityGroup*, smtk::model::ManagerPtr, int relDepth = 1);
  smtk::model::GroupEntity addMaterialToManager(const smtk::util::UUID&, vtkModelMaterial*, smtk::model::ManagerPtr, int relDepth = 1);
  smtk::model::FaceUse addFaceUseToManager(const smtk::util::UUID&, vtkModelFaceUse*, smtk::model::ManagerPtr, int relDepth = 1);
  smtk::model::EdgeUse addEdgeUseToManager(const smtk::util::UUID&, vtkModelEdgeUse*, smtk::model::ManagerPtr, int relDepth = 1);
  smtk::model::VertexUse addVertexUseToManager(const smtk::util::UUID&, vtkModelVertexUse*, smtk::model::ManagerPtr, int relDepth = 1);
  smtk::model::Shell addShellToManager(const smtk::util::UUID&, vtkModelShellUse*, smtk::model::ManagerPtr, int relDepth = 1);
  smtk::model::Loop addLoopToManager(const smtk::util::UUID&, vtkModelLoopUse*, smtk::model::ManagerPtr, int relDepth = 1);
  smtk::model::Volume addVolumeToManager(const smtk::util::UUID&, vtkModelRegion*, smtk::model::ManagerPtr, int relDepth = 1);
  smtk::model::Face addFaceToManager(const smtk::util::UUID&, vtkModelFace*, smtk::model::ManagerPtr, int relDepth = 1);
  smtk::model::Edge addEdgeToManager(const smtk::util::UUID&, vtkModelEdge*, smtk::model::ManagerPtr, int relDepth = 1);
  smtk::model::Vertex addVertexToManager(const smtk::util::UUID&, vtkModelVertex*, smtk::model::ManagerPtr, int relDepth = 1);

  template<class P, typename H>
  void addEntities(P& parent, vtkModelItemIterator* it, const H& method, int relDepth);

  template<class P, typename C, typename H>
  void addEntityArray(P& parent, C& childContainer, const H& method, int relDepth);

  bool addTessellation(const smtk::model::Cursor& cellOut, vtkModelGeometricEntity* cellIn);
  bool addProperties(smtk::model::Cursor& cellOut, vtkModelItem* cellIn, smtk::model::BitFlags props = 0xff);

  vtkItemWatcherCommand* m_itemWatcher;
  smtk::util::UUIDGenerator m_idGenerator;
  std::map<smtk::util::UUID,vtkModelItem*> m_itemsToRefs;

  static std::map<vtkDiscreteModel*,WeakPtr> s_modelsToBridges;
  static std::map<smtk::util::UUID,vtkSmartPointer<vtkDiscreteModelWrapper> > s_modelIdsToRefs;
  static std::map<vtkSmartPointer<vtkDiscreteModelWrapper>, smtk::util::UUID> s_modelRefsToIds;
};

    } // namespace cmb
  } // namespace bridge

} // namespace smtk

#endif // __smtk_bridge_cmb_Bridge_h
