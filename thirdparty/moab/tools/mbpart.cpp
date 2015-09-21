#include "moab/Core.hpp"
#include "moab/ProgOptions.hpp"
#include "moab/ReorderTool.hpp"

#ifdef MOAB_HAVE_ZOLTAN
#include "moab/ZoltanPartitioner.hpp"

#ifdef MOAB_HAVE_CGM
#include "InitCGMA.hpp"
#include "CubitCompat.hpp"
#endif

#endif

#ifdef MOAB_HAVE_METIS
#include "moab/MetisPartitioner.hpp"
#endif

#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <list>
#include <time.h>

using namespace moab;

const char DEFAULT_TAGGEDSETS_TAG[] = "PARALLEL_PARTITION";

const char DEFAULT_ZOLTAN_METHOD[] = "RCB";
#ifdef MOAB_HAVE_ZOLTAN
const char ZOLTAN_PARMETIS_METHOD[] = "PARMETIS";
const char ZOLTAN_OCTPART_METHOD[] = "OCTPART";
#endif

const char METIS_DEFAULT_METHOD[] = "ML_KWAY";
#ifdef MOAB_HAVE_METIS
const char METIS_ALTERNATIVE_METHOD[] = "ML_RB";
#endif

const char BRIEF_DESC[] = "Use Zoltan or Metis to partition MOAB meshes for use on parallel computers";
std::ostringstream LONG_DESC;

