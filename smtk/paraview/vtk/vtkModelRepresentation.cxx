#include "smtk/paraview/vtk/vtkModelRepresentation.h"

#include "smtk/model/Storage.h"
#include "smtk/model/Entity.h"

#include "vtkActor.h"
#include "vtkApplyColors.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkConvertSelection.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkNew.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkPoints.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkRenderView.h"
#include "vtkStringArray.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkTransformFilter.h"
#include "vtkViewTheme.h"

using namespace smtk::model;
vtkInstantiatorNewMacro(vtkModelRepresentation);

vtkCxxSetObjectMacro(vtkModelRepresentation,Actor,vtkActor);
vtkCxxSetObjectMacro(vtkModelRepresentation,ApplyColors,vtkApplyColors);
vtkCxxSetObjectMacro(vtkModelRepresentation,Mapper,vtkPolyDataMapper);
vtkCxxSetObjectMacro(vtkModelRepresentation,Transform,vtkTransformFilter);

vtkModelRepresentation* vtkModelRepresentation::New()
{
  VTK_STANDARD_NEW_BODY(vtkModelRepresentation);
}

vtkModelRepresentation::vtkModelRepresentation()
{
  this->Transform = NULL;
  this->ApplyColors = NULL;
  this->Mapper = NULL;
  this->Actor = NULL;
  this->SelectionMask = 0xffffff;

  vtkNew<vtkActor> act;
  vtkNew<vtkPolyDataMapper> map;
  vtkNew<vtkApplyColors> apc;
  vtkNew<vtkTransformFilter> tfm;

  this->SetApplyColors(apc.GetPointer());
  this->SetTransform(tfm.GetPointer());
  this->SetMapper(map.GetPointer());
  this->SetActor(act.GetPointer());

  this->Actor->SetMapper(this->Mapper);
  this->ApplyColors->SetInputConnection(this->Transform->GetOutputPort());
  this->Mapper->SetInputConnection(this->ApplyColors->GetOutputPort());

  this->Mapper->SetScalarModeToUseCellFieldData();
  this->Mapper->SelectColorArray("vtkApplyColors color");
  this->Mapper->SetScalarVisibility(true);

  // Apply default theme
  vtkNew<vtkViewTheme> theme;
  theme->SetCellOpacity(1.0);
  theme->SetLineWidth(3);
  theme->SetPointSize(4);
  this->ApplyViewTheme(theme.GetPointer());
}

vtkModelRepresentation::~vtkModelRepresentation()
{
  this->SetApplyColors(NULL);
  this->SetTransform(NULL);
  this->SetMapper(NULL);
  this->SetActor(NULL);
}

void vtkModelRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Actor: " << this->Actor << "\n";
  os << indent << "Mapper: " << this->Mapper << "\n";
  os << indent << "Transform: " << this->Transform << "\n";
}

/// Apply a theme to this representation.
void vtkModelRepresentation::ApplyViewTheme(vtkViewTheme* theme)
{
  this->Superclass::ApplyViewTheme(theme);

  this->ApplyColors->SetPointLookupTable(theme->GetPointLookupTable());
  this->ApplyColors->SetCellLookupTable(theme->GetCellLookupTable());

  this->ApplyColors->SetDefaultPointColor(theme->GetPointColor());
  this->ApplyColors->SetDefaultPointOpacity(theme->GetPointOpacity());
  this->ApplyColors->SetDefaultCellColor(theme->GetCellColor());
  this->ApplyColors->SetDefaultCellOpacity(theme->GetCellOpacity());
  this->ApplyColors->SetSelectedPointColor(theme->GetSelectedPointColor());
  //this->ApplyColors->SetSelectedPointOpacity(theme->GetSelectedPointOpacity());
  this->ApplyColors->SetSelectedCellColor(theme->GetSelectedCellColor());
  //this->ApplyColors->SetSelectedCellOpacity(theme->GetSelectedCellOpacity());
  this->ApplyColors->SetScalePointLookupTable(theme->GetScalePointLookupTable());
  this->ApplyColors->SetScaleCellLookupTable(theme->GetScaleCellLookupTable());

  float baseSize = static_cast<float>(theme->GetPointSize());
  float lineWidth = static_cast<float>(theme->GetLineWidth());
  this->Actor->GetProperty()->SetPointSize(baseSize);
  this->Actor->GetProperty()->SetLineWidth(lineWidth);

  // TODO: Enable labeling
  //this->VertexTextProperty->SetColor(theme->GetVertexLabelColor());
  //this->VertexTextProperty->SetLineOffset(-2*baseSize);
  //this->EdgeTextProperty->SetColor(theme->GetEdgeLabelColor());
}

