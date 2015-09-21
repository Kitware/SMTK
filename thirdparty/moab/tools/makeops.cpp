/*
 * Program to make hex modification operation meshes
 *
 */

#include "moab/Core.hpp"
#include "moab/Range.hpp"
#include <iostream>

using namespace moab;

Interface *gMB = NULL;

static ErrorCode make_atomic_pillow();
static ErrorCode make_face_shrink();
static ErrorCode make_face_open_collapse();
static ErrorCode make_chord_push();
static ErrorCode make_triple_chord_push();
static ErrorCode make_triple_hex_push();

enum OperationType {ATOMIC_PILLOW = 0, 
                    FACE_OPEN_COLLAPSE, 
                    FACE_SHRINK, 
                    CHORD_PUSH, 
                    MBTRIPLE_CHORD_PUSH, 
                    MBTRIPLE_HEX_PUSH,
                    UNDEFINED};

const char *OperationNames[] = {"atomic_pillow", 
                                "face_open_collapse", 
                                "face_shrink", 
                                "chord_push", 
                                "triple_chord_push", 
                                "triple_hex_push",
                                "undefined"};

int main(int argc, char **argv) 
{
  gMB = new Core();
  const char *extensions[] = 
    {
      ".g",
      ".h5m",
      ".vtk"
    };
  int file_exten = 1;
  
  std::vector<OperationType> op_types;

  if (argc < 2) {
    std::cout << "Usage: " << argv[0] 
              << " [-h5m] [-vtk] {-ap | -foc | -fs | -cp | -tcp | -thp}" 
              << std::endl;
    return 1;
  }
    
  int current_arg = 1;
  while (current_arg < argc) {
    if (!strcmp("-g", argv[current_arg])) file_exten = 0;
    else if (!strcmp("-h5m", argv[current_arg])) file_exten = 1;
    else if (!strcmp("-vtk", argv[current_arg])) file_exten = 2;
    else if (!strcmp("-ap", argv[current_arg])) 
      op_types.push_back(ATOMIC_PILLOW);
    else if (!strcmp("-foc", argv[current_arg])) 
      op_types.push_back(FACE_OPEN_COLLAPSE);
    else if (!strcmp("-fs", argv[current_arg])) 
      op_types.push_back(FACE_SHRINK);
    else if (!strcmp("-cp", argv[current_arg])) 
      op_types.push_back(CHORD_PUSH);
    else if (!strcmp("-tcp", argv[current_arg])) 
      op_types.push_back(MBTRIPLE_CHORD_PUSH);
    else if (!strcmp("-thp", argv[current_arg])) 
      op_types.push_back(MBTRIPLE_HEX_PUSH);
    current_arg++;
  }
  
  ErrorCode result = MB_SUCCESS, tmp_result = MB_FAILURE;

  for (std::vector<OperationType>::iterator vit = op_types.begin(); 
       vit != op_types.end(); vit++) {
    if (*vit == ATOMIC_PILLOW) {
      tmp_result = make_atomic_pillow();
    }
    else if (*vit == FACE_OPEN_COLLAPSE) {
      tmp_result = make_face_open_collapse();
    }
    else if (*vit == CHORD_PUSH) {
      tmp_result = make_chord_push();
    }
    else if (*vit == MBTRIPLE_CHORD_PUSH) {
      tmp_result = make_triple_chord_push();
    }

    else if (*vit == MBTRIPLE_HEX_PUSH) {
      tmp_result = make_triple_hex_push();
    }
    else if (*vit == FACE_SHRINK) {
      tmp_result = make_face_shrink();
    }
    else {
      std::cout << "Operation undefined." << std::endl;
      return 1;
    }
    if (MB_SUCCESS != tmp_result) result = tmp_result;
  
    // now write to a file
    std::string filename(OperationNames[*vit]);
    filename.append(extensions[file_exten]);
    tmp_result = gMB->write_mesh(filename.c_str());
    if (MB_SUCCESS != tmp_result) result = tmp_result;
  }
    
  return (result == MB_SUCCESS ? 0 : 1);
}

