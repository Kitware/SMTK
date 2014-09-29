#include "MergeOperator.h"

#include "Bridge.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ModelEntityItem.h"

#include "smtk/model/ModelEntity.h"

#include "vtkModelItem.h"
#include "vtkModelEntity.h"

#include "MergeOperator_xml.h"

using namespace smtk::model;

namespace smtk {
  namespace bridge {

  namespace cmb {

MergeOperator::MergeOperator()
{
}

bool MergeOperator::ableToOperate()
{
  smtk::model::ModelEntity model;

  return
    this->ensureSpecification() &&
    // The SMTK model must be valid
    (model = this->specification()->findModelEntity("model")->value().as<smtk::model::ModelEntity>()).isValid() &&
    // The CMB model must exist:
    this->cmbBridge()->findModel(model.entity()) &&
    // The source and target cells must be valid:
    this->fetchCMBCellId("source cell") >= 0 &&
    this->fetchCMBCellId("target cell") >= 0
    ;
}

OperatorResult MergeOperator::operateInternal()
{
  Bridge* bridge = this->cmbBridge();

  // Translate SMTK inputs into CMB inputs
  this->m_op->SetSourceId(this->fetchCMBCellId("source cell"));
  this->m_op->SetTargetId(this->fetchCMBCellId("target cell"));

  vtkDiscreteModelWrapper* modelWrapper =
    bridge->findModel(
      this->specification()->findModelEntity("model")->value().entity());

  this->m_op->Operate(modelWrapper);
  bool ok = this->m_op->GetOperateSucceeded();
  OperatorResult result =
    this->createResult(
      ok ?  OPERATION_SUCCEEDED : OPERATION_FAILED);

  if (ok)
    {
  // TODO: Determine lower-dimensional boundary cells
  //       to mark as defunct?

  // TODO: Read list of boundary cells modified by the merge and
  //       use the bridge to update their translations; then store
  //       them in the OperatorResult (well, a subclass).
    }

  return result;
}

Bridge* MergeOperator::cmbBridge() const
{
  return dynamic_cast<Bridge*>(this->bridge());
}

int MergeOperator::fetchCMBCellId(const std::string& pname) const
{
  vtkModelItem* item =
    this->cmbBridge()->entityForUUID(
      this->specification()->findModelEntity(pname)->value().entity());

  vtkModelEntity* cell = dynamic_cast<vtkModelEntity*>(item);
  if (cell)
    return cell->GetUniquePersistentId();

  return -1;
}

    } // namespace cmb
  } // namespace bridge

} // namespace smtk

smtkImplementsModelOperator(
  smtk::bridge::cmb::MergeOperator,
  cmb_merge,
  "merge",
  MergeOperator_xml,
  smtk::bridge::cmb::Bridge);