int main(int argc, char* argv[])
{
  int err = MPI_Init(&argc, &argv);
  if (err)
  {
    std::cerr << "MPI_Init failed.  Aborting." << std::endl;
    return 3;
  }

  Core moab;
  Interface& mb = moab;
  std::vector<int> set_l;

#ifdef MOAB_HAVE_ZOLTAN
  bool moab_use_zoltan=false;
#endif
#ifdef MOAB_HAVE_METIS
  bool moab_use_metis=false;
#endif

  LONG_DESC << "This utility invokes the ZoltanPartitioner or MetisPartitioner component of MOAB/CGM"
            "to partition a mesh/geometry." << std::endl
            << "If no partitioning method is specified, the defaults are: "
            << "for Zoltan=\"" << DEFAULT_ZOLTAN_METHOD 
            << "\" and Metis=\"" << METIS_DEFAULT_METHOD 
            << " method" << std::endl;

  ProgOptions opts(LONG_DESC.str(), BRIEF_DESC);

  int part_dim = 3;
  opts.addOpt<int>("dimension", "Specify dimension of entities to partition."
                   "  Default is  largest in file.", &part_dim, ProgOptions::int_flag);

  std::string zoltan_method, parm_method, oct_method, metis_method;
#ifdef MOAB_HAVE_ZOLTAN
  opts.addOpt<std::string>("zoltan,z", "(Zoltan) Specify Zoltan partition method.  "
                           "One of RR, RCB, RIB, HFSC, PHG, or Hypergraph (PHG and Hypergraph "
                           "are synonymous).", &zoltan_method);
#ifdef MOAB_HAVE_PARMETIS
  opts.addOpt<std::string>("parmetis,p", "(Zoltan+PARMetis) Specify PARMetis partition method.", &parm_method);
#endif // MOAB_HAVE_PARMETIS
  opts.addOpt<std::string>("octpart,o", "(Zoltan) Specify OctPart partition method.", &oct_method);

  bool incl_closure = false;
  opts.addOpt<void>("include_closure,c", "Include element closure for part sets.", &incl_closure);
#endif // MOAB_HAVE_ZOLTAN

  double imbal_tol = 1.03;
  opts.addOpt<double>("imbalance,i", "Imbalance tolerance (used in PHG/Hypergraph method)", &imbal_tol);

#ifdef MOAB_HAVE_METIS
  opts.addOpt<std::string>( "metis,m", "(Metis) Specify Metis partition method. One of ML_RB or ML_KWAY.", &metis_method);
#endif // MOAB_HAVE_METIS

  bool write_sets = true, write_tags = false;
  opts.addOpt<void>( "sets,s", "Write partition as tagged sets (Default)", &write_sets);
  opts.addOpt<void>( "tags,t",  "Write partition by tagging entities", &write_tags);

  int power = -1;
  opts.addOpt<int>("power,M", "Generate multiple partitions, in powers of 2, up to 2^(pow)", &power);

  bool reorder = false;
  opts.addOpt<void>("reorder,R", "Reorder mesh to group entities by partition", &reorder);

  double part_geom_mesh_size = -1.0;
#if MOAB_HAVE_ZOLTAN
  bool part_surf = false;
#ifdef MOAB_HAVE_CGM
  opts.addOpt<double>("geom,g", "(CGM) If partition geometry, specify mesh size.", &part_geom_mesh_size);
  opts.addOpt<void>("surf,f", "(CGM) Specify if partition geometry surface.", &part_surf);
#endif // MOAB_HAVE_CGM

  bool ghost = false;
  opts.addOpt<void>("ghost,H", "Specify if partition ghost geometry body.");

  int obj_weight = 0;
  opts.addOpt<int>("vertex_w,v", "Number of weights associated with a graph vertex.");

  int edge_weight = 0;
  opts.addOpt<int>("edge_w,e", "Number of weights associated with an edge.");

#endif // MOAB_HAVE_ZOLTAN

  long num_parts;
  opts.addOpt<std::vector<int> >("set_l,l", "Load material set(s) with specified ids (comma seperated) for partition");

  opts.addRequiredArg<int>("#parts", "Number of parts in partition");

  std::string input_file, output_file;
  opts.addRequiredArg<std::string>("input_file", "Mesh/geometry to partition", &input_file);
  opts.addRequiredArg<std::string>("output_file", "File to which to write partitioned mesh/geometry", &output_file);

  bool print_time = false;
  opts.addOpt<void>(",T", "Print CPU time for each phase.", &print_time);

  bool spherical_coords = false;
  opts.addOpt<void>("project_on_sphere,s", "use spherical coordinates for partitioning ", &spherical_coords);

#ifdef MOAB_HAVE_METIS
  bool partition_tagged_sets = false;
  opts.addOpt<void>( "taggedsets,x", "(Metis) Partition tagged sets.", &partition_tagged_sets);

  bool partition_tagged_ents = false;
  opts.addOpt<void>( "taggedents,y", "(Metis) Partition tagged ents.", &partition_tagged_ents);

  std::string aggregating_tag;
  opts.addOpt<std::string>( "aggregatingtag,a", "(Metis) Specify aggregating tag to partion tagged sets or tagged entities.", &aggregating_tag);

  std::string aggregating_bc_tag;
  opts.addOpt<std::string>( "aggregatingBCtag,B", "(Metis) Specify boundary id tag name used to group cells with same boundary ids.", &aggregating_bc_tag);

  std::string boundaryIds;
  std::vector<int> BCids;
  opts.addOpt<std::string>( "aggregatingBCids,I", " (Metis) Specify id or ids of boundaries to be aggregated before partitioning (all elements with same boundary id will be in the same partition). Comma separated e.g. -I 1,2,5 ", &boundaryIds);
#endif // MOAB_HAVE_METIS

  opts.parseCommandLine(argc, argv);

#ifdef MOAB_HAVE_ZOLTAN
  if (!zoltan_method.empty())
    moab_use_zoltan=true;
  else
#endif
#ifdef MOAB_HAVE_METIS
    if (!metis_method.empty())
      moab_use_metis=true;
    else
#endif
  MB_SET_ERR(MB_FAILURE, "Specify either Zoltan or Metis partitioner type");

  PartitionerBase *tool = NULL;

#ifdef MOAB_HAVE_ZOLTAN
  // check if partition geometry, if it is, should get mesh size for the geometry
  if (part_geom_mesh_size != -1.0 && part_geom_mesh_size <= 0.0)
  {
    std::cerr << part_geom_mesh_size
              << ": invalid geometry partition mesh size." << std::endl;
    return 1;
  }

  if (moab_use_zoltan) {
    if (part_geom_mesh_size < 0.)
    {
      // partition mesh
      tool = new ZoltanPartitioner(&mb, false, argc, argv);
    }
    else
    {
      // partition geometry
#ifdef MOAB_HAVE_CGM
      CubitStatus status = InitCGMA::initialize_cgma();
      if (CUBIT_SUCCESS != status)
      {
        std::cerr << "CGM couldn't be initialized." << std::endl;
        return 1;
      }
      GeometryQueryTool *gti = GeometryQueryTool::instance();
      tool = new ZoltanPartitioner (&mb, false, argc, argv, gti);
#else
      std::cerr << "CGM should be configured to partition geometry." << std::endl;
      return 1;
#endif // MOAB_HAVE_CGM
    }
  }

  if (zoltan_method.empty() && parm_method.empty() && oct_method.empty())
    zoltan_method = DEFAULT_ZOLTAN_METHOD;
  if (!parm_method.empty())
    zoltan_method = ZOLTAN_PARMETIS_METHOD;
  if (!oct_method.empty())
    zoltan_method = ZOLTAN_OCTPART_METHOD;
#endif // MOAB_HAVE_ZOLTAN

#ifdef MOAB_HAVE_METIS
  if (moab_use_metis && !tool) {
    tool = new MetisPartitioner (&mb, false, argc, argv);
  }

  if ((aggregating_tag.empty() && partition_tagged_sets) || (aggregating_tag.empty() && partition_tagged_ents))
    aggregating_tag = DEFAULT_TAGGEDSETS_TAG;
  if (!write_sets && !write_tags)
    write_sets = true;

  if (!boundaryIds.empty())
  {
    std::vector<std::string> ids;
    std::stringstream ss(boundaryIds);
    std::string item;
    while (std::getline(ss, item, ',')) {
      ids.push_back(item);
    }
    for (unsigned int i = 0; i < ids.size(); i++)
      BCids.push_back(std::atoi(ids[i].c_str()));
  }

  if (metis_method.empty()) {
    metis_method = METIS_DEFAULT_METHOD;
  }
#endif // MOAB_HAVE_METIS

  if (!write_sets && !write_tags)
    write_sets = true;

  if (-1 == power)
  {
    num_parts = opts.getReqArg<int>("#parts");
    power = 1;
  }
  else if (power < 1 || power > 18)
  {
    std::cerr << power
              << ": invalid power for multiple partitions. Expected value in [1,18]"
              << std::endl;
    return 1;
  }
  else
  {
    num_parts = 2;
  }

  if (part_dim < 0 || part_dim > 3)
  {
    std::cerr << part_dim << " : invalid dimension" << std::endl;
    return 1;
  }

  if (imbal_tol < 0.0)
  {
    std::cerr << imbal_tol << ": invalid imbalance tolerance" << std::endl;
    return 1;
  }

  bool load_msets = false;
  if (opts.getOpt("set_l,l", &set_l))
  {
    load_msets = true;
    if (set_l.size() <= 0)
    {
      std::cerr << " No material set id's to load" << std::endl;
      return 1;
    }
  }

  clock_t t = clock();

  const char* options = NULL;
  ErrorCode rval;
#ifdef MOAB_HAVE_ZOLTAN
  if (part_geom_mesh_size > 0.)
    options = "FACET_DISTANCE_TOLERANCE=0.1";
#endif // MOAB_HAVE_ZOLTAN

  std::cout << "Loading file " << input_file << "..." << std::endl;
  if (load_msets == false)
  {
    rval = mb.load_file(input_file.c_str(), 0, options);
    if (MB_SUCCESS != rval)
    {
      std::cerr << input_file << " : failed to read file." << std::endl;
      std::cerr << "  Error code: " << mb.get_error_string(rval) << " (" << rval
                << ")" << std::endl;
      std::string errstr;
      mb.get_last_error(errstr);
      if (!errstr.empty())
        std::cerr << "  Error message: " << errstr << std::endl;
      return 2;
    }
  }
  // load the material set(s)
  else
  {
    rval = mb.load_mesh(input_file.c_str(), &set_l[0], (int) set_l.size());
    if (MB_SUCCESS != rval)
    {
      std::cerr << input_file << " : failed to read file." << std::endl;
      std::cerr << "  Error code: " << mb.get_error_string(rval) << " (" << rval
                << ")" << std::endl;
      std::string errstr;
      mb.get_last_error(errstr);
      if (!errstr.empty())
        std::cerr << "  Error message: " << errstr << std::endl;
      return 2;
    }
  }
  if (print_time)
    std::cout << "Read input file in "
              << (clock() - t) / (double) CLOCKS_PER_SEC << " seconds" << std::endl;

  for (int dim = part_dim; dim >= 0; --dim)
  {
    int n;
    rval = mb.get_number_entities_by_dimension(0, dim, n);
    if (MB_SUCCESS == rval && 0 != n)
    {
      part_dim = dim;
      break;
    }
  }
  if (part_dim < 0)
  {
    std::cerr << input_file << " : file does not contain any mesh entities" << std::endl;
    return 2;
  }

  ReorderTool reorder_tool(&moab);

  for (int p = 0; p < power; p++)
  {
    t = clock();
#ifdef MOAB_HAVE_ZOLTAN
    if (moab_use_zoltan) {
      rval = tool->partition_mesh_and_geometry(part_geom_mesh_size, num_parts,
             zoltan_method.c_str(),
             (!parm_method.empty() ? parm_method.c_str() : oct_method.c_str()),
             imbal_tol, part_dim, write_sets, write_tags, obj_weight,
             edge_weight, part_surf, ghost, spherical_coords, print_time);
    }
#endif
#ifdef MOAB_HAVE_METIS
    if (moab_use_metis) {
      rval = tool->partition_mesh( num_parts, metis_method.c_str(), part_dim,
                                   write_sets, write_tags,
                                   partition_tagged_sets, partition_tagged_ents,
                                   aggregating_tag.c_str(), print_time);
    }
#endif
    if (MB_SUCCESS != rval)
    {
      std::cerr << "Partitioner failed!" << std::endl;
      std::cerr << "  Error code: " << mb.get_error_string(rval) << " (" << rval
                << ")" << std::endl;
      std::string errstr;
      mb.get_last_error(errstr);
      if (!errstr.empty())
        std::cerr << "  Error message: " << errstr << std::endl;
      return 3;
    }
    if (print_time)
      std::cout << "Generated " << num_parts << " part partitioning in "
                << (clock() - t) / (double) CLOCKS_PER_SEC << " seconds" << std::endl;

    if (reorder && part_geom_mesh_size < 0.)
    {
      std::cout << "Reordering mesh for partition..." << std::endl;

      Tag tag, order;
      rval = mb.tag_get_handle(DEFAULT_TAGGEDSETS_TAG, 1, MB_TYPE_INTEGER, tag);
      if (MB_SUCCESS != rval)
      {
        std::cerr << "Partitioner did not create " << DEFAULT_TAGGEDSETS_TAG << " tag" << std::endl;
        return 2;
      }

      t = clock();
      if (write_sets)
      {
        Range sets;
        mb.get_entities_by_type_and_tag(0, MBENTITYSET, &tag, 0, 1, sets);
        rval = reorder_tool.handle_order_from_sets_and_adj(sets, order);
      }
      else
      {
        rval = reorder_tool.handle_order_from_int_tag(tag, -1, order);
      }
      if (MB_SUCCESS != rval)
      {
        std::cerr << "Failed to calculate reordering!" << std::endl;
        return 2;
      }

      rval = reorder_tool.reorder_entities(order);
      if (MB_SUCCESS != rval)
      {
        std::cerr << "Failed to perform reordering!" << std::endl;
        return 2;
      }

      mb.tag_delete(order);
      if (print_time)
        std::cout << "Reordered mesh in "
                  << (clock() - t) / (double) CLOCKS_PER_SEC << " seconds"
                  << std::endl;
    }

#ifdef MOAB_HAVE_ZOLTAN
    if (incl_closure)
    {
      rval = tool->include_closure();
      if (MB_SUCCESS != rval)
      {
        std::cerr << "Closure inclusion failed." << std::endl;
        return 1;
      }
    }
#endif

    std::ostringstream tmp_output_file;

    if (power > 1)
    {
      // append num_parts to output filename
      std::string::size_type idx = output_file.find_last_of(".");
      if (idx == std::string::npos)
      {
        tmp_output_file << output_file << "_" << num_parts;
        if (part_geom_mesh_size < 0.)
          tmp_output_file << ".h5m";
        else
        {
          std::cerr << "output file type is not specified." << std::endl;
          return 1;
        }
      }
      else
      {
        tmp_output_file << output_file.substr(0, idx) << "_" << num_parts
                        << output_file.substr(idx);
      }
    }
    else
      tmp_output_file << output_file;

    t = clock();
    std::cout << "Saving file to " << output_file << "..." << std::endl;
    if (part_geom_mesh_size < 0.)
    {
      rval = mb.write_file(tmp_output_file.str().c_str());
      if (MB_SUCCESS != rval)
      {
        std::cerr << tmp_output_file << " : failed to write file." << std::endl;
        std::cerr << "  Error code: " << mb.get_error_string(rval) << " ("
                  << rval << ")" << std::endl;
        std::string errstr;
        mb.get_last_error(errstr);
        if (!errstr.empty())
          std::cerr << "  Error message: " << errstr << std::endl;
        return 2;
      }
    }
#ifdef MOAB_HAVE_ZOLTAN
#ifdef MOAB_HAVE_CGM
    else
    {
      std::string::size_type idx = output_file.find_last_of( "." );
      int c_size = output_file.length() - idx;
      const char* file_type = NULL;
      if (output_file.compare(idx, c_size, ".occ") == 0
          || output_file.compare(idx, c_size, ".OCC") == 0) file_type = "OCC";
      else if (output_file.compare(idx, c_size, ".sab") == 0) file_type = "ACIS_SAB";
      else if (output_file.compare(idx, c_size, ".sat") == 0) file_type = "ACIS_SAT";
      else
      {
        std::cerr << "File type for " << output_file.c_str() << " not supported." << std::endl;
        return 1;
      }

      int num_ents_exported=0;
      DLIList<RefEntity*> ref_entity_list;
      CubitStatus status = CubitCompat_export_solid_model(ref_entity_list,
                           tmp_output_file.str().c_str(),
                           file_type, num_ents_exported,
                           CubitString(__FILE__));
      if (CUBIT_SUCCESS != status)
      {
        std::cerr << "CGM couldn't export models." << std::endl;
        return 1;
      }
    }
#endif
#endif

    if (print_time)
      std::cout << "Wrote \"" << tmp_output_file.str() << "\" in "
                << (clock() - t) / (double) CLOCKS_PER_SEC << " seconds" << std::endl;

    num_parts *= 2;
  }

  delete tool;

  return 0;
}