ErrorCode make_atomic_pillow() 
{
    // make atomic pillow configuration
    // make all vertices
  double vtx_coord[] = 
    {
      1.0, 1.0, 1.0, 
      1.0, 0.0, 1.0, 
      0.0, 0.0, 1.0, 
      0.0, 1.0, 1.0, 
      .75, .75, 1.0, 
      .75, .25, 1.0, 
      .25, .25, 1.0, 
      .25, .75, 1.0
    };

  int connect[] = {
    0, 1, 2, 3, 4, 5, 6, 7,
    4, 5, 6, 7, 0, 1, 2, 3
  };

  ErrorCode result;
  EntityHandle vtx_handles[8];
  
  for (int i = 0; i < 8; i++) {
    result = gMB->create_vertex(&vtx_coord[3*i], vtx_handles[i]);
    if (MB_SUCCESS != result) return MB_FAILURE;
  }
  
  EntityHandle conn[8], elems[4];

    // make the two hexes
  for (int i = 0; i < 8; i++)
    conn[i] = vtx_handles[connect[i]];
  result = gMB->create_element(MBHEX, conn, 8, elems[0]);
  if (MB_SUCCESS != result) return MB_FAILURE;

  for (int i = 0; i < 8; i++)
    conn[i] = vtx_handles[connect[8+i]];
  result = gMB->create_element(MBHEX, conn, 8, elems[1]);
  if (MB_SUCCESS != result) return MB_FAILURE;

    // make one of the end quads explicitly and bind to the first hex
  for (int i = 0; i < 4; i++)
    conn[i] = vtx_handles[connect[i]];
  result = gMB->create_element(MBQUAD, conn, 4, elems[2]);
  if (MB_SUCCESS != result) return MB_FAILURE;
  
  result = gMB->add_adjacencies(elems[2], elems, 1, false);
  if (MB_SUCCESS != result) return MB_FAILURE;

    // now the other one
  result = gMB->create_element(MBQUAD, conn, 4, elems[3]);
  if (MB_SUCCESS != result) return MB_FAILURE;
  
  result = gMB->add_adjacencies(elems[3], &elems[1], 1, false);
  if (MB_SUCCESS != result) return MB_FAILURE;

  return MB_SUCCESS;
}

ErrorCode make_face_shrink() 
{
    // make face shrink configuration
    // make all vertices
  double vtx_coord[] = 
    {
      1.0, 1.0, 0.0, 
      1.0, 0.0, 0.0, 
      0.0, 0.0, 0.0, 
      0.0, 1.0, 0.0, 
      1.0, 1.0, 1.0, 
      1.0, 0.0, 1.0, 
      0.0, 0.0, 1.0, 
      0.0, 1.0, 1.0, 
      1.0, 1.0, 2.0, 
      1.0, 0.0, 2.0, 
      0.0, 0.0, 2.0, 
      0.0, 1.0, 2.0, 
      .75, .75, 1.0, 
      .75, .25, 1.0, 
      .25, .25, 1.0, 
      .25, .75, 1.0
    };

  int connect[] = {
    3, 7, 11, 15, 0, 4, 8, 12,
    0, 4, 8, 12, 1, 5, 9, 13,
    1, 5, 9, 13, 2, 6, 10, 14,
    2, 6, 10, 14, 3, 7, 11, 15,
    0, 3, 2, 1, 12, 15, 14, 13,
    12, 15, 14, 13, 8, 11, 10, 9
  };

  ErrorCode result;
  EntityHandle vtx_handles[16];
  
  for (int i = 0; i < 16; i++) {
    result = gMB->create_vertex(&vtx_coord[3*i], vtx_handles[i]);
    if (MB_SUCCESS != result) return MB_FAILURE;
  }
  
    // make all elements at once
  EntityHandle conn[8], elems[6];

  for (int j = 0; j < 6; j++) {
    for (int i = 0; i < 8; i++)
      conn[i] = vtx_handles[connect[j*8+i]];
    
    result = gMB->create_element(MBHEX, conn, 8, elems[j]);
    if (MB_SUCCESS != result) return MB_FAILURE;
  }
  
  return MB_SUCCESS;
}

