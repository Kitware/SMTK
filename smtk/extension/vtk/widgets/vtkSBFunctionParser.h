//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __vtkSBFunctionParser_h
#define __vtkSBFunctionParser_h

#include <vtkObject.h>
#include <vtkTimeStamp.h>

// STL includes.
#include <string> // STL required.

#include "smtk/extension/vtk/widgets/vtkSMTKWidgetsExtModule.h" // For export macro

// Forware declarations.
class vtkDoubleArray;

class VTKSMTKWIDGETSEXT_EXPORT vtkSBFunctionParser : public vtkObject
{
public:
  // Description:
  // Macro to determine if a class is same class or a subclass of same class.
  vtkTypeMacro(vtkSBFunctionParser, vtkObject);

  vtkSBFunctionParser(const vtkSBFunctionParser&) = delete;
  vtkSBFunctionParser& operator=(const vtkSBFunctionParser&) = delete;

  // Description:
  // Function to create new instance of this class.
  static vtkSBFunctionParser* New();

  // Description:
  // Print default values of the member variables.
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Set / Get the function. This could be a vector funtion or scalar function.
  // Such as sin(X) or sin(X)*iHat + cos(X)*jHat + tan(X)*kHat.
  void SetFunction(std::string function);
  std::string GetFunction();

  // Description:
  // Check if the result is vector or scalar.
  vtkGetMacro(IsVectorResult, bool);

  // Description:
  // Set / Get the initial value of the dependent variable.
  vtkSetMacro(InitialValue, double);
  vtkGetMacro(InitialValue, double);

  // Description:
  // Set / Get the delta by which to change the initial value. It could
  // be incremental or decremental depending upon the sign of the \c Delta.
  vtkSetMacro(Delta, double);
  vtkGetMacro(Delta, double);

  // Description:
  // Set a request to generate a specific number of values.
  vtkSetMacro(NumberOfValues, int);
  vtkGetMacro(NumberOfValues, int);

  // Description:
  // Return information on how to use this class, on constants etc.
  const char* GetHelp();

  // Description:
  // Query if the function has valid syntax.
  void CheckExpression(int& pos, std::string& error);

  // Description:
  // Return the result of the evaluation of the function. This could
  // be an array of two or 4 components. If the result if vector
  // then it would have 4 components, input, output*iHat, output2*jHat, output3*kHat.
  // Else it will have 2 components, input and output.
  vtkDoubleArray* GetResult();

protected:
  // Description:
  // Constructor / Destructor.
  vtkSBFunctionParser();
  ~vtkSBFunctionParser() override;

  // Description:
  void Initialize();

private:
  const std::string IndependentVariableName;

  std::string Function;

  bool IsVectorResult{ false };
  double InitialValue{ 0.0 };
  double Delta{ 0.0 };
  int NumberOfValues{ -1 };

  char* Help{ nullptr };
  vtkDoubleArray* Result{ nullptr };

  class vtkInternal;
  vtkInternal* Implementation;

  vtkTimeStamp CheckMTime;
};

#endif // __vtkSBFunctionParser_h
