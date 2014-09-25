/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/
// .NAME vtkCMBMeshServerJobSubmitter
// .SECTION Description
// Takes a serialized job submission and remus client endpoint creates
// a connection to the remus server and submits the job. Once the job
//is submitted you can query for the relevant job id.

#ifndef __vtkCMBMeshServerJobSubmitter_h
#define __vtkCMBMeshServerJobSubmitter_h

#include "vtkObject.h"
#include "vtkCmbDiscreteModelModule.h" // For export macro
#include <string> //needed for member variables
#include "cmbSystemConfig.h" //needed for warning suppression

class vtkDiscreteModelWrapper;

class VTKCMBDISCRETEMODEL_EXPORT vtkCMBMeshServerJobSubmitter : public vtkObject
{
public:
  //construction of this class will spawn
  //the CMBMeshServer
  vtkTypeMacro(vtkCMBMeshServerJobSubmitter,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkCMBMeshServerJobSubmitter *New();

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
  vtkCMBMeshServerJobSubmitter(const vtkCMBMeshServerJobSubmitter&);  // Not implemented.
  void operator=(const vtkCMBMeshServerJobSubmitter&);  // Not implemented.

  std::string Endpoint;
  std::string Submission;
  std::string LastSubmittedJob;

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;
};

#endif
