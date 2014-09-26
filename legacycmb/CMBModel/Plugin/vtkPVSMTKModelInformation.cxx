#include "vtkPVSMTKModelInformation.h"

#include "vtkClientServerStream.h"
#include "vtkDataObject.h"
#include "vtkDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkAlgorithmOutput.h"
#include "vtkAlgorithm.h"
#include "vtkCompositeDataIterator.h"

#include "vtkModelManagerWrapper.h"

vtkStandardNewMacro(vtkPVSMTKModelInformation);

//----------------------------------------------------------------------------
vtkPVSMTKModelInformation::vtkPVSMTKModelInformation()
{
}

//----------------------------------------------------------------------------
vtkPVSMTKModelInformation::~vtkPVSMTKModelInformation()
{
  this->UUID2BlockIdMap.clear();
}

//----------------------------------------------------------------------------
void vtkPVSMTKModelInformation::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkPVSMTKModelInformation::CopyFromObject(vtkObject* obj)
{
  this->UUID2BlockIdMap.clear();
  vtkModelManagerWrapper *modelwrapper = vtkModelManagerWrapper::SafeDownCast( obj );

  if (!modelwrapper)
    {
    vtkErrorMacro("Object is not a SMTK Model Wrapper!");
    return;
    }

  modelwrapper->GetUUID2BlockIdMap(this->UUID2BlockIdMap);
  this->BlockId2UUIDMap.clear();
  std::map<std::string, unsigned int>::iterator it =
    this->UUID2BlockIdMap.begin();
  for(; it != this->UUID2BlockIdMap.end(); ++it)
    {
    this->BlockId2UUIDMap[it->second] = it->first;
    }
}

//----------------------------------------------------------------------------
bool vtkPVSMTKModelInformation::GetBlockId(std::string uuid, unsigned int &bid)
{
  if(this->UUID2BlockIdMap.find(uuid) != this->UUID2BlockIdMap.end())
    {
    bid = this->UUID2BlockIdMap[uuid];
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
std::string vtkPVSMTKModelInformation::GetModelEntityId(unsigned int bid)
{
  if(this->BlockId2UUIDMap.find(bid) != this->BlockId2UUIDMap.end())
    {
    return this->BlockId2UUIDMap[bid];
    }
  return "";
}

//----------------------------------------------------------------------------
void vtkPVSMTKModelInformation::AddInformation(vtkPVInformation* info)
{
  vtkPVSMTKModelInformation *modelInfo =
    vtkPVSMTKModelInformation::SafeDownCast(info);
  if (modelInfo)
    {
    this->UUID2BlockIdMap.clear();
    this->UUID2BlockIdMap.insert(
      modelInfo->UUID2BlockIdMap.begin(), modelInfo->UUID2BlockIdMap.end());
    }
}
