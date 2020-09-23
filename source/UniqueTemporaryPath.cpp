#include "UniqueTemporaryPath.h"
#include <macgyver/Exception.h>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
UniqueTemporaryPath::UniqueTemporaryPath(const AutoRemove& autoRemove) : mAutoRemove(autoRemove)
{
  try
  {
    mTempDirectoryPath = boost::filesystem::temp_directory_path();

    // Each replacement '%' hexadecimal digit adds four bits of randomness.
    mUniquePath = boost::filesystem::unique_path("%%%%%%%%%%%%%%%%-%%%%%%%%%%%%%%%%");
  }
  catch (const boost::filesystem::filesystem_error& err)
  {
    throw Fmi::Exception::Trace(BCP, err.what());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Construction of UniqueTemporaryPath object failed.");
  }
}

UniqueTemporaryPath::~UniqueTemporaryPath()
{
  if (mAutoRemove)
    std::remove(get().c_str());
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
    throw Fmi::Exception::Trace(BCP, err.what());
  }
  catch (...)
  {
    throw Fmi::Exception::Trace(BCP, "Return of UniqueTemporaryPath as a string failed.");
  }
}
}  // namespace WCS
}  // namespace Plugin
}  // namespace SmartMet
