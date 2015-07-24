#include <iostream>
#include <stdlib.h>
#include <vector>
#include <set>
#include <string>
#include <stdio.h>
#include <iomanip>
#include "moab/MOABConfig.h"
#ifndef _WIN32
#  include <sys/times.h>
#  include <limits.h>
#  include <unistd.h>
#endif
#include <time.h>
#ifdef MOAB_HAVE_MPI
#  include "moab_mpi.h"
#endif
#if !defined(_MSC_VER) && !defined(__MINGW32__)
#  include <termios.h>
#  include <sys/ioctl.h>
#endif
#include <math.h>
#include <assert.h>
#include <float.h>

#include "moab/Core.hpp"
#include "moab/Range.hpp"
#include "MBTagConventions.hpp"
#include "moab/Interface.hpp"
#include "moab/ReaderWriterSet.hpp"

/* Exit values */
#define USAGE_ERROR 1
#define READ_ERROR 2
#define WRITE_ERROR 3
#define OTHER_ERROR 4
#define ENT_NOT_FOUND 5

using namespace moab;

#include "measure.hpp"

static void print_usage( const char* name, std::ostream& stream )
{
  stream << "Usage: " << name 
         << " <options> <input_file> [<input_file2> ...]" << std::endl
         << "Options: " << std::endl
         << "\t-f             - List available file formats and exit." << std::endl
         << "\t-g             - print counts by geometric owner" << std::endl
         << "\t-h             - Print this help text and exit." << std::endl
         << "\t-l             - Print counts of mesh" << std::endl
         << "\t-ll            - Verbose listing of every entity" << std::endl
         << "\t-m             - Print counts per block/boundary" << std::endl
         << "\t-O option      - Specify read option." << std::endl
#ifdef MOAB_HAVE_MPI
         << "\t-p[0|1|2]      - Read in parallel[0], optionally also doing resolve_shared_ents (1) and exchange_ghosts (2)" << std::endl
#endif
         << "\t-t             - Print counts by tag" << std::endl
         << "\t-T             - Time read of files." << std::endl
         << "\t--             - treat all subsequent options as file names" << std::endl
         << "\t                 (allows file names beginning with '-')" << std::endl
    ;
}

Core mb;


struct stat_set 
{
  double sum;
  double sqr;
  double min; 
  double max;
  long count;
  
  inline stat_set() : sum(0), sqr(0), min(HUGE_VAL), max(0), count (0) {}
  
  inline void add( double val )
  {
    if (val < min)
      min = val;
    if (val > max)
      max = val;
    sum += val;
    sqr += val*val;
    ++count;
  }
  
  inline void add( const stat_set& stats )
  {
    if (stats.min < min)
      min = stats.min;
    if (stats.max > max)
      max = stats.max;
    sum += stats.sum;
    sqr += stats.sqr;
    count += stats.count;
  }
  
  inline void clear()
  {
    sum = sqr = 0.0;
    max = count = 0;
    min = HUGE_VAL;
  }
};

struct set_stats {
  stat_set stats[MBMAXTYPE];
  stat_set edge_uses;
  size_t nodes;
  
  void add( const set_stats& other )
  {
    for (int i = 0; i < MBMAXTYPE; ++i)
      stats[i].add( other.stats[i] );
    edge_uses.add( other.edge_uses );
  }
  
  void clear()
  {
    for (int i = 0; i < MBMAXTYPE; ++i)
      stats[i].clear();
    edge_uses.clear();
  }
    
};


