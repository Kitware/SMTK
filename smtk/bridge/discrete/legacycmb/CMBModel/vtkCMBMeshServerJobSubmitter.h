//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkCMBMeshServerJobSubmitter
// .SECTION Description
// Takes a serialized job submission and remus client endpoint creates
// a connection to the remus server and submits the job. Once the job
//is submitted you can query for the relevant job id.

#ifndef __vtkCMBMeshServerJobSubmitter_h
#define __vtkCMBMeshServerJobSubmitter_h

#include "cmbSystemConfig.h"           //needed for warning suppression
#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkObject.h"
#include <string> //needed for member variables

class vtkDiscreteModelWrapper;

class VTKCMBDISCRETEMODEL_EXPORT vtkCMBMeshServerJobSubmitter : public vtkObject
{
public:
  //construction of this class will spawn
  //the CMBMeshServer
  vtkTypeMacro(vtkCMBMeshServerJobSubmitter, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkCMBMeshServerJobSubmitter* New();

  // Description:
  //Submits a job to the remus server, using the model we are operating
  //on as the input data
  //Will store the job id of the submitted job in LastSubmittedJobId
  //Sets OperateSucceeded.
  //Sets LastSubmittedJob.
  void Operate(vtkDiscreteModelWrapper* modelWrapper);

  // Description:
  // Returns success (1) or failure (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

  //After Operate is called we store the remus proto::job class, which can be
  //fetched by calling this method. The class it self when de-serialized will
  //set if it is valid or not.
  const char* GetLastSubmittedJob() const { return this->LastSubmittedJob.c_str(); }

  //Set the endpoint of the remus server that we need to connect too.
  //The form of the string should be "proto://hostname:port"
  void SetEndpoint(const char* endpoint);

  //Retrieve the EndPoint of the remus server that we will connect too
  //This must be a valid string before we call Operate.
  //The form of the string should be "proto://hostname:port"
  const char* GetEndpoint() const { return this->Endpoint.c_str(); }

  //Set the serialized remus::proto::Submission to be sent to the worker.
  //the submission that is being sent
  void SetSubmission(const char* submission);
  const char* GetSubmission() const { return this->Submission.c_str(); }

protected:
  vtkCMBMeshServerJobSubmitter();
  ~vtkCMBMeshServerJobSubmitter();

private:
  vtkCMBMeshServerJobSubmitter(const vtkCMBMeshServerJobSubmitter&); // Not implemented.
  void operator=(const vtkCMBMeshServerJobSubmitter&);               // Not implemented.

  std::string Endpoint;
  std::string Submission;
  std::string LastSubmittedJob;

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;
};

#endif
