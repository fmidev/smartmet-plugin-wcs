#pragma once

#include "Coverage.h"
#include "RequestType.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <spine/Thread.h>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
class WcsCapabilities
{
 public:
  using Dataset = std::string;
  using DatasetMap = std::map<Dataset, CoverageDescription>;
  using Format = std::string;
  using FormatSet = std::set<Format>;
  using FormatSetMap = std::map<RequestType, FormatSet>;
  using Operation = std::string;
  using OperationSet = std::set<Operation>;
  using Version = std::string;
  using VersionSet = std::set<Version>;

  WcsCapabilities();

  virtual ~WcsCapabilities();

  const DatasetMap& getSupportedDatasets() const;

  // If request type is UNDEFINED a compined list of formats will be retuned.
  const FormatSet getSupportedFormats(const RequestType& rtype = RequestType::UNDEFINED) const;

  const OperationSet& getSupportedOperations() const;

  const VersionSet& getSupportedVersions() const;

  const Version& getHighestVersion() const;

  bool registerOperation(const Operation& operation);

  bool registerOutputFormat(const RequestType& rtype, const Format& format);

  void registerDataset(const Dataset& code, const CoverageDescription& cov);

 private:
  FormatSetMap mFormats;
  OperationSet mOperations;
  VersionSet mVersions;
  Version mHighestVersion;
  DatasetMap mDatasetMap;

  /**
   *  @brief Mutex for preventing race conditions when accessing this object
   */
  mutable Spine::MutexType mutex;
};
}
}
}