static ErrorCode gather_set_stats( EntityHandle set, set_stats& stats )
{
  ErrorCode rval = MB_SUCCESS;
  
  int count;
  rval = mb.get_number_entities_by_type( set, MBVERTEX, count );
  if (MB_SUCCESS != rval) return rval;
  stats.nodes = count;
  
  int edge_vtx_idx[2];
  std::vector<EntityHandle> conn;
  std::vector<double> coords;
  for (EntityType type = MBEDGE; type < MBENTITYSET; ++type)
  {
    int num_edges = CN::NumSubEntities( type, 1 );
    
    Range range;
    rval = mb.get_entities_by_type( set, type, range, true );
    if (MB_SUCCESS != rval) return rval;
    for (Range::iterator i = range.begin(); i != range.end(); ++i)
    {
      rval = mb.get_connectivity( &*i, 1, conn, true );
      if (MB_SUCCESS != rval) return rval;
      if (type == MBPOLYHEDRON) {
        std::vector<EntityHandle> dum_conn(conn);
        conn.clear();
        rval = mb.get_adjacencies(&dum_conn[0], dum_conn.size(), 0, false, conn, Interface::UNION);
        if (MB_SUCCESS != rval) return rval;
      }
      coords.resize( 3*conn.size() );
      rval = mb.get_coords( &conn[0], conn.size(), &coords[0] );
      if (MB_SUCCESS != rval) return rval;
      stats.stats[type].add( measure( type, conn.size(), &coords[0] ) );
      
      if (type != MBEDGE)
      {
        if (type == MBPOLYGON)
          num_edges = conn.size();

        for (int e = 0; e < num_edges; ++e)
        {
          if (type == MBPOLYGON) {
            edge_vtx_idx[0] = e;
            edge_vtx_idx[1] = (e+1)%num_edges;
          }
          else
            CN::SubEntityVertexIndices( type, 1, e, edge_vtx_idx );
          stats.edge_uses.add( edge_length( &coords[3*edge_vtx_idx[0]],
                                            &coords[3*edge_vtx_idx[1]] ) );
        }
      }
    }
  }
  return MB_SUCCESS;
}

struct TagCounts {
  TagCounts(std::string n) : name(n) 
    { std::fill(counts, counts+MBMAXTYPE, 0); }
  std::string name;
  int counts[MBMAXTYPE];
};

static ErrorCode gather_tag_counts( EntityHandle set, 
                                    std::vector<TagCounts>& counts )
{
  std::vector<Tag> tags;
  mb.tag_get_tags( tags );
  for (size_t i = 0; i < tags.size(); ++i) {
    std::string name;
    ErrorCode rval = mb.tag_get_name( tags[i], name );
    if (MB_SUCCESS != rval || name.empty())
      continue;
    
    counts.push_back( name );
    for (EntityType t = MBVERTEX; t != MBMAXTYPE; ++t) 
      mb.get_number_entities_by_type_and_tag( set, t, &tags[i], 0, 1, counts.back().counts[t] );
  }
  
  return MB_SUCCESS;
}

void add_tag_counts( std::vector<TagCounts>& counts,
                     const std::vector<TagCounts>& add )
{
  for (size_t i = 0; i < add.size(); ++i) {
    size_t j;
    for (j = 0; j < counts.size(); ++j)
      if (add[i].name == counts[j].name)
        break;
    if (j == counts.size()) {
      counts.push_back( add[i] );
      continue;
    }
    for (EntityType t = MBVERTEX; t != MBMAXTYPE; ++t) 
      counts[j].counts[t] += add[i].counts[t];
  }
}

static const char* dashes( unsigned count )
{
  static std::vector<char> dashes;
  dashes.clear();
  dashes.resize( count + 1, '-' );
  dashes[count] = '\0';
  return &dashes[0];
}

static void print_tag_counts( const std::vector<TagCounts>& counts )
{
  if (counts.empty()) {
    printf( "<No tags>\n");
    return;
  }

  int widths[MBMAXTYPE] = { 0 };
  int name_width = 0;
  for (size_t i = 0; i < counts.size(); ++i) {
    if (counts[i].name.length() > (unsigned)name_width)
      name_width = counts[i].name.length();
    for (EntityType t = MBVERTEX; t != MBMAXTYPE; ++t) 
      if (counts[i].counts[t] != 0)
        widths[t] = std::max(8,(int)strlen(CN::EntityTypeName(t)));
  }
  
  if (0 == std::min_element(widths, widths+MBMAXTYPE)) {
    printf( "<No Tagged Entities>\n");
    return;
  }
  
    // Print header line
  const char* name_title = "Tag Name";
  if ((unsigned)name_width < strlen(name_title))
    name_width = strlen(name_title);
  printf( "%*s", name_width, name_title );
  for (EntityType t = MBVERTEX; t != MBMAXTYPE; ++t) 
    if (widths[t])
      printf( " %*s", widths[t], CN::EntityTypeName(t) );
  printf("\n%s", dashes(name_width));
  for (EntityType t = MBVERTEX; t != MBMAXTYPE; ++t) 
    if (widths[t])
      printf( " %s", dashes(widths[t]) );
    printf("\n");
    
    // print data
  for (size_t i = 0; i < counts.size(); ++i) {
    printf( "%*s", name_width, counts[i].name.c_str() );
    for (EntityType t = MBVERTEX; t != MBMAXTYPE; ++t) 
      if (widths[t])
        printf( " %*d", widths[t], counts[i].counts[t] );
    printf("\n");
  }
}

