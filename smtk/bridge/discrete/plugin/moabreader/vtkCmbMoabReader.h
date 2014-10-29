#ifndef __vtkCmbMoabReader_h
#define __vtkCmbMoabReader_h

#include "vtkPolyDataAlgorithm.h"

#include "vtkNew.h" //needed for api signature

class vtkInformation;
class vtkInformationVector;

namespace smoab{ class Tag; }

class vtkCmbMoabReader : public vtkPolyDataAlgorithm
{
public:
  static vtkCmbMoabReader *New();
  vtkTypeMacro(vtkCmbMoabReader,vtkPolyDataAlgorithm)
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Name of the file to be read.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

protected:
  vtkCmbMoabReader();
  ~vtkCmbMoabReader();

  int RequestData(vtkInformation *vtkNotUsed(request),
                  vtkInformationVector **vtkNotUsed(inputVector),
                  vtkInformationVector *outputVector);
private:
  char *FileName;

  vtkCmbMoabReader(const vtkCmbMoabReader&);  // Not implemented.
  void operator=(const vtkCmbMoabReader&);  // Not implemented.

};

#endif // __vtkCmbMoabReader_h
