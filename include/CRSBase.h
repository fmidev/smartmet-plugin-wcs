#pragma once

#include <spine/ConfigBase.h>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
class CRSBase
{
 public:
  using Identifier = std::string;
  using Abbrev = std::string;
  using DimensionSize = size_t;

  CRSBase();
  virtual ~CRSBase() {}
  inline const Abbrev& getAbbrev() const { return mAbbrev; }
  inline const DimensionSize& getDimensionSize() const { return mDimensionSize; }
  inline const Identifier& getIdentifier() const { return mIdentifier; }

 protected:
  void parse(boost::shared_ptr<SmartMet::Spine::ConfigBase> config, libconfig::Setting& setting);
  void setAbbrev(const Abbrev& abbrev);
  void setIdentifier(const Identifier& identifier);

 private:
  Identifier mIdentifier;
  Abbrev mAbbrev;
  DimensionSize mDimensionSize;
};
}  // namespace WCS
}  // namespace Plugin
}  // namespace SmartMet