static void print_stats( set_stats& stats )
{
  const char* edge_use_name = "1D Side";
  const char* vertex_name = "Vertex";
  
  bool have_some = stats.edge_uses.count > 0 || stats.nodes > 0;
  for (int i = 0; i < MBMAXTYPE; ++i)
    if (stats.stats[i].count > 0)
      have_some = true;
  
  if (!have_some)
  {
    std::cout << "NO MESH" << std::endl;
    return;
  }
  
    // get field widths
  unsigned type_width = std::max( strlen(vertex_name), strlen( edge_use_name ) );
  unsigned count_width = 5;
  unsigned total_width = 5;
  unsigned total_prec = 2;
  unsigned precision = 5;
  int total_log = -10000;
  
  unsigned node_count_width = (unsigned)(ceil(log10((double)stats.nodes))) + 1;
  if (count_width < node_count_width)
    count_width = node_count_width;
  
  for (EntityType i = MBEDGE; i < MBMAXTYPE; ++i)
  {
    stat_set& s = (i == MBMAXTYPE) ? stats.edge_uses : stats.stats[i];

    if (s.count == 0)
      continue;
    
    unsigned len = strlen(CN::EntityTypeName(i));
    if (len > type_width)
      type_width = len;
    
    unsigned cw = (unsigned)(ceil(log10((double)s.count))) + 1;
    if (cw > count_width)
      count_width = cw;
    
    int tl = (unsigned)(ceil(log10(fabs(s.sum)))) + 1;
    if (tl > total_log)
      total_log = tl;
  }
  
  if (total_log > (int)total_width || total_log == -10000)
  {
    total_width = 8;
    total_prec = 2;
  }
  else if (total_log <= -(int)total_width)
  {
    total_width = -total_log + 5;
    total_prec = 2;
  }
  else if (total_log < 1)
  {
    total_width = -total_log + 4;
    total_prec = -total_log + 1;
  }
  else
  {
    total_width += 2;
  }
    
  
  // get terminal width
  unsigned term_width = 80;
#if !defined(_MSC_VER) && !defined(__MINGW32__)
  struct winsize size;
  if ( ioctl( fileno(stdout), TIOCGWINSZ, (char*)&size ) == 0 )
    term_width = size.ws_col;
  if (!term_width) term_width = 80;
#endif
  assert(term_width > 7 + type_width + count_width + total_width);
  
  term_width -= 7; // spaces
  term_width -= type_width;
  term_width -= count_width;
  term_width -= total_width;
  unsigned val_width = term_width / 5;
  if (val_width < 8)
    val_width = 8;
  
  printf( "%*s %*s %*s %*s %*s %*s %*s %*s\n",
          type_width, "type",
          count_width, "count",
          total_width, "total",
          val_width, "minimum",
          val_width, "average",
          val_width, "rms",
          val_width, "maximum",
          val_width, "std.dev." );
  
  printf( "%*s ", type_width, dashes(type_width) );
  printf( "%*s ", count_width, dashes(count_width) );
  printf( "%*s ", total_width, dashes(total_width) );
  printf( "%*s ", val_width, dashes(val_width) );
  printf( "%*s ", val_width, dashes(val_width) );
  printf( "%*s ", val_width, dashes(val_width) );
  printf( "%*s ", val_width, dashes(val_width) );
  printf( "%*s\n", val_width, dashes(val_width) );
  
  for (EntityType i = MBEDGE; i <= MBMAXTYPE; ++i)
  {
    stat_set& s = (i == MBMAXTYPE) ? stats.edge_uses : stats.stats[i];
    
    if (s.count == 0)
      continue;

    double tmp_dbl = s.sqr / s.count - s.sum*s.sum / (double)s.count / (double)s.count;
    if (tmp_dbl < 0.0) {
      if (tmp_dbl < -100.0*DBL_EPSILON)
        std::cout << "WARNING: stat values dubious, s^2 - sig_s = " << tmp_dbl << std::endl;
      tmp_dbl = 0.0;
    }
    
    printf( "%*s %*ld %*.*g %*.*g %*.*g %*.*g %*.*g %*.*g\n",
            type_width, i == MBMAXTYPE ? edge_use_name : CN::EntityTypeName(i),
            count_width, s.count,
            total_width, total_prec, s.sum,
            val_width, precision, s.min,
            val_width, precision, s.sum / s.count,
            val_width, precision, sqrt( s.sqr / s.count ),
            val_width, precision, s.max,
            val_width, precision,  
            sqrt(tmp_dbl)
          );
  }
  printf( "%*s %*lu\n", type_width, vertex_name, count_width, (unsigned long)stats.nodes );
  
  puts("");
}

