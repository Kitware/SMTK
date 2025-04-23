//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/io/Logger.h"

// #include "smtk/session/aeva/NameManager.h"
// #include "smtk/session/aeva/Predicates.h"
#include "smtk/markup/operators/Import.h"

#include "smtk/markup/Resource.h"
#include "smtk/markup/operators/Import_xml.h"

#include "smtk/extension/vtk/io/ImportAsVTKData.h"

#include "smtk/view/Selection.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/operation/Hints.h"
#include "smtk/operation/MarkGeometry.h"

#include "smtk/resource/Manager.h"

#include "smtk/string/Token.h"

#include "smtk/common/Paths.h"

#include "vtkCellData.h"
#include "vtkCellTypes.h"
#include "vtkDataArray.h"
#include "vtkDataSet.h"
#include "vtkDataSetSurfaceFilter.h"
#include "vtkExodusIIReader.h"
#include "vtkFieldData.h"
#include "vtkGenerateGlobalIds.h"
#include "vtkGeometryFilter.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkIntArray.h"
#include "vtkLookupTable.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkMultiThreshold.h"
#include "vtkOrientPolyData.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkStringArray.h"
#include "vtkUnstructuredGrid.h"
#include "vtkXMLImageDataReader.h"
#include "vtkXMLPImageDataReader.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "boost/filesystem.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

#define PUGIXML_HEADER_ONLY
// NOLINTNEXTLINE(bugprone-suspicious-include)
#include "pugixml/src/pugixml.cpp"

#ifdef ERROR
#undef ERROR
#endif

#include <fstream>
#include <sstream>

namespace smtk
{
namespace markup
{

namespace
{

vtkSmartPointer<vtkImageData> readVTKImage(const std::string& filename)
{
  using namespace smtk::string::literals;
  vtkSmartPointer<vtkImageData> image;
  smtk::string::Token ext = smtk::common::Paths::extension(filename);
  switch (ext.id())
  {
    case ".nii"_hash:
    {
      smtkErrorMacro(smtk::io::Logger::instance(), "No support for nii yet.");
    }
    break;
    case ".vti"_hash:
    {
      // For now, load a surrogate VTK image
      vtkNew<vtkXMLImageDataReader> reader;
      reader->SetFileName(filename.c_str());
      reader->Update();
      image = reader->GetOutput();
    }
    break;
  }
  if (image)
  {
    auto* scalars = image->GetPointData()->GetScalars();
    if (scalars)
    {
      // Create a greyscale lookup table by default.
      auto* lkup = scalars->GetLookupTable();
      if (!lkup)
      {
        vtkNew<vtkLookupTable> greyscale;
        greyscale->SetSaturationRange(0, 0);
        greyscale->SetValueRange(0, 1);
        greyscale->Build();
        scalars->SetLookupTable(greyscale);
        lkup = greyscale;
      }
      lkup->SetTableRange(scalars->GetRange());
    }
  }
  return image;
}

int maxDimension(vtkSmartPointer<vtkDataObject> data)
{
  int result = -1;
  auto* dset = vtkDataSet::SafeDownCast(data);
  if (!dset)
  {
    return result;
  }

#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 2, 0)
  // This is stupid. In VTK 9.2+, we are not allowed to call the
  // vtkDataSet::GetCellTypes() method on unstructured grids without
  // a warning but polydata and other subclasses of dataset provide
  // no alternative API. vtkDataSet should provide a new API common
  // to *all* subclasses if the unstructured-grid implementation is
  // deprecated.
  if (auto* ugrid = vtkUnstructuredGrid::SafeDownCast(dset))
  {
    auto* cellTypes = ugrid->GetDistinctCellTypesArray();
    for (vtkIdType ii = 0; ii < cellTypes->GetNumberOfTuples(); ++ii)
    {
      int cellType = cellTypes->GetTuple1(ii);
      int dim = vtkCellTypes::GetDimension(cellType);
      if (dim > result)
      {
        result = dim;
      }
    }
  }
  else
#endif
  {
    // Use deprecated API.
    vtkNew<vtkCellTypes> cellTypes;
    dset->GetCellTypes(cellTypes);
    for (vtkIdType ii = 0; ii < cellTypes->GetNumberOfTypes(); ++ii)
    {
      int cellType = cellTypes->GetCellType(ii);
      int dim = vtkCellTypes::GetDimension(cellType);
      if (dim > result)
      {
        result = dim;
      }
    }
  }

  return result;
}

vtkSmartPointer<vtkPolyData> makeConsistent(const vtkSmartPointer<vtkPolyData>& surface)
{
  vtkNew<vtkOrientPolyData> orient;
  orient->SetInputDataObject(surface);
  orient->ConsistencyOn();
  orient->AutoOrientNormalsOn();
  orient->NonManifoldTraversalOn();
  orient->Update();

  auto result = vtkSmartPointer<vtkPolyData>::New();
  result->ShallowCopy(orient->GetOutput());
  return result;
}

struct OwlNode
{
  OwlNode() = default;
  OwlNode(const std::string& uu, const std::string& nn, const std::string& ii)
    : url(uu)
    , name(nn)
    , inherits(ii)
  {
  }

