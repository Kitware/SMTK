#include "vtkMOABReader.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkDataSetMapper.h"
#include "vtkActor.h"
#include "vtkProperty.h"
//#include "vtk.h"


int main(int argc, char **argv)
{
  if (argc < 2)
    {
      cout << "Usage: " << argv[0] << " <mesh_filename>" << endl;
    return 1;
    }

  vtkMOABReader* l = vtkMOABReader::New();

  l->SetFileName(argv[1]);
  l->Update();
  l->Print(std::cout);
  for (int i = 0; i < l->GetNumberOfOutputs(); i++) 
    l->GetOutput(i)->Print(std::cout);
  
  vtkRenderer *aRenderer = vtkRenderer::New();
  vtkRenderWindow *renWin = vtkRenderWindow::New();
    renWin->AddRenderer(aRenderer);
  vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::New();
    iren->SetRenderWindow(renWin);

  aRenderer->SetBackground(0,0,1);
  renWin->SetSize(640, 480);

  vtkDataSetMapper *meshMapper = vtkDataSetMapper::New();
  meshMapper->SetInput(l->GetOutput(1));
  vtkActor *meshActor = vtkActor::New();
  meshActor->SetMapper(meshMapper);
  meshActor->GetProperty()->SetColor(0.8, 0.1, 0.1);
  meshActor->GetProperty()->SetRepresentation(1);
  meshActor->GetProperty()->SetOpacity(0.1);
  meshActor->GetProperty()->SetEdgeVisibility(1);
  meshActor->GetProperty()->SetPointSize(3.0);
  meshActor->GetProperty()->SetLineWidth(3.0);
  aRenderer->AddActor(meshActor);
  
  // interact with data
  iren->Initialize();
  iren->Start(); 

  aRenderer->Delete();
  renWin->Delete();
  iren->Delete();

  return 0;
}