bool parse_id_list( const char* string, std::set<int>& results )
{
  bool okay = true;
  char* mystr = strdup( string );
  for (const char* ptr = strtok(mystr, ","); ptr; ptr = strtok(0,","))
  {
    char* endptr;
    long val = strtol( ptr, &endptr, 0 );
    if (endptr == ptr || val <= 0) {
      std::cerr << "Not a valid id: " << ptr << std::endl;
      okay = false;
      break;
    }
    
    long val2 = val;
    if (*endptr == '-') {
      const char* sptr = endptr+1;
      val2 = strtol( sptr, &endptr, 0 );
      if (endptr == sptr || val2 <= 0) {
        std::cerr << "Not a valid id: " << sptr << std::endl;
        okay = false;
        break;
      }
      if (val2 < val) {
        std::cerr << "Invalid id range: " << ptr << std::endl;
        okay = false;
        break;
      }
    }
    
    if (*endptr) {
      std::cerr << "Unexpected character: " << *endptr << std::endl;
      okay = false;
      break;
    }
    
    for (; val <= val2; ++val)
      if (!results.insert( (int)val ).second) 
        std::cerr << "Warning: duplicate Id: " << val << std::endl;

  }
  
  free( mystr );
  return okay;    
}

bool make_opts_string( std::vector<std::string> options, std::string& opts )
{
  opts.clear();
  if (options.empty())
    return true;

    // choose a separator character
  std::vector<std::string>::const_iterator i;
  char separator = '\0';
  const char* alt_separators = ";+,:\t\n";
  for (const char* sep_ptr = alt_separators; *sep_ptr; ++sep_ptr) {
    bool seen = false;
    for (i = options.begin(); i != options.end(); ++i)
      if (i->find( *sep_ptr, 0 ) != std::string::npos) {
        seen = true;
        break;
      }
    if (!seen) {
      separator = *sep_ptr;
      break;
    }
  }
  if (!separator) {
    std::cerr << "Error: cannot find separator character for options string" << std::endl;
    return false;
  }
  if (separator != ';') {
    opts = ";";
    opts += separator;
  }
  
    // concatenate options
  i = options.begin();
  opts += *i;
  for (++i; i != options.end(); ++i) {
    opts += separator;
    opts += *i;
  }

  return true;
}


