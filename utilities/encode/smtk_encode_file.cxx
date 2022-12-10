#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <locale> // for std::isalpha, std::isdigit
#include <regex>
#include <sstream>
#include <string>

static bool debug = false;
static std::size_t pieceLen = 16383; // = 2^14 - 1

int usage(const std::string& message, int code)
{
  std::cout << R"(Usage:
    smtk_encode_file [TYPE <type>] [EXT <ext>] [INCLUDE_DIRS <dir>…]
      [NAME <name>] [PIECE_SIZE <size>] [HEADER_OUTPUT <output>]
      [DEBUG] <input>
where:
    <type>    is one of "py", "json", "svg", "xml", "c", or "cpp".
    <ext>     is a filename extension to append to the output
              filename (ignored when HEADER_OUTPUT is present).
    <dir>…    is one or more include directories, indicating
              that #include directives (or <include/> directives
              for TYPE xml) should be processed.
              No include processing occurs unless at least one
              search directory is provided; use "INCLUDE_DIRS ."
              to search only the current working directory.
    <name>    is a valid C/C++ identifier to use as the variable
              name or function name that produces the contents of
              the encoded file.
    <size>    is the number of bytes allowed per string literal.
    <output>  is the path to the output file to generate.
    <input>   is the path to the file to encode.

Notes:
    1. If HEADER_OUTPUT is specified, the given output filename is used.
       Otherwise, the stem of the input file plus any type and extension
       are combined. For example, an input `/path/to/foo.xml`, a TYPE
       `xml`, and an EXT `h` will produce `foo_xml.h` as output.
    2. If NAME is specified, then it is used as the variable or function
       name. Otherwise, the stem of the input filename plus any type
       are combined into a variable name. For example, an input
       `/path/to/foo.xml` and a TYPE of `xml` produce `foo_xml`.
       Note that whatever name is used will be sanitized to produce
       a valid C/C++ identifier by omitting invalid characters.
       If no valid characters are present, an underscore (`_`) is used.
    3. In general, the order of options is not important **except**
       that INCLUDE_DIRS must not immediately precede the <input>
       filename.
    4. The default <size> is 16383.
    4. If DEBUG is specified, then informational messages will be
       printed to the terminal.

)" << message
            << "\n";

  return code;
}

inline bool isStart(char xx)
{
  return std::isalpha(xx) || xx == '_';
}

inline bool isContinue(char xx)
{
  return std::isdigit(xx) || isStart(xx);
}

void addPaths(std::vector<std::string>& includeDirs, const std::string& paths)
{
  std::size_t last = 0;
  std::size_t where = paths.find(';');
  for (; where != std::string::npos; last = where + 1, where = paths.find(';', last))
  {
    auto dir = paths.substr(last, where);
    if (!dir.empty())
    {
      includeDirs.push_back(dir);
    }
  }
  if (last < paths.size())
  {
    includeDirs.push_back(paths.substr(last));
  }
}

std::string sanitizeIdentifier(const std::string& identifier)
{
  bool first = true;
  std::ostringstream result;
  for (const auto& character : identifier)
  {
    if ((first && isStart(character)) || (!first && isContinue(character)))
    {
      result << character;
      first = false;
    }
  }
  if (result.str().empty())
  {
    result << "_";
  }
  return result.str();
}

bool isKeyword(const std::string& word)
{
  return word == "TYPE" || word == "EXT" || word == "INCLUDE_DIRS" || word == "NAME" ||
    word == "PIECE_SIZE" || word == "HEADER_OUTPUT" || word == "DEBUG";
}