ErrorCode make_face_open_collapse() 
{
  return MB_FAILURE;
}

ErrorCode make_chord_push() 
{
    // make chord push configuration
    // make all vertices
  double vtx_coord[] = 
    {
        // first layer
      0.0, 0.0, 0.5,
      0.0, 1.0, 0.0,
      -1.0, 0.5, 0.0, 
      -1.0, -0.5, 0.0,
      0.0, -1.0, 0.0,
      1.0, -0.5, 0.0,
      1.0, 0.5, 0.0,
        // second layer
      0.0, 0.0, -1.5,
      0.0, 1.0, -1.0,
      -1.0, 0.5, -1.0, 
      -1.0, -0.5, -1.0,
      0.0, -1.0, -1.0,
      1.0, -0.5, -1.0,
      1.0, 0.5, -1.0,
        // 2 extra vertices for chord push
      0.0, -.333, 0.05,
      0.0, -.667, 0.10
    };

  int connect[] = {
      // 3 "normal" hexes first
      // top hex
    0, 2, 1, 6, 7, 9, 8, 13,
      // bottom left
    0, 4, 3, 2, 7, 11, 10, 9,
      // bottom right
    6, 5, 4, 0, 13, 12, 11, 7,
      // front chord push hex
    2, 0, 4, 3, 14, 6, 5, 15,
      // back chord push hex
    2, 14, 15, 3, 0, 6, 5, 4,
      // front/rear quads a, b
    2, 0, 4, 3, 6, 5, 4, 0,
      // duplicate edges from chord push
    0, 4,
      // face between bottom 2 normal hexes (needed for explicit
      // adjacency)
    0, 4, 11, 7
  };

  ErrorCode result;
  EntityHandle vtx_handles[16];
  
  for (int i = 0; i < 16; i++) {
    result = gMB->create_vertex(&vtx_coord[3*i], vtx_handles[i]);
    if (MB_SUCCESS != result) return MB_FAILURE;
  }
  
  EntityHandle conn[8], elems[12];

    // make the five hexes
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 8; j++)
      conn[j] = vtx_handles[connect[8*i+j]];
    result = gMB->create_element(MBHEX, conn, 8, elems[i]);
    if (MB_SUCCESS != result) return MB_FAILURE;
  }

    // make the frontmost pair of quads and bind to the front degen hex
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 4; j++)
      conn[j] = vtx_handles[connect[40+4*i+j]];
    result = gMB->create_element(MBQUAD, conn, 4, elems[5+i]);
    if (MB_SUCCESS != result) return MB_FAILURE;
  }
  
    // now the back pair
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 4; j++)
      conn[j] = vtx_handles[connect[40+4*i+j]];
    result = gMB->create_element(MBQUAD, conn, 4, elems[7+i]);
    if (MB_SUCCESS != result) return MB_FAILURE;
  }

    // make the duplicated edges explicitly too
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2; j++)
      conn[j] = vtx_handles[connect[48+j]];
    result = gMB->create_element(MBEDGE, conn, 2, elems[9+i]);
    if (MB_SUCCESS != result) return MB_FAILURE;
  }

    // now the quad between the lower pair of hexes
  for (int j = 0; j < 4; j++)
    conn[j] = vtx_handles[connect[50+j]];
  result = gMB->create_element(MBQUAD, conn, 4, elems[11]);
  if (MB_SUCCESS != result) return MB_FAILURE;

    // now set adjacencies explicitly
    // front/rear duplicated edge to front/rear pair of quads
  result = gMB->add_adjacencies(elems[9], &elems[5], 2, false);
  if (MB_SUCCESS != result) return MB_FAILURE;
  result = gMB->add_adjacencies(elems[10], &elems[7], 2, false);
  if (MB_SUCCESS != result) return MB_FAILURE;

    // rear duplicated edge to quad between lower pair of normal hexes
  result = gMB->add_adjacencies(elems[10], &elems[11], 1, false);
  if (MB_SUCCESS != result) return MB_FAILURE;

    // front/rear duplicated edge to front/rear degen hex
  result = gMB->add_adjacencies(elems[9], &elems[3], 1, false);
  if (MB_SUCCESS != result) return MB_FAILURE;
  result = gMB->add_adjacencies(elems[10], &elems[4], 1, false);
  if (MB_SUCCESS != result) return MB_FAILURE;

    // rear duplicated edge to normal hexes behind it
  result = gMB->add_adjacencies(elems[10], &elems[1], 2, false);
  if (MB_SUCCESS != result) return MB_FAILURE;

    // front pair of quads to front degen hex
  result = gMB->add_adjacencies(elems[5], &elems[3], 1, false);
  if (MB_SUCCESS != result) return MB_FAILURE;
  result = gMB->add_adjacencies(elems[6], &elems[3], 1, false);
  if (MB_SUCCESS != result) return MB_FAILURE;

    // rear pair of quads to rear degen hex
  result = gMB->add_adjacencies(elems[7], &elems[4], 1, false);
  if (MB_SUCCESS != result) return MB_FAILURE;
  result = gMB->add_adjacencies(elems[8], &elems[4], 1, false);
  if (MB_SUCCESS != result) return MB_FAILURE;

    // rear pair of quads to normal hexes behind them
  result = gMB->add_adjacencies(elems[7], &elems[1], 1, false);
  if (MB_SUCCESS != result) return MB_FAILURE;
  result = gMB->add_adjacencies(elems[8], &elems[2], 1, false);
  if (MB_SUCCESS != result) return MB_FAILURE;

  return MB_SUCCESS;
}

