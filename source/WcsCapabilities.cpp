#include "WcsCapabilities.h"
#include <macgyver/TypeName.h>
#include <spine/Convenience.h>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
using SmartMet::Spine::log_time_str;

WcsCapabilities::WcsCapabilities() : mVersions({"2.0.1", "2.0.0"}), mHighestVersion("2.0.1")
{
  if (mVersions.empty() or mHighestVersion.empty())
  {
    std::ostringstream msg;
    msg << METHOD_NAME << ": Version(s) not defined.";
    throw std::runtime_error(msg.str());
  }
  if (mVersions.find(mHighestVersion) == mVersions.end())
  {
    std::ostringstream msg;
    msg << METHOD_NAME << ": Highest WCS version of service not found from the supported versions.";
    throw std::runtime_error(msg.str());
  }
}
WcsCapabilities::~WcsCapabilities()
{
}

bool WcsCapabilities::registerOperation(const std::string& operation)
{
  Spine::WriteLock lock(mutex);
  return mOperations.insert(operation).second;
}

bool WcsCapabilities::registerOutputFormat(const RequestType& rtype, const std::string& format)
{
  Spine::WriteLock lock(mutex);
  const char* pattern =
      "(application|audio|image|text|video|message|multipart|model)/.+(;\\s*.+=.+)*";
  static const boost::regex e(pattern);
  if (not regex_match(format, e))
  {
    std::ostringstream msg;
    msg << METHOD_NAME << ": unsupported MIME type set to registration: code='" << format << "'. "
        << "The value should match with the pattern \"" << pattern << "\".";
    throw std::runtime_error(msg.str());
  }

  if (0 > rtype or rtype >= RequestType::UNDEFINED)
  {
    std::ostringstream msg;
    msg << METHOD_NAME << ": unsupported request type '" << rtype << "' used with output format '"
        << format << "' requestration.";
    throw std::runtime_error(msg.str());
  }

  FormatSetMap::iterator it = mFormats.find(rtype);
  if (it == mFormats.end())
    mFormats.insert(std::make_pair(rtype, FormatSet()));

  it = mFormats.find(rtype);
  return it->second.insert(format).second;
}

const WcsCapabilities::FormatSet WcsCapabilities::getSupportedFormats(
    const RequestType& rtype) const
{
  Spine::ReadLock lock(mutex);
  FormatSetMap::const_iterator it = mFormats.find(rtype);
  if (it != mFormats.end())
    return it->second;
  else
  {
    FormatSet fset;
    for (auto& item : mFormats)
      fset.insert(item.second.begin(), item.second.end());
    return fset;
  }
}

const WcsCapabilities::OperationSet& WcsCapabilities::getSupportedOperations() const
{
  Spine::ReadLock lock(mutex);
  return mOperations;
}

const WcsCapabilities::VersionSet& WcsCapabilities::getSupportedVersions() const
{
  Spine::ReadLock lock(mutex);
  return mVersions;
}

const WcsCapabilities::Version& WcsCapabilities::getHighestVersion() const
{
  Spine::ReadLock lock(mutex);
  return mHighestVersion;
}

void WcsCapabilities::registerDataset(const WcsCapabilities::Dataset& code,
                                      const CoverageDescription& cov)
{
  Spine::WriteLock lock(mutex);
  if (not mDatasetMap.insert(std::make_pair(code, cov)).second)
  {
    std::ostringstream msg;
    msg << METHOD_NAME << ": duplicate data set registration: code='" << code << "'";
    throw std::runtime_error(msg.str());
  }
}

const WcsCapabilities::DatasetMap& WcsCapabilities::getSupportedDatasets() const
{
  Spine::ReadLock lock(mutex);
  return mDatasetMap;
}
}
}
}
