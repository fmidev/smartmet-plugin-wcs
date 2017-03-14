// ======================================================================
/*!
 * \brief Parameter configuration loading
 */
// ======================================================================

#include "ParamConfig.h"

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <macgyver/StringConversion.h>

#include <fstream>

#include <json/json.h>
#include <json/reader.h>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
ParamConfig::ParamConfig(const FilePathType& filePath) : mFilePath(filePath)
{
  if (filePath.empty())
  {
    throw std::runtime_error("Empty ParamConfig file path detected.");
  }
}

ParamConfig::~ParamConfig()
{
}

void ParamConfig::addParamMetaItem(const ParamMetaItemType& paramItem)
{
  long ident = paramItem.mWantedParam.GetIdent();

  auto it = mParamMetaItemMap.find(ident);
  if (it != mParamMetaItemMap.end())
  {
    std::ostringstream msg;
    msg << "Duplicate parameter item id '" << ident << "' detected";
    throw std::runtime_error(msg.str());
  }
  mParamMetaItemMap.emplace(ident, std::make_shared<ParamMetaItemType>(paramItem));
}

std::shared_ptr<ParamConfig::ParamMetaItemType> ParamConfig::getParamMetaItem(
    const ParamIdType& paramId) const
{
  auto it = mParamMetaItemMap.find(paramId);
  if (it != mParamMetaItemMap.end())
    return it->second;
  else
    return std::shared_ptr<ParamMetaItemType>();
}

NetcdfParamConfig::NetcdfParamConfig(const FilePathType& filePath) : ParamConfig(filePath)
{
  ParamChangeTable paramTable = readParamConfig(filePath, false);
  for (const auto& item : paramTable)
    addParamMetaItem(item);
}

NetcdfParamConfig::~NetcdfParamConfig()
{
}

// ======================================================================
/*!
 * \brief Parameter configuration item
 */
// ======================================================================

ParamChangeItem::ParamChangeItem()
    : mOriginalParamId(0),
      mWantedParam(
          0, "", kFloatMissing, kFloatMissing, kFloatMissing, kFloatMissing, "%.1f", kLinearly),
      mConversionBase(0),
      mConversionScale(1.0),
      mLevel(NULL),
      mPeriodLengthMinutes(0),
      mTemplateNumber(0)
{
}

ParamChangeItem::~ParamChangeItem()
{
  if (mLevel)
    delete mLevel;
}

ParamChangeItem::ParamChangeItem(const ParamChangeItem& theOther)
    : mOriginalParamId(theOther.mOriginalParamId),
      mWantedParam(theOther.mWantedParam),
      mConversionBase(theOther.mConversionBase),
      mConversionScale(theOther.mConversionScale),
      mLevel(theOther.mLevel ? new NFmiLevel(*theOther.mLevel) : NULL),
      mStepType(theOther.mStepType),
      mPeriodLengthMinutes(theOther.mPeriodLengthMinutes),
      mUnit(theOther.mUnit),
      mStdName(theOther.mStdName),
      mLongName(theOther.mLongName),
      mCentre(theOther.mCentre),
      mTemplateNumber(theOther.mTemplateNumber)
{
}

ParamChangeItem& ParamChangeItem::operator=(const ParamChangeItem& theOther)
{
  if (this != &theOther)
  {
    mOriginalParamId = theOther.mOriginalParamId;
    mWantedParam = theOther.mWantedParam;
    mConversionBase = theOther.mConversionBase;
    mConversionScale = theOther.mConversionScale;
    mLevel = theOther.mLevel ? new NFmiLevel(*theOther.mLevel) : NULL;
    mStepType = theOther.mStepType;
    mPeriodLengthMinutes = theOther.mPeriodLengthMinutes;
    mUnit = theOther.mUnit;
    mStdName = theOther.mStdName;
    mLongName = theOther.mLongName;
    mCentre = theOther.mCentre;
    mTemplateNumber = theOther.mTemplateNumber;
  }

  return *this;
}

// ======================================================================
/*!
 * \brief Functions to convert a json value to a number or a string.
 */
// ======================================================================
unsigned long asUInt64(const std::string& name, const Json::Value& json, uint arrayIndex)
{
  if (json.isUInt64())
    return json.asUInt64();

  throw std::runtime_error("'" + name + "': uint64 value expected at array index " +
                           Fmi::to_string(arrayIndex) + ", got value " + json.asString() +
                           " instead");
}

unsigned int asUInt(const std::string& name, const Json::Value& json, uint arrayIndex)
{
  if (json.isUInt())
    return json.asUInt();

  throw std::runtime_error("'" + name + "': uint value expected at array index " +
                           Fmi::to_string(arrayIndex));
}

float asFloat(const std::string& name, const Json::Value& json, uint arrayIndex)
{
  if (json.isDouble())
    return json.asFloat();

  throw std::runtime_error("'" + name + "': float value expected at array index " +
                           Fmi::to_string(arrayIndex));
}

