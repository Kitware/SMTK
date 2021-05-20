//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkSBFunctionParser.h"

#include <vtkDoubleArray.h>
#include <vtkFunctionParser.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

#include <algorithm>
#include <map>
#include <sstream> // STL required.

vtkStandardNewMacro(vtkSBFunctionParser);

class vtkSBFunctionParser::vtkInternal
{
public:
  vtkInternal()
  {
    this->Parser = vtkSmartPointer<vtkFunctionParser>::New();

    this->DefineConstants();
  }

  void DefineConstants();

  vtkSmartPointer<vtkFunctionParser> Parser;

  std::map<std::string, double> Constants;
};

void vtkSBFunctionParser::vtkInternal::DefineConstants()
{
  this->Constants["PI"] = 3.1415926535;
}

vtkSBFunctionParser::vtkSBFunctionParser()
  : IndependentVariableName("X")
{
  this->Implementation = new vtkInternal();

  this->CheckMTime.Modified();
}

vtkSBFunctionParser::~vtkSBFunctionParser()
{
  if (this->Help)
  {
    delete[] this->Help;
    this->Help = nullptr;
  }
  if (this->Result)
  {
    this->Result->Delete();
    this->Result = nullptr;
  }

  if (this->Implementation)
  {
    delete this->Implementation;
    this->Implementation = nullptr;
  }
}

void vtkSBFunctionParser::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Function: " << this->Function << "\n";
  os << indent << "InitialValue: " << this->InitialValue << "\n";
  os << indent << "Delta: " << this->Delta << "\n";
  os << indent << "NumberOfValues: " << this->NumberOfValues << "\n";
  os << indent << "Help: " << (this->Help ? this->Help : "NULL") << "\n";
}

void vtkSBFunctionParser::SetFunction(std::string function)
{
  this->Function = function;
  this->Modified();
}

std::string vtkSBFunctionParser::GetFunction()
{
  return this->Function;
}

const char* vtkSBFunctionParser::GetHelp()
{
  if (!this->Help)
  {
    std::stringstream ss;
    ss << "Example Function: f(X) = cos(X).\n";
    ss << "(Note: Use capital X as variable!)\n";
    ss << "                                  \n";
    ss << "Standard constants available:\n";
    ss << "  PI = 3.1415926535\n";
    ss << "                                \n";
    ss << "Standard operations available:\n";
    ss << "  + - * / ^\n";
    ss << "                                \n";
    ss << "Standard functions available:\n";
    ss << "  abs acos asin atan ceil cos cosh\n";
    ss << "  exp floor log mag min max norm\n";
    ss << "  sign sin sinh sqrt tan tanh\n";
    this->Help = new char[ss.str().length() + 1];
    strcpy(this->Help, ss.str().c_str());
  }

  return this->Help;
}

void vtkSBFunctionParser::CheckExpression(int& pos, std::string& error)
{
  if (this->Function.empty())
  {
    error = std::string("Function is not set yet. First set the function.");
    return;
  }

  // Initialize only once.
  if (this->GetMTime() > this->CheckMTime.GetMTime())
  {
    this->Initialize();
  }

  // We still have to the check expression here as we are not storing
  // error and error position.
  char* parseError;
  this->Implementation->Parser->CheckExpression(pos, &parseError);

  if (parseError)
  {
    error = std::string(parseError);
  }

  // Now we update the modified time.
  this->CheckMTime.Modified();
}

vtkDoubleArray* vtkSBFunctionParser::GetResult()
{
  int pos;
  std::string err;

  this->CheckExpression(pos, err);

  if (!err.empty() || (pos != -1))
  {
    return nullptr;
  }

  this->Result = vtkDoubleArray::New();

  this->IsVectorResult = this->Implementation->Parser->IsVectorResult() != 0;
  if (this->IsVectorResult)
  {
    this->Result->SetNumberOfComponents(4);
  }
  else
  {
    this->Result->SetNumberOfComponents(2);
  }

  double indepVar = this->InitialValue;
  for (int i = 0; i < this->NumberOfValues; ++i)
  {
    // Since we know that we first set the independent variable
    // its index is 0. Since other method is not very efficient
    // where we set scalar value using name of independent variable.
    this->Implementation->Parser->SetScalarVariableValue(0, indepVar);

    if (this->IsVectorResult)
    {
      double* result = this->Implementation->Parser->GetVectorResult();
      this->Result->InsertNextTuple4(indepVar, result[0], result[1], result[2]);
    }
    else
    {
      this->Result->InsertNextTuple2(indepVar, this->Implementation->Parser->GetScalarResult());
    }

    indepVar += this->Delta;
  }

  return this->Result;
}

void vtkSBFunctionParser::Initialize()
{
  this->Implementation->Parser->SetFunction(this->Function.c_str());
  this->Implementation->Parser->SetScalarVariableValue(
    this->IndependentVariableName.c_str(), this->InitialValue);

  std::map<std::string, double>::const_iterator itr;

  for (itr = this->Implementation->Constants.begin(); itr != this->Implementation->Constants.end();
       ++itr)
  {
    this->Implementation->Parser->SetScalarVariableValue(itr->first.c_str(), itr->second);
  }
}
