#include "vtkPVSMTKModelInformation.h"

#include "vtkClientServerStream.h"
#include "vtkDataObject.h"
#include "vtkDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkAlgorithmOutput.h"
#include "vtkAlgorithm.h"
#include "vtkCompositeDataIterator.h"

#include "smtk/paraview/pvvtk/vtkSMTKModelReader.h"

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
  vtkSMTKModelReader *modelreader = vtkSMTKModelReader::SafeDownCast( obj );

  if (!modelreader)
    {
    vtkErrorMacro("Object is not a SMTK Model Reader!");
    return;
    }

  modelreader->GetEntityId2BlockIdMap(this->UUID2BlockIdMap);
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