/// Generate polydata from an smtk::model with tessellation information.
int vtkModelRepresentation::RequestData(
  vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inInfo),
  vtkInformationVector* vtkNotUsed(outInfo))
{
  this->Transform->SetInputConnection(0, this->GetInternalOutputPort());
  this->ApplyColors->SetInputConnection(1, this->GetInternalAnnotationOutputPort());
  return 1;
}

void vtkModelRepresentation::PrepareForRendering(vtkRenderView* view)
{
  this->Superclass::PrepareForRendering(view);

  this->Transform->SetTransform(view->GetTransform());
}

bool vtkModelRepresentation::AddToView(vtkView* view)
{
  vtkRenderView* rview = vtkRenderView::SafeDownCast(view);
  if (!rview)
    {
    vtkErrorMacro(<< "Cannot add to \"" << view->GetClassName() << "\"; must add to vtkRenderView.");
    return false;
    }
  rview->GetRenderer()->AddActor(this->Actor);
  return true;
}

bool vtkModelRepresentation::RemoveFromView(vtkView* view)
{
  vtkRenderView* rview = vtkRenderView::SafeDownCast(view);
  if (!rview)
    {
    vtkErrorMacro(<< "Cannot remove from \"" << view->GetClassName() << "\"; must remove from vtkRenderView.");
    return false;
    }
  rview->GetRenderer()->RemoveActor(this->Actor);
  return true;
}

vtkSelection* vtkModelRepresentation::ConvertSelection(vtkView* view, vtkSelection* selection)
{
  (void)view;
  /*
  cout << "Convert selection called\n";
  vtkIndent indent;
  selection->PrintSelf(cout, indent);
  return selection;
  */

  vtkSmartPointer<vtkSelection> propSelection =
    vtkSmartPointer<vtkSelection>::New();

  // Extract the selection for the right prop
  if (selection->GetNumberOfNodes() > 1)
    {
    for (unsigned int i = 0; i < selection->GetNumberOfNodes(); i++)
      {
      vtkSelectionNode* node = selection->GetNode(i);
      vtkProp* prop = vtkProp::SafeDownCast(
        node->GetProperties()->Get(vtkSelectionNode::PROP()));
      if (prop == this->Actor)
        {
        vtkSmartPointer<vtkSelectionNode> nodeCopy =
          vtkSmartPointer<vtkSelectionNode>::New();
        nodeCopy->ShallowCopy(node);
        nodeCopy->GetProperties()->Remove(vtkSelectionNode::PROP());
        propSelection->AddNode(nodeCopy);
        }
      }
    }
  else
    {
    propSelection->ShallowCopy(selection);
    }

  // Start with an empty selection
  vtkSelection* converted = vtkSelection::New();
  vtkSmartPointer<vtkSelectionNode> node = vtkSmartPointer<vtkSelectionNode>::New();
  node->SetContentType(this->SelectionType);
  node->SetFieldType(vtkSelectionNode::CELL);
  vtkSmartPointer<vtkIdTypeArray> empty =
    vtkSmartPointer<vtkIdTypeArray>::New();
  node->SetSelectionList(empty);
  // Convert to the correct type of selection
  if (this->GetInput())
    {
    vtkDataObject* obj = this->GetInput();
    if (obj)
      {
      vtkSelection* index = vtkConvertSelection::ToSelectionType(
        propSelection, obj, this->SelectionType,
        this->SelectionArrayNames);

      // If we have a model and pedigree Ids (UUIDs), then
      // subset the selected UUIDs using our SelectionMask:
      bool emptyResult = false;
      vtkSelectionNode* snode = index->GetNode(0);
      if (this->Model && snode && snode->GetContentType() == vtkSelectionNode::PEDIGREEIDS)
        {
        vtkStringArray* uuids = vtkStringArray::SafeDownCast(
          snode->GetSelectionData()->GetAbstractArray(0));
        if (uuids)
          {
          vtkIdType jj = 0;
          for (vtkIdType ii = 0; ii < uuids->GetNumberOfValues(); ++ii)
            {
            smtk::util::UUID uid(uuids->GetValue(ii));
            smtk::model::Entity* entity = this->Model->findEntity(uid);
            bool keepId = true;
            if (entity && (entity->entityFlags() & this->SelectionMask) == 0)
              {
              keepId = false;
              }
            if (keepId && jj < ii)
              {
              uuids->SetValue(jj, uid.toString());
              ++jj;
              }
            }
          uuids->SetNumberOfValues(jj);
          emptyResult = (jj == 0);
          }
        }

      // If anything ends up being selected, update our output:
      if (!emptyResult)
        {
        converted->AddNode(node);
        converted->ShallowCopy(index);
        index->Delete();
        }
      }
    }

  return converted;
}