  std::string url;
  std::string name;
  std::string inherits;
  mutable smtk::markup::OntologyIdentifier* node = nullptr;
  mutable std::set<std::string> collection;

  bool operator<(const OwlNode& other) const { return url < other.url; }
};

struct OwlArc
{
  std::string url;
  std::string name;
  std::string domainUrl;
  std::string rangeUrl;
  std::size_t plural; // 0 = unlimited, 1 = singular, 2+ = fixed maximum

  bool operator<(const OwlArc& other) const { return url < other.url; }
};

class OWLParser
{
public:
  pugi::xml_document doc;
  std::map<std::string, OwlNode> nodeTypes;
  std::map<std::string, OwlArc> arcTypes;
  std::set<std::string> visitedNodes;

  void writeNode(std::ostream& sbt, const OwlNode& nodeType)
  {
    // Ignore nodes already written
    if (this->visitedNodes.find(nodeType.url) != this->visitedNodes.end())
    {
      return;
    }
    // Write base type before self
    if (!nodeType.inherits.empty())
    {
      this->writeNode(sbt, this->nodeTypes[nodeType.inherits]);
    }

    this->visitedNodes.insert(nodeType.url);
    std::cout << "  " << nodeType.url << " " << nodeType.name;
    sbt << "    <AttDef Type=\"" << nodeType.url << "\" Label=\"" << nodeType.name << "\"";
    if (!nodeType.inherits.empty())
    {
      std::cout << " :: " << nodeType.inherits;
      sbt << " BaseType=\"" << nodeType.inherits << "\"";
    }
    else
    {
      sbt << " BaseType=\"annotation\"";
    }
    std::cout << "\n";
    sbt << ">\n"
        << "      <BriefDescription>" << nodeType.name << "</BriefDescription>\n"
        << "    </AttDef>\n";
    for (const auto& subType : nodeType.collection)
    {
      std::cout << "    " << subType << "\n";
    }
  }

  std::string parseClass(const pugi::xml_node& classNode)
  {
    static int classId = 0;
    std::string label = OWLParser::labelOfNode(classNode, classId);
    std::string url = classNode.attribute("rdf:about").as_string(label.c_str());
    std::string inherits = OWLParser::inheritsNode(classNode);
    OwlNode node(url, label, inherits);
    auto result = this->nodeTypes.insert(std::make_pair(node.url, node));
    if (result.second)
    {
      // If it is a union of other classes, record the URLs in the rdf:about attributes:
      // <owl:unionOf rdf:parseType="Collection">
      //   <rdf:Description rdf:about="xxx"/>
      //   ...
      // </owl:unionOf>
      auto collNodes = classNode.select_nodes(
        "./owl:unionOf[@rdf:parseType='Collection']/rdf:Description[@rdf:about]");
      for (const auto& node : collNodes)
      {
        result.first->second.collection.insert(node.node().attribute("rdf:about").value());
      }
    }
    return url;
  }

  std::string parseObjectProperty(const pugi::xml_node& arcNode)
  {
    static int objectPropertyId = 0;
    std::string label = OWLParser::labelOfNode(arcNode, objectPropertyId);
    std::string url = arcNode.attribute("rdf:about").as_string(label.c_str());

    // If the ObjectProperty has a domain, parse it:
    auto domainNode = arcNode.select_node("./rdfs:domain");
    // std::string domainUrl = domainNode.node().find_attribute([](const pugi::xml_attribute& att) { return std::string(att.name()) == "rdf:resource"; }).value();
    std::string domainUrl = domainNode.node().attribute("rdf:resource").as_string("");
    if (domainUrl.empty())
    {
      auto domainClass = domainNode.node().select_node("./owl:Class").node();
      if (!domainClass.empty())
      {
        domainUrl = this->parseClass(domainClass);
      }
    }

    // If the ObjectProperty has a range, parse it:
    auto rangeNode = arcNode.select_node("./rdfs:range");
    std::string rangeUrl = rangeNode.node()
                             .find_attribute([](const pugi::xml_attribute& att) {
                               return std::string(att.name()) == "rdf:resource";
                             })
                             .value();
    if (rangeUrl.empty())
    {
      auto rangeClass = rangeNode.node().select_node("./owl:Class").node();
      if (!rangeClass.empty())
      {
        rangeUrl = this->parseClass(rangeClass);
      }
    }

    OwlArc arc{ url, label, domainUrl, rangeUrl, 0 };
    this->arcTypes.insert(std::make_pair(arc.url, arc));
    return url;
  }