ErrorCode make_triple_chord_push() 
{
    // make chord push configuration
    // make all vertices
  double vtx_coord[] = 
    {
        // first layer
      0.0, 0.0, 0.5,
      0.0, 1.0, 0.0,
      -1.0, 0.5, 0.0, 
      -1.0, -0.5, 0.0,
      0.0, -1.0, 0.0,
      1.0, -0.5, 0.0,
      1.0, 0.5, 0.0,
        // second layer
      0.0, 0.0, -1.5,
      0.0, 1.0, -1.0,
      -1.0, 0.5, -1.0, 
      -1.0, -0.5, -1.0,
      0.0, -1.0, -1.0,
      1.0, -0.5, -1.0,
      1.0, 0.5, -1.0,
        // 2 extra vertices in middle
      0.0, 0.0, -0.25,
      0.0, 0.0, 0.0
    };

  int connect[] = {
      // 3 "normal" hexes first
      // top hex
    14, 2, 1, 6, 7, 9, 8, 13,
      // bottom left
    14, 4, 3, 2, 7, 11, 10, 9,
      // bottom right
    6, 5, 4, 14, 13, 12, 11, 7,
      // front triple chord push hex
    0, 4, 3, 2, 6, 5, 15, 1,
      // back triple chord push hex
    2, 1, 15, 3, 14, 6, 5, 4
  };

  ErrorCode result;
  EntityHandle vtx_handles[16];
  
  for (int i = 0; i < 16; i++) {
    result = gMB->create_vertex(&vtx_coord[3*i], vtx_handles[i]);
    if (MB_SUCCESS != result) return MB_FAILURE;
  }
  
  EntityHandle conn[8], elems[12];

    // make the five hexes
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 8; j++)
      conn[j] = vtx_handles[connect[8*i+j]];
    result = gMB->create_element(MBHEX, conn, 8, elems[i]);
    if (MB_SUCCESS != result) return MB_FAILURE;
  }

  return MB_SUCCESS;
}

ErrorCode make_triple_hex_push() 
{
  return MB_FAILURE;
}

