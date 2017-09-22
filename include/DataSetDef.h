#pragma once

#include "CompoundCRS.h"
#include "MultiLanguageString.h"
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <engines/gis/Engine.h>
#include <engines/querydata/Engine.h>
#include <spine/ConfigBase.h>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
/**
@verbatim
name = "hbm";
abstract: { eng="HBM model"; fin="HBM malli"};
keywords = ["forecast","ocean"];
parameters = ["TemperatureSea","Salinity"];
subtype = "RectifiedGridCoverage";

dataset:
{
  id = "1000567";
  uuid = "6667f5d8-041f-4b00-a55b-62980f6f78ef";
  ns = "FI";
};
compoundcrs:
{
//Some configurations from CompoundCRS class
};
@endverbatim
*/

class Dataset
{
 public:
  using Optional = boost::optional<Dataset>;
  using Id = std::string;
  using Ns = std::string;
  using Uuid = std::string;

  Dataset(boost::shared_ptr<SmartMet::Spine::ConfigBase> config, libconfig::Setting& setting);
  virtual ~Dataset() {}
  inline const Id& getId() const { return mId; }
  inline const boost::optional<Uuid>& getUuid() const { return mUuid; }
  inline const boost::optional<Ns>& getIdNs() const { return mIdNs; }

 private:
  Id mId;
  boost::optional<Ns> mIdNs;
  boost::optional<Uuid> mUuid;
};

class DataSetDef
{
 public:
  using Envelope = Engine::Querydata::WGS84Envelope;
  using Language = MultiLanguageString::Language;
  using Message = MultiLanguageString::Message;
  using Keyword = std::string;
  using Keywords = std::vector<Keyword>;

  DataSetDef(const std::string& defaultLanguage,
             boost::shared_ptr<SmartMet::Spine::ConfigBase> config,
             libconfig::Setting& setting);

  virtual ~DataSetDef();

  inline const bool& getDisabled() const { return mDisabled; }
  inline const std::string& getName() const { return mName; }
  boost::optional<Message> getTitle(const Language& language) const;
  boost::optional<Message> getAbstract(const Language& language) const;
  boost::optional<Keywords> getKeywords() const { return mKeywords; }
  inline const std::vector<std::string>& getParameters() const { return mParameters; }
  inline const std::string& getSubtype() const { return mSubtype; }
  const CompoundCRS& getCompoundcrs() const { return mCompoundcrs.get(); }
  const Dataset::Optional& getDataset() const { return mDataset; }
  const boost::optional<Envelope>& getWgs84BBox() const { return mWGS84BBox; }
  bool process(const Engine::Querydata::Engine& querydataEngine);

 private:
  DataSetDef& operator=(const DataSetDef& other);

  bool mDisabled;
  std::string mName;
  MultiLanguageString::Shared mTitle;
  MultiLanguageString::Shared mAbstract;
  boost::optional<Keywords> mKeywords;
  std::vector<std::string> mParameters;
  std::string mSubtype;
  CompoundCRS::Optional mCompoundcrs;
  Dataset::Optional mDataset;
  boost::optional<Envelope> mWGS84BBox;
};
}
}
}
