/*=========================================================================

  Program:   ParaView
  Module:    $RCSfile: vtkCMBModelActor.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkCMBModelActor.h"
#include "vtkCMBModelMapper.h"
#include "vtkObjectFactory.h"
#include "vtkMapper.h"
#include "vtkMatrix4x4.h"
#include "vtkDataSet.h"
#include "vtkIntArray.h"
#include "vtkFieldData.h"
#include "vtkProperty.h"
#include "vtkOpenGL.h"
#include "vtkRenderer.h"
#include "vtkTexture.h"

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkCMBModelActor);

//----------------------------------------------------------------------------
vtkCMBModelActor::vtkCMBModelActor()
{
}

//----------------------------------------------------------------------------
vtkCMBModelActor::~vtkCMBModelActor()
{
}
//----------------------------------------------------------------------------
void vtkCMBModelActor::Render(vtkRenderer *ren, vtkMapper *vtkNotUsed(m))
{
  vtkMapper *mapper;

  if (this->Mapper == NULL)
    {
    vtkErrorMacro("No mapper for actor.");
    return;
    }

  mapper = this->SelectMapper();

  if (mapper == NULL)
    {
    return;
    }

  /* create the property */
  if (!this->Property)
    {
    // force creation of a property
    this->GetProperty();
    }

  this->PreModelRender(ren);
  mapper->Render(ren, this);
  this->PostModelRender(ren);
  this->EstimatedRenderTime = mapper->GetTimeToDraw();
}
//----------------------------------------------------------------------------
int vtkCMBModelActor::RenderOpaqueGeometry(vtkViewport *vp)
{
  int          renderedSomething = 0;
  vtkRenderer  *ren = static_cast<vtkRenderer*>(vp);

  if ( ! this->Mapper )
    {
    return 0;
    }
  vtkCMBModelMapper* mapper = vtkCMBModelMapper::SafeDownCast(this->Mapper);
  if(!mapper)
    {
    return 0;
    }

  this->Render(ren,this->Mapper);
  renderedSomething = 1;

  return renderedSomething;
}

//-----------------------------------------------------------------------------
void vtkCMBModelActor::PreModelRender(vtkRenderer* ren)
{
  vtkCMBModelMapper* mapper = vtkCMBModelMapper::SafeDownCast(this->Mapper);
  if(!mapper)
    {
    return;
    }
  // get opacity
  int opaque = this->GetIsOpaque();
  if (opaque == 1)
    {
    glDepthMask (GL_TRUE);
    }
  else
    {
    // add this check here for GL_SELECT mode
    // If we are not picking, then don't write to the zbuffer
    // because we probably haven't sorted the polygons. If we
    // are picking, then translucency doesn't matter - we want to
    // pick the thing closest to us.
    GLint param[1];
    glGetIntegerv(GL_RENDER_MODE, param);
    if(param[0] == GL_SELECT )
      {
      glDepthMask(GL_TRUE);
      }
    else
      {
      if(ren->GetLastRenderingUsedDepthPeeling())
        {
        glDepthMask(GL_TRUE); // transparency with depth peeling
        }
      else
        {
        glDepthMask (GL_FALSE); // transparency with alpha blending
        }
      }
    }

  // build transformation
  if (!this->IsIdentity)
    {
    double *mat = this->GetMatrix()->Element[0];
    double mat2[16];
    mat2[0] = mat[0];
    mat2[1] = mat[4];
    mat2[2] = mat[8];
    mat2[3] = mat[12];
    mat2[4] = mat[1];
    mat2[5] = mat[5];
    mat2[6] = mat[9];
    mat2[7] = mat[13];
    mat2[8] = mat[2];
    mat2[9] = mat[6];
    mat2[10] = mat[10];
    mat2[11] = mat[14];
    mat2[12] = mat[3];
    mat2[13] = mat[7];
    mat2[14] = mat[11];
    mat2[15] = mat[15];

    // insert model transformation
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glMultMatrixd(mat2);
    }
}


//-----------------------------------------------------------------------------
void vtkCMBModelActor::PostModelRender(vtkRenderer* ren)
{
  vtkCMBModelMapper* mapper = vtkCMBModelMapper::SafeDownCast(this->Mapper);
  if(!mapper)
    {
    return;
    }
  // get opacity
  int opaque = !this->GetIsOpaque();

  // pop transformation matrix
  if (!this->IsIdentity)
    {
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();
    }

  if (opaque != 1)
    {
    glDepthMask (GL_TRUE);
    }

  this->Property->PostRender(this, ren);
}

//----------------------------------------------------------------------------
void vtkCMBModelActor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