bool readFile(std::string& contents, const std::string& inputFilename)
{
  std::ifstream file(inputFilename.c_str());
  if (!file.good())
  {
    return false;
  }
  contents =
    std::string((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
  return true;
}

bool writeFile(const std::string& contents, const std::string& outputFilename)
{
  auto outdir = boost::filesystem::path(outputFilename).parent_path();
  if (!outdir.empty() && !boost::filesystem::exists(outdir))
  {
    if (debug)
    {
      std::cout << "No directory \"" << outdir.string() << "\" exists; creating.\n";
    }
    if (!boost::filesystem::create_directories(outdir))
    {
      std::cerr << "  ERROR: Could not create \"" << outdir.string() << "\".\n";
      return false;
    }
  }
  std::ofstream file(outputFilename.c_str());
  if (!file.good())
  {
    return false;
  }
  file << contents;
  file.close();
  return true;
}

std::string expandIncludesInternal(
  const std::string& contents,
  const std::vector<std::string>& includeDirs,
  std::vector<std::string>& visiting,
  bool& ok)
{
  std::string result;
  std::size_t last = 0;
  if (debug)
  {
    std::cout << "      last = " << std::hex << last << std::dec << "\n";
  }
  using path = boost::filesystem::path;
  std::regex directives("<include[^/]* href=\"[^\"]+\"[^/]*/>");
  std::sregex_iterator begin(contents.begin(), contents.end(), directives);
  for (std::sregex_iterator dd = begin; dd != std::sregex_iterator(); ++dd)
  {
    // We found an include directive at dd->position().
    // Extract the URL.
    std::regex directive("href=\"([^\"]+)\"");
    std::smatch url;
    std::string include = dd->str();
    path absPath;
    result += contents.substr(last, dd->position() - last);
    if (std::regex_search(include, url, directive))
    {
      if (debug)
      {
        std::cout << "  include " << include << " @ " << dd->position() << " URL " << url[1]
                  << "\n";
      }
      std::string incContents;
      bool found = false;
      for (const auto& includeDir : includeDirs)
      {
        absPath = includeDir;
        absPath += "/";
        absPath += url[1].str();
        if (readFile(incContents, absPath.string()))
        {
          found = true;
          if (debug)
          {
            std::cout << "  Tried \"" << absPath.string() << "\" FOUND.\n\n";
          }
          break;
        }
        else if (debug)
        {
          std::cout << "  Tried \"" << absPath.string() << "\" but missing.\n";
        }
      }
      if (!found)
      {
        ok = false;
        std::cerr << "ERROR: Could not find \"" << url[1] << "\" "
                  << "referenced from " << dd->position() << "\n";
        return std::string();
      }
      visiting.push_back(absPath.string());
      incContents = expandIncludesInternal(incContents, includeDirs, visiting, ok);
      visiting.pop_back();
      result += incContents;
    }
    else
    {
      ok = false;
      std::cerr << "ERROR: include directive \"" << dd->str() << "\" at offset " << dd->position()
                << " is invalid.\n";
      return std::string();
    }
    last = dd->position() + include.size();
    if (debug)
    {
      std::cout << "      last = " << std::hex << last << std::dec << "\n";
    }
  }
  if (debug)
  {
    std::cout << "      last = " << std::hex << last << std::dec << "*\n";
  }
  result += contents.substr(last);
  return result;
}

std::string
expandIncludes(const std::string& contents, const std::vector<std::string>& includeDirs, bool& ok)
{
  ok = true;
  std::vector<std::string> visiting;
  auto result = expandIncludesInternal(contents, includeDirs, visiting, ok);

  return result;
}

std::string encodeAsPython(const std::string& name, const std::string& contents)
{
  return name + R"( = """)" + contents + R"(""")";
}

std::string encodeAsCppLiteral(const std::string& name, const std::string& contents)
{
  return "const char " + name + "[] = R\"v0g0nPoetry(" + contents + ")v0g0nPoetry\";\n";
}

std::string encodeAsCppFunction(const std::string& name, const std::string& contents)
{
  std::ostringstream encoded;
  encoded << R"(
#include <mutex>
#include <string>
#include <thread>

namespace
{

inline const std::string& )"
          << name << R"(()
{
  static std::mutex lock;
  static std::string data;
  std::lock_guard<std::mutex> guard(lock);
  if (data.empty())
  {
)";

  std::size_t dataLen = contents.size();
  // constexpr std::size_t pieceLen = 2048; // = 2^11
  std::size_t numPieces = dataLen / pieceLen;
  for (std::size_t ii = 0; ii < numPieces; ++ii)
  {
    encoded << "    data += R\"v0g0nPoetry(" << contents.substr(ii * pieceLen, pieceLen)
            << ")v0g0nPoetry\";\n";
  }
  // If there is a remainder smaller than the buffer size, encode it.
  if (dataLen % pieceLen)
  {
    encoded << "    data += R\"v0g0nPoetry("
            << contents.substr(numPieces * pieceLen) // , std::string::npos)
            << ")v0g0nPoetry\";\n";
  }

  encoded << R"(
  }
  return data;
}
} // anonymous namespace)";

  return encoded.str();
}

int main(int argc, char* argv[])
{
  using path = boost::filesystem::path;
  enum EncodeTo
  {
    Python,
    CppLiteral,
    CppFunction
  };
  bool processIncludes = false;
  std::vector<std::string> includeDirs;
  EncodeTo encodeTo = EncodeTo::CppLiteral;
  std::string inputFilename;
  std::string outputFilename;
  std::string ext;
  std::string encType;
  std::string nameToken;
  for (int ii = 1; ii < argc; ++ii)
  {
    std::string argii = argv[ii];
    if (argii == "TYPE")
    {
      if (ii + 1 >= argc)
      {
        return usage("The TYPE keyword requires a value after it.", 1);
      }
      encType = argv[ii + 1];
      ++ii;
      std::transform(encType.begin(), encType.end(), encType.begin(), ::tolower);
      if (encType.substr(0, 2) == "py")
      {
        encodeTo = EncodeTo::Python;
      }
      else if (encType == "xml" || encType == "c" || encType == "svg" || encType == "json")
      {
        encodeTo = EncodeTo::CppLiteral;
      }
      else if (encType == "c++" || encType == "cpp")
      {
        encodeTo = EncodeTo::CppFunction;
      }
      else
      {
        return usage("Unknown encoding type " + encType, 1);
      }
    }
    else if (argii == "EXT")
    {
      if (ii + 1 >= argc)
      {
        return usage("The EXT keyword requires a value after it.", 1);
      }
      ext = argv[ii + 1];
      ++ii;
    }
    else if (argii == "INCLUDE_DIRS")
    {
      for (int jj = ii + 1; jj < argc; ++jj)
      {
        std::string argjj = argv[jj];
        if (isKeyword(argjj))
        {
          break;
        }
        addPaths(includeDirs, argjj);
        ++ii;
      }
      if (includeDirs.empty())
      {
        return usage("The INCLUDE_DIRS keyword requires one or more values after it.", 1);
      }
      processIncludes = true;
    }
    else if (argii == "HEADER_OUTPUT")
    {
      if (ii + 1 >= argc)
      {
        return usage("The HEADER_OUTPUT keyword requires a value after it.", 1);
      }
      outputFilename = argv[ii + 1];
      ++ii;
    }
    else if (argii == "NAME")
    {
      if (ii + 1 >= argc)
      {
        return usage("The NAME keyword requires a value after it.", 1);
      }
      nameToken = sanitizeIdentifier(argv[ii + 1]);
      ++ii;
    }
    else if (argii == "PIECE_SIZE")
    {
      if (ii + 1 >= argc)
      {
        return usage("The NAME keyword requires a value after it.", 1);
      }
      pieceLen = atoi(argv[ii + 1]);
      ++ii;
    }
    else if (argii == "DEBUG")
    {
      debug = true;
    }
    else if (argii == "--help" || argii == "-h")
    {
      return usage("", 0);
    }
    else
    {
      if (inputFilename.empty())
      {
        inputFilename = argii;
      }
      else
      {
        return usage("Only one input file at a time is allowed.", 1);
      }
    }
  }
  if (inputFilename.empty())
  {
    return usage("You must provide an input filename.", 1);
  }
  if (ext.empty())
  {
    ext = (encodeTo == EncodeTo::Python ? "py" : "h");
  }
  if (outputFilename.empty())
  {
    path pathIn(inputFilename);
    outputFilename = pathIn.stem().string() + "_" + encType + "." + ext;
  }
  if (nameToken.empty())
  {
    // TODO: sanitize stem to a C variable name.
    path pathIn(inputFilename);
    nameToken = sanitizeIdentifier(pathIn.stem().string() + "_" + encType);
  }
  if (debug)
  {
    std::cout << "Encode \"" << inputFilename << "\" to \"" << outputFilename << "\""
              << " as " << nameToken
              << (encodeTo == EncodeTo::CppFunction
                    ? "()"
                    : (encodeTo == EncodeTo::CppLiteral ? "[]" : " python"))
              << "\n";
  }

  std::string contents;
  if (!readFile(contents, inputFilename))
  {
    return usage("ERROR: Unable to read file.", 3);
  }
  if (processIncludes)
  {
    bool ok;
    contents = expandIncludes(contents, includeDirs, ok);
    if (!ok)
    {
      return usage("ERROR: Include expansion failed.", 5);
    }
  }
  switch (encodeTo)
  {
    case EncodeTo::Python:
      contents = encodeAsPython(nameToken, contents);
      break;
    case EncodeTo::CppLiteral:
      contents = encodeAsCppLiteral(nameToken, contents);
      break;
    default: // fall through
    case EncodeTo::CppFunction:
      contents = encodeAsCppFunction(nameToken, contents);
      break;
  }
  if (!writeFile(contents, outputFilename))
  {
    return usage("ERROR: Unable to write file \"" + outputFilename + "\"", 9);
  }
  return 0;
}
