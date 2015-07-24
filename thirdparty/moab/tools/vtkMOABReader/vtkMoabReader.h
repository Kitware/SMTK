#ifndef VTKMOABREADER_H
#define VTKMOABREADER_H

#include "vtkIOGeometryModule.h" // For export macro
#include "vtkMultiBlockDataSetAlgorithm.h"
#include "vtkNew.h" //needed for api signature

class vtkInformation;
class vtkInformationVector;

namespace smoab{ class Tag; class Interface; }

class vtkMoabReader : public vtkMultiBlockDataSetAlgorithm
{
public:
  static vtkMoabReader *New();
  vtkTypeMacro(vtkMoabReader,vtkMultiBlockDataSetAlgorithm)
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify file name of the MOAB mesh file.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

protected:
  vtkMoabReader();
  ~vtkMoabReader();

  int RequestInformation(vtkInformation *vtkNotUsed(request),
                         vtkInformationVector **vtkNotUsed(inputVector),
                         vtkInformationVector *outputVector);

  int RequestData(vtkInformation *vtkNotUsed(request),
                  vtkInformationVector **vtkNotUsed(inputVector),
                  vtkInformationVector *outputVector);
private:
  void CreateSubBlocks(vtkNew<vtkMultiBlockDataSet> &root,
                       smoab::Interface* interface,
                       smoab::Tag const* parentTag,
                       smoab::Tag const* extractTag=NULL);

  void ExtractShell(vtkNew<vtkMultiBlockDataSet> &root,
                       smoab::Interface* interface,
                       smoab::Tag const* parentTag);

  vtkMoabReader(const vtkMoabReader&);  // Not implemented.
  void operator=(const vtkMoabReader&);  // Not implemented.
  char* FileName;

};

#endif // VTKMOABREADER_H
