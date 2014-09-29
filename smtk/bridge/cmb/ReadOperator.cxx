#include "ReadOperator.h"

#include "Bridge.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/ModelEntityItem.h"

#include "smtk/model/Operator.h"
#include "smtk/model/ModelEntity.h"
#include "smtk/model/Manager.h"

#include "vtkDiscreteModelWrapper.h"
#include "vtkModelItem.h"
#include "vtkModelEntity.h"

#include "ReadOperator_xml.h"

using namespace smtk::model;

namespace smtk {
  namespace bridge {

  namespace cmb {

ReadOperator::ReadOperator()
{
}

bool ReadOperator::ableToOperate()
{
  return
    this->ensureSpecification()
    ;
}

OperatorResult ReadOperator::operateInternal()
{
  std::string fname = this->specification()->findFile("filename")->value();
  if (fname.empty())
    return this->createResult(OPERATION_FAILED);

  this->m_op->SetFileName(fname.c_str());

  // Create a new model to hold the result.
  vtkNew<vtkDiscreteModelWrapper> mod;
  this->m_op->Operate(mod.GetPointer());

  // Now assign a UUID to the model and associate its filename with
  // a URL property (if things went OK).
  if (!this->m_op->GetOperateSucceeded())
    {
    std::cerr << "Could not read file \"" << fname << "\".\n";
    return this->createResult(OPERATION_FAILED);
    }

  smtk::common::UUID modelId = this->cmbBridge()->trackModel(
    mod.GetPointer(), fname, this->manager());
  smtk::model::Cursor modelEntity(this->manager(), modelId);

  OperatorResult result = this->createResult(OPERATION_SUCCEEDED);
  result->findModelEntity("model")->setValue(modelEntity);

  return result;
}

Bridge* ReadOperator::cmbBridge() const
{
  return dynamic_cast<Bridge*>(this->bridge());
}

    } // namespace cmb
  } // namespace bridge

} // namespace smtk

smtkImplementsModelOperator(
  smtk::bridge::cmb::ReadOperator,
  cmb_read,
  "read",
  ReadOperator_xml,
  smtk::bridge::cmb::Bridge);
