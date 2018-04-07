// ======================================================================
/*!
 * \brief Parameter configuration
 */
// ======================================================================

#pragma once

#include <newbase/NFmiLevel.h>
#include <newbase/NFmiParam.h>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

#include <map>
#include <string>
#include <vector>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
// Parameter configuration

struct ParamChangeItem
{
 public:
  ParamChangeItem();
  ParamChangeItem(const ParamChangeItem& theOther);
  ~ParamChangeItem();
  ParamChangeItem& operator=(const ParamChangeItem& theOther);

  unsigned long mOriginalParamId;
  NFmiParam mWantedParam;
  float mConversionBase;  // f(x) = (scale * x) + base
  float mConversionScale;
  NFmiLevel* mLevel;
  std::string mLevelType;  // Temporary storage for level type ..
  boost::optional<float>
      mLevelValue;  // .. and level value; used when creating NFmiLevel object (mLevel)
  boost::optional<std::string>
      mStepType;  // For average, cumulative etc. data; "accum", "max", "min", ...
  unsigned int mPeriodLengthMinutes;   // Period length in minutes for average, cumulative etc. data
  boost::optional<std::string> mUnit;  // Unit for netcdf parameters
  boost::optional<std::string> mStdName;   // Standfard name for netcdf parameters
  boost::optional<std::string> mLongName;  // Long name for netcdf parameters
  boost::optional<std::string> mCentre;    // Originating centre for grib parameters
  unsigned int mTemplateNumber;            // 'productDefinitionTemplateNumber' for grib parameters
};

using ParamChangeTable = std::vector<ParamChangeItem>;
ParamChangeTable readParamConfig(const boost::filesystem::path& configFilePath, bool grib = true);

class ParamConfig
{
 public:
  using Shared = std::shared_ptr<ParamConfig>;
  using FilePathType = std::string;
  using ParamIdType = unsigned long;
  using ParamMetaItemType = ParamChangeItem;
  using ParamMetaItemMapType = std::map<ParamIdType, std::shared_ptr<ParamMetaItemType> >;

  ParamConfig(const FilePathType& filePath);
  virtual ~ParamConfig();
  std::shared_ptr<ParamMetaItemType> getParamMetaItem(const ParamIdType& paramId) const;

 protected:
  void addParamMetaItem(const ParamMetaItemType& paramItem);
  const FilePathType& getFilePath() const { return mFilePath; }

 private:
  ParamConfig(const ParamConfig&) = delete;
  ParamConfig& operator=(const ParamConfig& other) = delete;

  ParamMetaItemMapType mParamMetaItemMap;
  FilePathType mFilePath;
};

class NetcdfParamConfig : public ParamConfig
{
 public:
  NetcdfParamConfig(const FilePathType& filePath);
  ~NetcdfParamConfig();

 private:
  NetcdfParamConfig(const NetcdfParamConfig& other) = delete;
  NetcdfParamConfig& operator=(const NetcdfParamConfig& other) = delete;
};
}  // namespace WCS
}  // namespace Plugin
}  // namespace SmartMet
