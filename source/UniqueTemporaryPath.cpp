#include "UniqueTemporaryPath.h"
#include <spine/Exception.h>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
UniqueTemporaryPath::UniqueTemporaryPath()
{
  try
  {
    mTempDirectoryPath = boost::filesystem::temp_directory_path();

    // Each replacement '%' hexadecimal digit adds four bits of randomness.
    mUniquePath = boost::filesystem::unique_path("%%%%-%%%%-%%%%-%%%%-%%%%-%%%%-%%%%-%%%%");
  }
  catch (const boost::filesystem::filesystem_error& err)
  {
    throw Spine::Exception(BCP, err.what(), NULL);
  }
  catch (...)
  {
    throw Spine::Exception(BCP, "Operation failed!", NULL);
  }
}

UniqueTemporaryPath::~UniqueTemporaryPath()
{
}

std::string UniqueTemporaryPath::get() const
{
  try
  {
    std::string name;
    name.append(mTempDirectoryPath.native());
    name.append("/");
    name.append(mUniquePath.native());
    return name;
  }
  catch (const boost::filesystem::filesystem_error& err)
  {
    throw Spine::Exception(BCP, err.what(), NULL);
  }
  catch (...)
  {
    throw Spine::Exception(BCP, "Operation failed!", NULL);
  }
}

const char* UniqueTemporaryPath::c_str() const
{
  try
  {
    return get().c_str();
  }
  catch (...)
  {
    throw Spine::Exception(BCP, "Operation failed!", NULL);
  }
}
}
}
}
