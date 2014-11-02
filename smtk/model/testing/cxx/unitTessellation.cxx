#include "smtk/model/Tessellation.h"

#include "smtk/common/testing/cxx/helpers.h"

using namespace smtk::model;

int main()
{
  Tessellation tess;
  double coords[] = {
    0., 0., 0.,
    2., 0., 0.,
    2., 2., 0.,
    0., 2., 0.,
    1., 3., 0.,
    0., 4., 0.,
    1., 1., 1.
  };
  tess.coords().insert(tess.coords().end(), coords, coords + sizeof(coords)/sizeof(coords[0]));

  std::vector<int> cellTypeCheck;
  std::vector<Tessellation::size_type> cellMatCheck;

  int cellConn1[] = { // INVALID vertex (not enough entries)
    TESS_VERTEX | TESS_FACE_MATERIAL | TESS_FACE_VERTEX_NORMAL,
    0
  };
  std::vector<int> cell1(cellConn1, cellConn1 + sizeof(cellConn1)/sizeof(cellConn1[0]));
  test(!tess.insertCell(0, cell1), "Expected invalid vertex 1 insertion to fail.");

  int cellConn2[] = { // VALID vertex with "don't care" material ID of 100 and "don't care" normal ID
    TESS_VERTEX | TESS_FACE_MATERIAL | TESS_FACE_VERTEX_NORMAL,
    0, 100, 0
  };
  std::vector<int> cell2(cellConn2, cellConn2 + sizeof(cellConn2)/sizeof(cellConn2[0]));
  test(tess.insertCell(0, cell2), "Expected valid vertex 2 insertion to succeed.");
  cellTypeCheck.insert(cellTypeCheck.begin(), cellConn2[0]);
  cellMatCheck.insert(cellMatCheck.begin(), cellConn2[2]);

  int cellConn3[] = { // INVALID polyvertex (missing normal ID entries)
    TESS_POLYVERTEX | TESS_FACE_VERTEX_NORMAL,
    3,
    0, 1, 2
  };
  std::vector<int> cell3(cellConn3, cellConn3 + sizeof(cellConn3)/sizeof(cellConn3[0]));
  test(!tess.insertCell(0, cell3), "Expected invalid polyvertex 3 insertion to fail.");

  int cellConn4[] = { // VALID polyvertex with normals
    TESS_POLYVERTEX | TESS_FACE_VERTEX_NORMAL,
    3, // num vertices
    0, 1, 2, // vert conn
    0, 1, 2  // vert normal IDs
  };
  std::vector<int> cell4(cellConn4, cellConn4 + sizeof(cellConn4)/sizeof(cellConn4[0]));
  test(tess.insertCell(0, cell4), "Expected valid polyvertex 4 insertion to succeed.");
  cellTypeCheck.insert(cellTypeCheck.begin(), cellConn4[0]);
  cellMatCheck.insert(cellMatCheck.begin(), -1);

  int cellConn5[] = { // VALID triangle
    TESS_TRIANGLE,
    2, 4, 3 // vert conn
  };
  std::vector<int> cell5(cellConn5, cellConn5 + sizeof(cellConn5)/sizeof(cellConn5[0]));
  test(tess.insertCell(0, cell5), "Expected valid triangle 5 insertion to succeed.");
  cellTypeCheck.insert(cellTypeCheck.begin(), cellConn5[0]);
  cellMatCheck.insert(cellMatCheck.begin(), -1);

  int cellConn6[] = { // VALID quad
    TESS_QUAD,
    0, 1, 2, 3 // vert conn
  };
  std::vector<int> cell6(cellConn6, cellConn6 + sizeof(cellConn6)/sizeof(cellConn6[0]));
  test(tess.insertCell(0, cell6), "Expected valid quad 6 insertion to succeed.");
  cellTypeCheck.insert(cellTypeCheck.begin(), cellConn6[0]);
  cellMatCheck.insert(cellMatCheck.begin(), -1);

  int cellConn7[] = { // VALID polyline
    TESS_POLYLINE | TESS_FACE_UV | TESS_FACE_MATERIAL | TESS_FACE_VERTEX_NORMAL | TESS_FACE_VERTEX_COLOR,
    5,
    0, 1, 2, 4, 3,
    100, // material
    10, // UV
    0, 1, 2, 4, 3,
    0, 1, 2, 4, 3
  };
  std::vector<int> cell7(cellConn7, cellConn7 + sizeof(cellConn7)/sizeof(cellConn7[0]));
  test(tess.insertCell(0, cell7), "Expected valid polyline 7 insertion to succeed.");
  cellTypeCheck.insert(cellTypeCheck.begin(), cellConn7[0]);
  cellMatCheck.insert(cellMatCheck.begin(), cellConn7[7]);

  int cellConn8[] = { // VALID polygon
    TESS_POLYGON,
    6,
    0, 1, 2, 4, 5, 3
  };
  std::vector<int> cell8(cellConn8, cellConn8 + sizeof(cellConn8)/sizeof(cellConn8[0]));
  test(tess.insertCell(0, cell8), "Expected valid polygon 8 insertion to succeed.");
  cellTypeCheck.insert(cellTypeCheck.begin(), cellConn8[0]);
  cellMatCheck.insert(cellMatCheck.begin(), -1);

  int cellConn9[] = { // VALID triangle strip
    TESS_TRIANGLE_STRIP,
    5,
    0, 1, 3, 2, 4
  };
  std::vector<int> cell9(cellConn9, cellConn9 + sizeof(cellConn9)/sizeof(cellConn9[0]));
  test(tess.insertNextCell(cell9) > 0, "Expected valid triangle strip 9 insertion to succeed.");
  cellTypeCheck.push_back(cellConn9[0]);
  cellMatCheck.push_back(-1);

  int cellConn10[] = { // INVALID cell
    TESS_INVALID_CELL,
    0, 1, 2
  };
  std::vector<int> cell10(cellConn10, cellConn10 + sizeof(cellConn10)/sizeof(cellConn10[0]));
  test(!tess.insertCell(0, cell10), "Expected invalid cell 10 insertion to fail.");

  // Now iterate over cells
  Tessellation::size_type i;
  Tessellation::size_type cellType;
  Tessellation::size_type materialId;
  std::vector<int> conn;
  int cellId = 0;
  std::vector<int>::const_iterator ctcit = cellTypeCheck.begin();
  std::vector<Tessellation::size_type>::const_iterator cmcit = cellMatCheck.begin();
  for (i = tess.begin(); i != tess.end(); i = tess.nextCellOffset(i), ++cellId, ++ctcit, ++cmcit)
    {
    cellType = tess.cellType(i);
    materialId = tess.materialIdOfCell(i);
    std::cout
      << "Cell " << cellId
      << ": type " << cellType
      << " off " << i
      << " # " << tess.numberOfCellVertices(i, NULL)
      << " mat " << tess.materialIdOfCell(i)
      << "\n  conn";
    test(*ctcit == cellType, "Incorrect cell type.");
    test(*cmcit == materialId, "Incorrect cell material ID.");
    tess.vertexIdsOfCell(i, conn);
    for (std::vector<int>::iterator it = conn.begin(); it != conn.end(); ++it)
      std::cout << " " << *it;
    std::cout << "\n";
    conn.clear();
    }

  return 0;
}
