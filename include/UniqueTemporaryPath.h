#pragma once

#include <memory>
#include <boost/filesystem.hpp>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
/**
 *  \brief Unique temporary file path generator
 *
 *  The class creates a unique file path to temporary
 *  directory with 128 bits of randomness.
 *  e.g. /tmp/702047549334c327-8363ab3781287928
 *
 *  The class throws SmartMet::Spine::Exception
 *  exception when temporary path generation fails.
 */
class UniqueTemporaryPath
{
 public:
  using Shared = std::shared_ptr<UniqueTemporaryPath>;
  using Unique = std::unique_ptr<UniqueTemporaryPath>;
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