void list_formats( Interface* gMB )
{
  const char iface_name[] = "ReaderWriterSet";
  ErrorCode err;
  ReaderWriterSet* set = 0;
  ReaderWriterSet::iterator i;
  std::ostream& str = std::cout;
    
    // get ReaderWriterSet
  err = gMB->query_interface( set );
  if (err != MB_SUCCESS || !set) {
    std::cerr << "Internal error:  Interface \"" << iface_name 
              << "\" not available.\n";
    exit(OTHER_ERROR);
  }
  
    // get field width for format description
  size_t w = 0;
  for (i = set->begin(); i != set->end(); ++i)
    if (i->description().length() > w)
      w = i->description().length();
  
    // write table header
  str << "Format  " << std::setw(w) << std::left << "Description"
      << "  Read  Write  File Name Suffixes\n"
      << "------  " << std::setw(w) << std::setfill('-') << "" << std::setfill(' ')
      << "  ----  -----  ------------------\n";
      
    // write table data
  for (i = set->begin(); i != set->end(); ++i)
  {
    std::vector<std::string> ext;
    i->get_extensions( ext );
    str << std::setw(6) << i->name() << "  " 
        << std::setw(w) << std::left << i->description() << "  "
        << (i->have_reader() ?  " yes" :  "  no") << "  "
        << (i->have_writer() ? "  yes" : "   no") << " ";
    for (std::vector<std::string>::iterator j = ext.begin(); j != ext.end(); ++j)
      str << " " << *j;
    str << std::endl;
  }
  str << std::endl;
  
  gMB->release_interface( set );
  exit(0);
}

static void usage_error( const char* name )
{
  print_usage( name, std::cerr );
#ifdef MOAB_HAVE_MPI
  MPI_Finalize();
#endif
  exit(USAGE_ERROR);
} 

static void print_time( int clk_per_sec, const char* prefix, clock_t ticks, std::ostream& stream )
{
  ticks *= clk_per_sec/100;
  clock_t centi = ticks % 100;
  clock_t seconds = ticks / 100;
  stream << prefix;
  if (seconds < 120)
  {
    stream << (ticks / 100) << "." << centi << "s" << std::endl;
  }
  else
  {
    clock_t minutes = (seconds / 60) % 60;
    clock_t hours = (seconds / 3600);
    seconds %= 60;
    if (hours)
      stream << hours << "h";
    if (minutes)
      stream << minutes << "m";
    if (seconds || centi)
      stream << seconds << "." << centi << "s";
    stream << " (" << (ticks/100) << "." << centi << "s)" << std::endl;
  }
}

clock_t usr_time, sys_time, abs_time;

#ifdef _WIN32

void reset_times() 
{
  abs_time = clock();
}


void write_times( std::ostream& stream ) 
{
  clock_t abs_tm = clock();
  print_time( CLOCKS_PER_SEC, "  ", abs_tm - abs_time, stream );
  abs_time = abs_tm;
}

#else

void reset_times()
{
  tms timebuf;
  abs_time = times( &timebuf );
  usr_time = timebuf.tms_utime;
  sys_time = timebuf.tms_stime;
}

void write_times( std::ostream& stream )
{
  clock_t usr_tm, sys_tm, abs_tm;
  tms timebuf;
  abs_tm = times( &timebuf );
  usr_tm = timebuf.tms_utime;
  sys_tm = timebuf.tms_stime;
  print_time( sysconf(_SC_CLK_TCK), "  real:   ", abs_tm - abs_time, stream );
  print_time( sysconf(_SC_CLK_TCK), "  user:   ", usr_tm - usr_time, stream );
  print_time( sysconf(_SC_CLK_TCK), "  system: ", sys_tm - sys_time, stream );
  abs_time = abs_tm;
  usr_time = usr_tm;
  sys_time = sys_tm;
}

#endif

const char* geom_type_names[] = { "Vertex", "Curve", "Surface", "Volume" } ;
const char* mesh_type_names[] = { "Dirichlet Set", "Neumann Set", "Material Set" };
const char* mesh_type_tags[] = { DIRICHLET_SET_TAG_NAME, NEUMANN_SET_TAG_NAME, MATERIAL_SET_TAG_NAME };