  std::string parseAnnotationProperty(const pugi::xml_node& propNode)
  {
    (void)propNode;
    return std::string();
  }

  std::string labelOfNode(const pugi::xml_node& node, int& number)
  {
    std::string label;
    auto labelSel = node.select_node("./rdfs:label");
    if (!labelSel.node().empty())
    {
      label = labelSel.node().text().get();
    }
    else
    {
      std::ostringstream labelStr;
      labelStr << "anonymous class " << number++;
      label = labelStr.str();
    }
    return label;
  }

  std::string inheritsNode(const pugi::xml_node& node)
  {
    std::string inherits;
    auto subclassSel = node.select_node("./rdfs:subClassOf[@rdf:resource]");
    if (!subclassSel.node().empty())
    {
      inherits = subclassSel.node().attribute("rdf:resource").as_string("");
    }
    return inherits;
  }
};

} // anonymous namespace

Import::Result Import::operateInternal()
{
  using namespace smtk::string::literals;

  smtk::markup::Resource::Ptr resource = nullptr;
  m_result = this->createResult(Import::Outcome::FAILED);

  auto assoc = this->parameters()->associations();
  if (assoc->numberOfValues() > 0 && assoc->isSet())
  {
    resource = assoc->valueAs<smtk::markup::Resource>();
  }
  if (!resource)
  {
    resource = smtk::markup::Resource::create();
  }

  bool ok = true;
  bool consistency = this->parameters()->findVoid("consistency")->isEnabled();
  smtk::attribute::FileItem::Ptr filenameItem = this->parameters()->findFile("filename");
  for (std::size_t ii = 0; ii < filenameItem->numberOfValues(); ++ii)
  {
    std::string filename = filenameItem->value(ii);

    std::string potentialName = smtk::common::Paths::stem(filename);
    if (resource->name().empty() && !potentialName.empty())
    {
      // Name the resource after the first file we import.
      resource->setName(potentialName);
    }

    smtk::string::Token ext = smtk::common::Paths::extension(filename);
    switch (ext.id())
    {
      case ".owl"_hash:
        ok &= this->importOWL(resource, filename);
        break;
      case ".nii"_hash: // fall through
      case ".vti"_hash:
        ok &= this->importVTKImage(resource, filename);
        break;
      case ".ply"_hash: // fall through
      case ".obj"_hash: // fall through
      case ".stl"_hash: // fall through
      case ".vtp"_hash: // fall through
      case ".vtu"_hash: // fall through
      case ".vtk"_hash:
        ok &= this->importVTKMesh(resource, filename, consistency);
        break;
      default:
      {
        smtkErrorMacro(this->log(), "File \"" << filename << "\" has unsupported extension.");
        ok = false;
      }
      break;
    }
    if (!ok)
    {
      break;
    }
  }
  if (ok)
  {
    m_result->findInt("outcome")->setValue(0, static_cast<int>(Import::Outcome::SUCCEEDED));
    m_result->findResource("resourcesCreated")->appendValue(resource);

    auto created = m_result->findComponent("created");
    std::set<smtk::resource::Component::Ptr> importedComponents;
    importedComponents.insert(created->begin(), created->end());
    smtk::operation::addSelectionHint(
      m_result, importedComponents, smtk::view::SelectionAction::UNFILTERED_REPLACE);
  }
  return m_result;
}

Import::Specification Import::createSpecification()
{
  Specification spec = this->smtk::operation::XMLOperation::createSpecification();
  auto importDef = spec->findDefinition("import");

  std::vector<smtk::attribute::FileItemDefinition::Ptr> fileItemDefinitions;
  auto fileItemDefinitionFilter = [](smtk::attribute::FileItemDefinition::Ptr const& ptr) {
    return ptr->name() == "filename";
  };
  importDef->filterItemDefinitions(fileItemDefinitions, fileItemDefinitionFilter);

  assert(fileItemDefinitions.size() == 1);

  std::stringstream fileFilters;
  // Add fixed list of explicitly-supported import formats:
  fileFilters << "Web Ontology Language (*.owl);; ";
  fileFilters << "VTK Image Data (*.vti);; ";

  // Add extensions SMTK knows VTK can do.
  auto vtkFormats = Import::supportedVTKFileFormats();
  for (const auto& format : vtkFormats)
  {
    std::set<std::string> exts = format.second;
    if (exts.empty())
    {
      continue;
    }
    fileFilters << format.first << " (";
    bool vefirst = true;
    for (const auto& ext : exts)
    {
      fileFilters << (vefirst ? "*." : " *.") << ext;
      vefirst = false;
    }
    fileFilters << ");;";
  }

  fileItemDefinitions[0]->setFileFilters(fileFilters.str());
  return spec;
}

const char* Import::xmlDescription() const
{
  return Import_xml;
}

bool Import::importVTKImage(const Resource::Ptr& resource, const std::string& filename)
{
  vtkSmartPointer<vtkImageData> image = readVTKImage(filename);
  if (!image || image->GetNumberOfCells() == 0)
  {
    smtkInfoMacro(
      this->log(), "Unable to read image from \"" << filename << "\" or image is empty.");
    return false;
  }

  auto createdItems = m_result->findComponent("created");
  auto stem = smtk::common::Paths::stem(filename);
  auto imageNode = resource->createNode<ImageData>();
  auto imageImportURL = resource->createNode<URL>();
  auto imageURL = resource->createNode<URL>();
  ImageData::ShapeOptions options{ { m_result } };
  imageNode->setName(stem);
  imageNode->setShapeData(image, options);
  imageNode->importedFrom().connect(imageImportURL);
  imageImportURL->setName(stem + " url");
  imageImportURL->setLocation(filename);
  imageImportURL->setType("vtk/image");
  imageURL->setName(stem + " url");
  imageURL->setType("vtk/image");
  // NB: We do not set locations on the "output" URL as that will be
  //     set once the resource is written.
  imageURL->data().connect(imageNode);
  operation::MarkGeometry(resource).markModified(imageNode);

  createdItems->appendValue(imageNode);
  createdItems->appendValue(imageImportURL);
  createdItems->appendValue(imageURL);
  // NB: setShapeData has added Field nodes to the result for any
  //     new point/cell-data arrays.

  return true;
}

bool Import::importVTKMesh(
  const Resource::Ptr& resource,
  const std::string& filename,
  bool consistency)
{
  smtk::extension::vtk::io::ImportAsVTKData vtkImporter;
  auto data = vtkImporter(filename);
  if (!data)
  {
    return false;
  }
  if (consistency)
  {
    auto* pp = vtkPolyData::SafeDownCast(data.GetPointer());
    if (pp)
    {
      vtkSmartPointer<vtkPolyData> pdata(pp);
      data = makeConsistent(pdata);
    }
  }

  auto createdItems = m_result->findComponent("created");
  auto stem = smtk::common::Paths::stem(filename);
  auto meshNode = resource->createNode<UnstructuredData>(data);
  auto meshImportURL = resource->createNode<URL>();
  auto meshURL = resource->createNode<URL>();
  UnstructuredData::ShapeOptions options;
  options.trackedChanges = m_result;
  options.sharedPointIds = nullptr;
  meshURL->data().connect(meshNode);
  meshURL->setName(stem + " url");
  meshURL->setType(data->IsA("vtkUnstructuredGrid") ? "vtk/unstructured-grid" : "vtk/polydata");
  // NB: We do not set locations on the "output" URL as that will be
  //     set once the resource is written.
  meshImportURL->setName(stem + " import url");
  meshImportURL->setLocation(filename);
  meshImportURL->setType(smtk::common::Paths::extension(filename));
  meshNode->importedFrom().connect(meshImportURL);
  meshNode->setName(stem);
  meshNode->setShapeData(data, options);
  operation::MarkGeometry marker(resource);
  marker.markModified(meshNode);
  createdItems->appendValue(meshNode);
  createdItems->appendValue(meshImportURL);
  createdItems->appendValue(meshURL);
  // NB: setShapeData has added Field nodes to the result for any
  //     new point/cell-data arrays.

  // If importing a volume mesh, add its boundary as another component.
  // TODO: Detect feature edges/vertices and add even more boundaries of boundaries?
  if (maxDimension(data) == 3)
  {
    // TODO: Create a SideSet, not an "unrelated" UnstructuredData
    vtkNew<vtkDataSetSurfaceFilter> dssf;
    dssf->SetInputDataObject(data);
    dssf->PassThroughCellIdsOn();
    dssf->PassThroughPointIdsOn();
    dssf->Update();
    auto* dataBdy = dssf->GetOutputDataObject(0);
    auto bdyNode = resource->createNode<UnstructuredData>();
    options.sharedPointIds = const_cast<AssignedIds*>(&meshNode->pointIds())->shared_from_this();
    bdyNode->setShapeData(dataBdy, options);
    bdyNode->parents().connect(meshNode);
    bdyNode->setName("∂" + stem);
    createdItems->appendValue(bdyNode);
  }

  return true;
}

bool Import::importOWL(const Resource::Ptr& resource, const std::string& filename)
{
  OWLParser p;
  pugi::xml_parse_result presult = p.doc.load_file(filename.c_str());
  if (presult.status != pugi::status_ok)
  {
    smtkErrorMacro(this->log(), presult.description());
    return false;
  }
  pugi::xml_node root = p.doc.child("rdf:RDF");
  if (!root)
  {
    smtkErrorMacro(this->log(), "No root node in OWL file \"" << filename << "\".");
    return false;
  }
  std::map<std::string, int> nodeNames;
  for (const auto& node : root)
  {
    std::string nodeName = node.name();
    ++nodeNames[nodeName];
    if (nodeName == "owl:Class")
    {
      p.parseClass(node);
    }
    else if (nodeName == "owl:AnnotationProperty")
    {
      p.parseAnnotationProperty(node);
    }
    else if (nodeName == "owl:ObjectProperty")
    {
      p.parseObjectProperty(node);
    }
  }

  // Create the parent of all ontology identifiers.
  auto createdItems = m_result->findComponent("created");
  auto ontologyNode = resource->createNode<Ontology>();
  auto ontologyImportURL = resource->createNode<URL>();
  auto stem = smtk::common::Paths::stem(filename);
  ontologyNode->setUrl(filename);
  ontologyNode->importedFrom().connect(ontologyImportURL);
  ontologyImportURL->setName(stem + " import url");
  ontologyImportURL->setLocation(filename);
  ontologyImportURL->setType("ontology/owl");
  createdItems->appendValue(ontologyNode);
  createdItems->appendValue(ontologyImportURL);
  // NB: We do not provide an "output" URL as we do not save
  //     the entire imported ontology with each document.
  //     They can get quite large and are properly part of
  //     the application, not its documents.

  try
  {
    // Pass 1: Add nodes.
    for (const auto& entry : p.nodeTypes)
    {
      auto node = resource->createNode<OntologyIdentifier>();
      node->setName(entry.second.name);
      node->setOntologyId(entry.second.url);
      ontologyNode->outgoing<arcs::OntologyToIdentifiers>().connect(node);
      entry.second.node = node.get();
      createdItems->appendValue(node);
    }

    // Pass 2: Add arcs for collection entries.
    for (const auto& parent : p.nodeTypes)
    {
      for (const auto& childURL : parent.second.collection)
      {
        const auto& child(p.nodeTypes[childURL]);
        parent.second.node->outgoing<arcs::OntologyIdentifiersToSubtypes>().connect(child.node);
      }
    }
  }
  catch (std::exception& e)
  {
    smtkErrorMacro(this->log(), "Could not create nodes. " << e.what());
    return false;
  }
  return true;
}

std::map<std::string, std::set<std::string>> Import::supportedVTKFileFormats()
{
  std::map<std::string, std::set<std::string>> result;
  smtk::extension::vtk::io::ImportAsVTKData vtkImporter;
  auto formats = vtkImporter.fileFormats();
  for (const auto& format : formats)
  {
    std::set<std::string> extensions(format.Extensions.begin(), format.Extensions.end());
    result[format.Name] = extensions;
  }
  return result;
}

smtk::resource::ResourcePtr importResource(const std::string& filename)
{
  Import::Ptr importResource = Import::create();
  importResource->parameters()->findFile("filename")->setValue(filename);
  Import::Result result = importResource->operate();
  if (result->findInt("outcome")->value() != static_cast<int>(Import::Outcome::SUCCEEDED))
  {
    return smtk::resource::ResourcePtr();
  }
  return result->findResource("resourcesCreated")->value();
}
} // namespace markup
} // namespace smtk