std::string asString(const std::string& name, const Json::Value& json, uint arrayIndex)
{
  // json.isConvertibleTo(Json::stringValue) for a number returns true but json.asString() fails
  //
  // if (json.isConvertibleTo(Json::stringValue))

  if (json.isString())
    return json.asString();

  throw std::runtime_error("'" + name + "': string value expected at array index " +
                           Fmi::to_string(arrayIndex));
}

// ======================================================================
/*!
 * \brief Load grib format specific configuration fields.
 */
// ======================================================================

bool readGribParamConfigField(const std::string& name,
                              const Json::Value& json,
                              ParamChangeItem& p,
                              unsigned int arrayIndex)
{
  if (name == "gribid")
  {
    p.mOriginalParamId = asUInt64(name, json, arrayIndex);
  }
  else if (name == "leveltype")
  {
    p.mLevelType = asString(name, json, arrayIndex);
  }
  else if (name == "levelvalue")
  {
    p.mLevelValue = asFloat(name, json, arrayIndex);
  }
  else if (name == "center")
  {
    p.mCentre = asString(name, json, arrayIndex);
  }
  else if (name == "templatenumber")
  {
    p.mTemplateNumber = asUInt(name, json, arrayIndex);
  }
  else
  {
    // Unknown setting
    //
    return false;
  }

  return true;
}

// ======================================================================
/*!
 * \brief Load netcdf format specific configuration fields.
 */
// ======================================================================

bool readNetCdfParamConfigField(const std::string& name,
                                const Json::Value& json,
                                ParamChangeItem& p,
                                unsigned int arrayIndex)
{
  if (name == "standardname")
    p.mStdName = asString(name, json, arrayIndex);
  else if (name == "longname")
    p.mLongName = asString(name, json, arrayIndex);
  else if (name == "unit")
    p.mUnit = asString(name, json, arrayIndex);
  else
    // Unknown setting
    //
    return false;

  return true;
}

// ======================================================================
/*!
 * \brief Load parameter configuration.
 */
// ======================================================================

ParamChangeTable readParamConfig(const boost::filesystem::path& configFilePath, bool grib)
{
  // Read and parse the JSON formatted configuration

  Json::Reader reader;
  Json::Value theJson;
  std::vector<ParamChangeItem> paramChangeTable;

  std::ifstream in(configFilePath.c_str());
  if (!in)
    throw std::runtime_error("Failed to open '" + configFilePath.string() + "' for reading");

  std::string content;
  content.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());

  if (!reader.parse(content, theJson))
    throw std::runtime_error("Failed to parse '" + configFilePath.string() + "': " +
                             reader.getFormattedErrorMessages());

  if (!theJson.isArray())
    throw std::runtime_error("Parameter configuration must contain an array of JSON objects");

  // Iterate through all the objects

  auto& fmtConfigFunc(grib ? readGribParamConfigField : readNetCdfParamConfigField);

  for (unsigned int i = 0; i < theJson.size(); i++)
  {
    const Json::Value& paramJson = theJson[i];

    if (!paramJson.isObject())
      throw std::runtime_error("JSON object expected at array index " + Fmi::to_string(i));

    ParamChangeItem p;
    std::string paramName;
    uint paramId = 0;

    const auto members = paramJson.getMemberNames();
    BOOST_FOREACH (const auto& name, members)
    {
      const Json::Value& json = paramJson[name];
      if (json.isArray() || json.isObject())
        throw std::runtime_error(name + ": value is neither a string nor a number at array index " +
                                 Fmi::to_string(i));

      // Ignore null values

      if (json.isNull())
        continue;
      //
      // Handle common settings
      //
      if (name == "newbaseid")
        paramId = asUInt(name, json, i);
      else if (name == "name")
        paramName = asString(name, json, i);
      else if (name == "offset")
        p.mConversionBase = asFloat(name, json, i);
      else if (name == "divisor")
        p.mConversionScale = asFloat(name, json, i);
      else if (name == "aggregatetype")
        p.mStepType = asString(name, json, i);
      else if (name == "aggregatelength")
        p.mPeriodLengthMinutes = asUInt(name, json, i);
      //
      // Handle format specific settings
      //
      else if (!fmtConfigFunc(name, json, p, i))
        throw std::runtime_error(std::string(grib ? "Grib" : "Netcdf") +
                                 " parameter configuration does not have a setting named '" + name +
                                 "'");
    }

    // Set parameter id and name

    p.mWantedParam.SetIdent(paramId);
    p.mWantedParam.SetName(paramName);

    // Create level object if level data was given

    if (p.mLevelValue || (!p.mLevelType.empty()))
      p.mLevel = new NFmiLevel(0, p.mLevelType, p.mLevelValue ? *(p.mLevelValue) : 0);

    paramChangeTable.push_back(p);
  }

  return paramChangeTable;
}
}
}
}