int main( int argc, char* argv[] )
{
  bool geom_owners = false;
  bool mesh_owners = false;
  bool just_list = false;
  bool just_list_basic = false;
  bool tag_count = false;
  std::vector<std::string> file_list;
  set_stats total_stats, file_stats;
  std::vector<TagCounts> total_counts, file_counts;
  ErrorCode rval;
  
  Range range;

  int i;
  std::vector<std::string> read_opts;
  
  int proc_id = 0;
#ifdef MOAB_HAVE_MPI
  int initd = 0;
  MPI_Initialized(&initd);
  if (!initd) MPI_Init(&argc,&argv);
  MPI_Comm_rank( MPI_COMM_WORLD, &proc_id );
#endif

    // scan arguments
  bool do_flag = true;
  bool print_times = false;
  bool parallel = false, resolve_shared = false, exchange_ghosts = false;
  bool printed_usage = false;
  for (i = 1; i < argc; i++)
  {
    if (!argv[i][0])
      usage_error(argv[0]);
      
    if (do_flag && argv[i][0] == '-')
    {
      switch ( argv[i][1] )
      {
          // do flag arguments:
        case '-': do_flag = false;       break;
        case 'T': print_times = true;    break;
        case 'h': 
        case 'H': print_usage( argv[0], std::cerr ); printed_usage = true; break;
        case 'f': list_formats( &mb );   break;
        case 'l': 
            if (strlen(argv[i]) == 2)
              just_list_basic = true;
            else if (strlen(argv[i]) == 3 && argv[i][2] == 'l')
              just_list = true;
            break;
#ifdef MOAB_HAVE_MPI
        case 'p':
            parallel = true;
            if (argv[i][2] == '1' || argv[i][2] == '2') resolve_shared = true;
            if (argv[i][2] == '2') exchange_ghosts = true;
            break;
#endif
        case 'g': geom_owners = true; break;
        case 'm': mesh_owners = true; break;
        case 't': tag_count = true; break;
        default: 
            ++i;
            switch ( argv[i-1][1] )
            {
              case 'O':  read_opts.push_back(argv[i]); break;
              default: std::cerr << "Invalid option: " << argv[i] << std::endl;
            }
          
      }
    }
      // do file names
    else {
      file_list.push_back( argv[i] );
    }
  }
    
    // construct options string from individual options
  std::string read_options;
  if (parallel) {
    read_opts.push_back("PARALLEL=READ_PART");
    read_opts.push_back("PARTITION=PARALLEL_PARTITION");
  }
  if (resolve_shared) read_opts.push_back("PARALLEL_RESOLVE_SHARED_ENTS");
  if (exchange_ghosts) read_opts.push_back("PARALLEL_GHOSTS=3.0.1");
  
  if (!make_opts_string(  read_opts,  read_options )) 
  {
#ifdef MOAB_HAVE_MPI
    MPI_Finalize();
#endif
    return USAGE_ERROR;
  }
  
  if (file_list.empty() && !printed_usage)
    print_usage(argv[0], std::cerr);
  
  for (std::vector<std::string>::iterator f = file_list.begin(); f != file_list.end(); ++f)
  {
    reset_times();
    printf("File %s:\n", f->c_str() );
    if (MB_SUCCESS != mb.load_file( f->c_str(), NULL, read_options.c_str() ))
    {
      fprintf(stderr, "Error reading file: %s\n", f->c_str() );
      return 1;
    }
    
    if (tag_count)
      rval = gather_tag_counts( 0, file_counts );
    else if (!just_list)
      rval = gather_set_stats( 0, file_stats );
    else
      rval = MB_SUCCESS;

    if (MB_SUCCESS != rval)
    {
      fprintf(stderr, "Error processing mesh from file: %s\n", f->c_str());
      return 1;
    }
    
    if (tag_count) {
      add_tag_counts( total_counts, file_counts );
      print_tag_counts( file_counts );
      file_counts.clear();
    }
    else if (just_list) {
      mb.list_entities( 0, -1 );
    }
    else {
      total_stats.add( file_stats );
      print_stats( file_stats );
      file_stats.clear();
    }
    
    if (geom_owners)
    {
      Range entities;
      Tag dim_tag = 0, id_tag = 0;
      rval = mb.tag_get_handle( GEOM_DIMENSION_TAG_NAME, 1, MB_TYPE_INTEGER, dim_tag );
      if (MB_TAG_NOT_FOUND == rval) 
      {
        fprintf( stderr, "No geometry tag defined.\n" );
      }
      else if (MB_SUCCESS != rval)
      {
        fprintf( stderr, "Error retreiving geometry tag.\n");
        return 2;
      }
      
      rval = mb.tag_get_handle( GLOBAL_ID_TAG_NAME, 1, MB_TYPE_INTEGER, id_tag );
      if (MB_TAG_NOT_FOUND == rval) 
      {
        fprintf( stderr, "No ID tag defined.\n" );
      }
      else if (MB_SUCCESS != rval)
      {
        fprintf( stderr, "Error retreiving ID tag.\n");
        return 2;
      }
      
      if (dim_tag && id_tag)
      {
        if (MB_SUCCESS != mb.get_entities_by_type_and_tag( 0, 
                                                        MBENTITYSET, 
                                                        &dim_tag,
                                                        0,
                                                        1,
                                                        entities ))
        {
          fprintf( stderr, "Error retreiving geometry entitities.\n" );
        }
      }
      
      if (entities.empty())
      {
        fprintf( stderr, "No geometry entities defined in file.\n" );
      }
      
      for (Range::iterator rit = entities.begin(); rit != entities.end(); ++rit)
      {
        int id = 0, dim = 0;
        if (MB_SUCCESS != mb.tag_get_data( dim_tag, &*rit, 1, &dim ) ||
            MB_SUCCESS != mb.tag_get_data(  id_tag, &*rit, 1,  &id ))
        {
          fprintf( stderr, "Error retreiving tag data for geometry entity.\n");
          continue;
        }
        
        printf( "%s %d:\n", geom_type_names[dim], id );
        if (tag_count)
          rval = gather_tag_counts( *rit, file_counts );
        else if (!just_list && !just_list_basic)
          rval = gather_set_stats( *rit, file_stats );
        
        if (MB_SUCCESS != rval)
          fprintf(stderr, "Error processing mesh from file: %s\n", f->c_str());
        else if (tag_count)
          print_tag_counts( file_counts );
        else if (just_list)
          mb.list_entities( 0, 1 );
        else if (just_list_basic)
          mb.list_entities( 0, 0 );
        else
          print_stats( file_stats );

        file_stats.clear();
        file_counts.clear();
      }
    }


    if (mesh_owners)
    {
      for (int t = 0; t < 3; ++t)
      {
        Range entities;
        Tag tag = 0;
        rval = mb.tag_get_handle( mesh_type_tags[t], 1, MB_TYPE_INTEGER, tag );
        if (MB_TAG_NOT_FOUND == rval) 
        {
          continue;
        }
        else if (MB_SUCCESS != rval)
        {
          fprintf( stderr, "Error retreiving %s tag.\n", mesh_type_tags[t]);
          return 2;
        }
      
        if (MB_SUCCESS != mb.get_entities_by_type_and_tag( 0, 
                                                        MBENTITYSET, 
                                                        &tag,
                                                        0,
                                                        1,
                                                        entities ))
        {
          fprintf( stderr, "Error retreiving %s entitities.\n", mesh_type_names[t] );
          continue;
        }
      
        for (Range::iterator rit = entities.begin(); rit != entities.end(); ++rit)
        {
          int id = 0;
          if (MB_SUCCESS != mb.tag_get_data( tag, &*rit, 1, &id ))
          {
            fprintf( stderr, "Error retreiving tag data for %s entity.\n", mesh_type_names[t]);
            continue;
          }

          printf( "%s %d:\n", mesh_type_names[t], id );
          if (tag_count) {
            rval = gather_tag_counts( *rit, file_counts );
            if (MB_SUCCESS != rval) 
              fprintf(stderr, "Error processing tags from file: %s\n", f->c_str());
            else
              print_tag_counts( file_counts );
          }
          else if (just_list)
            mb.list_entities( 0, 1 );
          else if (just_list_basic)
            mb.list_entities( 0, 0 );
          else if (!just_list && !just_list_basic) {
            rval = gather_set_stats( *rit, file_stats );

            if (rval != MB_SUCCESS)
              fprintf(stderr, "Error processing mesh from file: %s\n", f->c_str());
            else
              print_stats( file_stats );
          }
          file_stats.clear();
          file_counts.clear();
        }
      }
    }
    
    if (print_times && !proc_id) write_times( std::cout );
    mb.delete_mesh();
  }
  
  if (file_list.size() > 1 && !just_list && !just_list_basic)
  {
    printf("Total for all files:\n");
    if (tag_count)
      print_tag_counts( total_counts );
    else
      print_stats( total_stats );
  }
  
  return 0;
}

  
