#pragma once

#include <boost/filesystem.hpp>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
/**
 *  \brief An unique temporary file path generator
 *
 *  The class creates an unique file path to temporary
 *  directory with 128 bits of randomness.
 *  e.g. /tmp/7020-4754-9334-c328-363a-b378-1287-9283
 *
 *  The class thows SmartMet::Spine::Exception
 *  exception when temporary path generation fails.
 */
class UniqueTemporaryPath
{
 public:
  explicit UniqueTemporaryPath();
  virtual ~UniqueTemporaryPath();

  std::string get() const;
  const char* c_str() const;

 private:
  boost::filesystem::path mTempDirectoryPath;
  boost::filesystem::path mUniquePath;
};
}
}
}
