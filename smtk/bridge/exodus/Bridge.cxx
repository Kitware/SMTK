#include "Bridge.h"
#include "Bridge_json.h"

#include "vtkCellArray.h"
#include "vtkInformation.h"
#include "vtkInformationIntegerVectorKey.h"
#include "vtkInformationStringKey.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkUnsignedIntArray.h"

namespace smtk {
  namespace bridge {
    namespace exodus {

vtkInformationKeyMacro(Bridge,SMTK_UUID_KEY,String);

enum smtkCellTessRole {
  SMTK_ROLE_VERTS,
  SMTK_ROLE_LINES,
  SMTK_ROLE_POLYS
};

Bridge::Bridge()
{
}

Bridge::~Bridge()
{
}

/// Turn any valid cursor into an entity handle.
EntityHandle toEntity(const smtk::model::Cursor& eid)
{
  ReverseIdMap_t::const_iterator it = this->m_revIdMap.find(eid);
  if (it == this->m_revIdMap.end())
    return EntityHandle();
  return it->second;
}

smtk::model::Cursor toCursor(const EntityHandle& ent)
{
  vtkDataObject* entData = this->toBlock<vtkDataObject>(ent);
  if (!entData)
    return Cursor();

  const char* uuidChar = entData->GetInformation()->Get(SMTK_UUID_KEY());
  smtk::common::UUID uid;
  if (!uuidChar)
    { // We have not assigned a UUID yet. Do so now.
    uid = this->m_uuidGen.random();
    entData->GetInformation()->Set(SMTK_UUID_KEY(), uid.toString().c_str());
    }
  else
    {
    uid = smtk::common::UUID(uuidChar);
    }
  return Cursor(this->m_manager, uid);
}

BridgedInfoBits Bridge::transcribeInternal(
  const smtk::model::Cursor& entity,
  BridgedInfoBits requestedInfo)
{
  EntityHandle handle = this->toHandle(entity);
  if (!handle.isValid())
    return BRIDGE_NOTHING;

  vtkDataSet* obj = this->toBlock<vtkDataSet>(handle);
  if (!obj)
    return BRIDGE_NOTHING;

}

// A method that helps convert vtkPolyData into an SMTK Tessellation.
static void AddCellsToTessellation(
  vtkPoints* pts,
  vtkCellArray* cells,
  smtkCellTessRole role,
  std::map<vtkIdType,int>& vertMap,
  smtk::model::Tessellation& tess)
{
  vtkIdType npts;
  vtkIdType* conn;
  std::vector<int> tconn;
  std::map<vtkIdType,int>::iterator pit;
  for (cells->InitTraversal(); cells->GetNextCell(npts, conn); )
    {
    tconn.clear();
    tconn.reserve(npts + 2);
    switch (role)
      {
    case SMTK_ROLE_VERTS:
      if (npts > 1)
        {
        tconn.push_back(TESS_POLYVERTEX);
        tconn.push_back(npts);
        }
      else
        {
        tconn.push_back(TESS_VERTEX);
        }
      break;
    case SMTK_ROLE_LINES:
      tconn.push_back(TESS_POLYLINE);
      tconn.push_back(npts);
      break;
    case SMTK_ROLE_POLYS:
      switch (npts)
        {
      case 0:
      case 1:
      case 2:
        std::cerr
          << "Too few points (" << npts
          << ") for a surface primitive. Skipping.\n";
        continue;
        break;
      case 3: tconn.push_back(TESS_TRIANGLE); break;
      case 4: tconn.push_back(TESS_QUAD); break;
      default: tconn.push_back(TESS_POLYGON); tconn.push_back(npts); break;
        }
      break;
    default:
      std::cerr << "Unknown tessellation role " << role << ". Skipping.\n";
      continue;
      break;
      }
    for (vtkIdType i = 0; i < npts; ++i)
      {
      if ((pit = vertMap.find(conn[i])) == vertMap.end())
        pit = vertMap.insert(
          std::pair<vtkIdType,int>(
            conn[i], tess.addCoords(pts->GetPoint(conn[i])))).first;
      tconn.push_back(pit->second );
      }
    tess.insertNextCell(tconn);
    }
}


bool Bridge::addTessellation(
  const smtk::model::Cursor& cursor,
  vtkDataObject* data)
{
  if (cursor.hasTessellation())
    return true; // no need to recompute.

  if (!data)
    return false; // can't squeeze triangles from a NULL

  vtkNew<vtkGeometryFilter> bdyFilter;
  bdyFilter->MergingOff();
  bdyFilter->SetInputDataObject(data);
  bdyFilter->Update();
  vtkPolyData* bdy = bdyFilter->GetOutput();

  if (!bdy)
    return BRIDGE_NOTHING;

  smtk::model::Tessellation tess;
  std::map<vtkIdType,int> vertMap;
  vtkPoints* pts = bdy->GetPoints();
  vtkCellArray* cells;
  cells = bdy->GetVerts();
  AddCellsToTessellation(pts, bdy->GetVerts(), SMTK_ROLE_VERTS, vertMap, tess);
  AddCellsToTessellation(pts, bdy->GetLines(), SMTK_ROLE_LINES, vertMap, tess);
  AddCellsToTessellation(pts, bdy->GetPolys(), SMTK_ROLE_POLYS, vertMap, tess);
  if (bdy->GetStrips() && bdy->GetStrips()->GetNumberOfCells() > 0)
    {
    std::cerr << "Warning: Triangle strips in discrete cells are unsupported. Ignoring.\n";
    }
  if (!vertMap.empty())
    cursor.manager()->setTessellation(cursor.entity(), tess);

  return true;
}

    } // namespace exodus
  } // namespace bridge
} // namespace smtk

smtkImplementsModelingKernel(
  exodus,
  Bridge_json,
  BridgeHasNoStaticSetup,
  smtk::bridge::exodus::Bridge
);
