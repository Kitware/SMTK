//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/markup/ImageData.h"

#include "smtk/markup/Resource.h"
#include "smtk/markup/SequentialAssignedIds.h"

#include "smtk/resource/json/Helper.h"

#include "smtk/common/Paths.h"

#include "vtkImageData.h"
#include "vtkXMLImageDataReader.h"

using namespace smtk::string::literals; // for ""_token

namespace smtk
{
namespace markup
{

ImageData::~ImageData() = default;

void ImageData::initialize(const nlohmann::json& data, smtk::resource::json::Helper& helper)
{
  auto resource = helper.resourceAs<smtk::markup::Resource>();
  auto pointSpace = std::dynamic_pointer_cast<IdSpace>(resource->domains().find("points"_token));
  auto cellSpace = std::dynamic_pointer_cast<IdSpace>(resource->domains().find("cells"_token));
  auto jpid = data["point_ids"];
  auto jcid = data["cell_ids"];
  auto jprr = jpid["range"].get<AssignedIds::IdRange>();
  auto jcrr = jcid["range"].get<AssignedIds::IdRange>();
  auto jpnn = jpid["nature"].get<std::string>();
  auto jcnn = jcid["nature"].get<std::string>();
  IdNature pnat = natureEnumerant(jpnn);
  IdNature cnat = natureEnumerant(jcnn);

  m_pointIds =
    std::make_shared<smtk::markup::AssignedIds>(pointSpace, pnat, jprr[0], jprr[1], this);
  m_cellIds = std::make_shared<smtk::markup::AssignedIds>(cellSpace, cnat, jcrr[0], jcrr[1], this);

  // Fetch data from shape URL.
  this->incoming<arcs::URLsToData>().visit([this, &helper](const URL* url) {
    std::string location = url->location().data();
    if (smtk::common::Paths::isRelative(location))
    {
      // Relative paths must be relative to the resource's directory.
      location = smtk::common::Paths::canonical(
        location, smtk::common::Paths::directory(url->parentResource()->location()));
    }
    smtk::string::Token mimeType = url->type();
    helper.futures().emplace_back(
      smtk::resource::json::Helper::threadPool()([this, &helper, location, mimeType]() {
        switch (mimeType.id())
        {
          case "vtk/image"_hash:
          {
            vtkNew<vtkXMLImageDataReader> reader;
            reader->SetFileName(location.c_str());
            reader->Update();
            m_image = reader->GetOutput();
          }
          break;
          default:
            smtkErrorMacro(
              smtk::io::Logger::instance(),
              "Unsupported image format \"" << mimeType.data() << "\"");
            break;
        }
      }));
  });
}

std::unordered_set<Domain*> ImageData::domains() const
{
  std::unordered_set<Domain*> result;
  if (m_pointIds)
  {
    result.insert(m_pointIds->space().get());
  }
  if (m_cellIds)
  {
    result.insert(m_cellIds->space().get());
  }
  return result;
}

AssignedIds* ImageData::domainExtent(smtk::string::Token domainName) const
{
  if (m_cellIds && domainName == m_cellIds->space()->name())
  {
    return m_cellIds.get();
  }
  else if (m_pointIds && domainName == m_pointIds->space()->name())
  {
    return m_pointIds.get();
  }
  return nullptr;
}

void ImageData::assignedIds(std::vector<AssignedIds*>& assignments) const
{
  assignments.clear();
  assignments.push_back(m_pointIds.get());
  assignments.push_back(m_cellIds.get());
}

bool ImageData::setShapeData(vtkSmartPointer<vtkImageData> image, Superclass::ShapeOptions& options)
{
  bool didChange = false;
  if (image == m_image)
  {
    return didChange;
  }
  // Add/remove Field instances connected to this image
  // as needed match the new \a image:
  didChange |= this->updateChildren(image, options);

  // Assign the image and request new ID assignments if needed.
  auto numberOfPointsPrior = m_pointIds ? m_pointIds->size() : 0;
  auto numberOfCellsPrior = m_cellIds ? m_cellIds->size() : 0;

  m_image = image;
  didChange = true;

  // TODO: Find child Subset/Sideset nodes and adjust? We don't have enough
  //       context here to translate IDs in point/cell space because we don't
  //       know why the shapeData is being replaced.
  if (m_image)
  {
    auto* resource = dynamic_cast<Resource*>(this->parentResource());
    auto numberOfPoints = static_cast<std::size_t>(image->GetNumberOfPoints());
    auto numberOfCells = static_cast<std::size_t>(image->GetNumberOfCells());
    if (numberOfPoints > numberOfPointsPrior || numberOfCells > numberOfCellsPrior)
    {
      auto* self = const_cast<ImageData*>(this);
      auto assignedIdsCtor =
        [self](const std::shared_ptr<IdSpace>& domain, IdNature nature, IdType begin, IdType end) {
          auto assignment =
            std::make_shared<SequentialAssignedIds>(domain, nature, begin, end, self);
          // If we need to initialize index←→ID lookups, do it here.
          return assignment;
        };
      // NB: Unlike unstructured data, images generally "own" their points
      //     (unless we start supporting overlapping AMR).
      auto pointIds =
        resource->domains()
          .findAs<IdSpace>("points"_token)
          ->requestRange(IdNature::NonExclusive, numberOfPoints, InvalidId(), assignedIdsCtor);
      auto cellIds =
        resource->domains()
          .findAs<IdSpace>("cells"_token)
          ->requestRange(IdNature::Primary, numberOfCells, InvalidId(), assignedIdsCtor);
      // NB: Here is where we might reconcile old point/cell IDs in Subset or Sideset
      //     nodes to the new IDs allocated to us.
      m_pointIds = pointIds;
      m_cellIds = cellIds;
    }
    // TODO: Should we iterate over referential geometry and erase it?
    //       In some cases, operations might be able to preserve them.
  }
  return didChange;
}

bool ImageData::assign(
  const smtk::graph::Component::ConstPtr& source,
  smtk::resource::CopyOptions& options)
{
  bool ok = this->Superclass::assign(source, options);
  if (auto sourceImageData = std::dynamic_pointer_cast<const ImageData>(source))
  {
    ok &= this->copyAssignment(sourceImageData->pointIds(), m_pointIds);
    ok &= this->copyAssignment(sourceImageData->cellIds(), m_cellIds);
    ok &= this->copyData(sourceImageData->shapeData(), m_image, options);
  }
  else
  {
    ok = false;
  }
  return ok;
}

} // namespace markup
} // namespace smtk
