#pragma once

#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <spine/ConfigBase.h>
#include <string>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
/**
@verbatim
dataset:
{
  id = "1000567";
  uuid = "6667f5d8-041f-4b00-a55b-62980f6f78ef";
  ns = "FI";
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
  Dataset(const Dataset& other) = default;
  Dataset& operator=(const Dataset& other) = default;

  inline const Id& getId() const { return mId; }
  inline const boost::optional<Uuid>& getUuid() const { return mUuid; }
  inline const boost::optional<Ns>& getIdNs() const { return mIdNs; }

 private:
  Id mId;
  boost::optional<Ns> mIdNs;
  boost::optional<Uuid> mUuid;
};
}
}
}
