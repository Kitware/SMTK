//-------------------------------------------------------------------------
// Filename      : NCHelperGCRM.hpp
//
// Purpose       : Climate NC file helper for GCRM grid
//
// Creator       : Danqing Wu
//-------------------------------------------------------------------------

#ifndef NCHELPERGCRM_HPP
#define NCHELPERGCRM_HPP

#include "NCHelper.hpp"

namespace moab {

//! Child helper class for GCRM grid
class NCHelperGCRM : public UcdNCHelper
{
public:
  NCHelperGCRM(ReadNC* readNC, int fileId, const FileOptions& opts, EntityHandle fileSet);
  static bool can_read_file(ReadNC* readNC);

private:
  //! Implementation of NCHelper::init_mesh_vals()
  virtual ErrorCode init_mesh_vals();
  //! Implementation of NCHelper::check_existing_mesh()
  virtual ErrorCode check_existing_mesh();
  //! Implementation of NCHelper::create_mesh()
  virtual ErrorCode create_mesh(Range& faces);
  //! Implementation of NCHelper::get_mesh_type_name()
  virtual std::string get_mesh_type_name() { return "GCRM"; }

  //! Implementation of UcdNCHelper::read_ucd_variables_to_nonset_allocate()
  virtual ErrorCode read_ucd_variables_to_nonset_allocate(std::vector<ReadNC::VarData>& vdatas,
                                                         std::vector<int>& tstep_nums);
#ifdef MOAB_HAVE_PNETCDF
  //! Implementation of UcdNCHelper::read_ucd_variables_to_nonset_async()
  virtual ErrorCode read_ucd_variables_to_nonset_async(std::vector<ReadNC::VarData>& vdatas,
                                                      std::vector<int>& tstep_nums);
#else
  //! Implementation of UcdNCHelper::read_ucd_variables_to_nonset()
  virtual ErrorCode read_ucd_variables_to_nonset(std::vector<ReadNC::VarData>& vdatas,
                                                std::vector<int>& tstep_nums);
#endif

  //! Redistribute local cells after trivial partition (e.g. Zoltan partition, if applicable)
  ErrorCode redistribute_local_cells(int start_cell_index);

  //! Create local vertices
  ErrorCode create_local_vertices(const std::vector<int>& vertices_on_local_cells, EntityHandle& start_vertex);

  //! Create local edges (optional)
  ErrorCode create_local_edges(EntityHandle start_vertex);

  //! Create local cells with padding (pentagons are padded to hexagons)
  ErrorCode create_padded_local_cells(const std::vector<int>& vertices_on_local_cells,
                                      EntityHandle start_vertex, Range& faces);

  //! Create gather set vertices
  ErrorCode create_gather_set_vertices(EntityHandle gather_set, EntityHandle& gather_set_start_vertex);

  //! Create gather set edges (optional)
  ErrorCode create_gather_set_edges(EntityHandle gather_set, EntityHandle gather_set_start_vertex);

  //! Create gather set cells with padding (pentagons are padded to hexagons)
  ErrorCode create_padded_gather_set_cells(EntityHandle gather_set, EntityHandle gather_set_start_vertex);

private:
  bool createGatherSet;
  Range facesOwned;
};

} // namespace moab

#endif
