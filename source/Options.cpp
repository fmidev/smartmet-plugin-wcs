#include "Options.h"
#include "WcsConst.h"
#include "WcsConvenience.h"
#include "WcsException.h"
#include "xml/XmlUtils.h"
#include <boost/algorithm/string.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/spirit/include/classic_ref_value_actor.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/qi.hpp>
#include <cmath>
#include <typeindex>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
void Options::setDimensionSubset(const DimensionSubset& dimensionSubset)
{
  const DimensionSubset* subset = reinterpret_cast<const DimensionSubset*>(&dimensionSubset);

  const std::string dimensionName = subset->get("Dimension").to_string();
  if (getDimensionSlice(dimensionName))
  {
    std::ostringstream msg;
    msg << "Only one subset allowed for '" << dimensionName << "' dimension.";
    throw WcsException(WcsException::INVALID_SUBSETTING, msg.str());
  }

  try
  {
    if (const DimensionSlice* slice = dynamic_cast<const DimensionSlice*>(subset))
    {
      auto val = slice->get("Dimension");
      if (not val.is_empty())
        mDimensionSlices.emplace(val.to_string(), std::move(*slice));
    }
    else if (const DimensionTrim* trim = dynamic_cast<const DimensionTrim*>(subset))
    {
      auto val = trim->get("Dimension");
      if (not val.is_empty())
        mDimensionTrims.emplace(val.to_string(), std::move(*trim));
    }
    else
    {
      throw std::runtime_error(
          "Unsupported Dimension class detected in Wcs::Options::setDimensionSubset");
    }
  }
  catch (const std::bad_cast& e)
  {
    throw std::runtime_error("Cast from DimensionSubset to target failed.");
  }
  catch (const std::exception& e)
  {
    throw std::runtime_error(e.what());
  }
}

boost::optional<DimensionSlice> Options::getDimensionSlice(const NameType& name) const
{
  auto it = mDimensionSlices.find(name);
  if (it != mDimensionSlices.end())
    return boost::optional<DimensionSlice>(it->second);
  else
    return boost::optional<DimensionSlice>();
}

boost::optional<DimensionTrim> Options::getDimensionTrim(const NameType& name) const
{
  auto it = mDimensionTrims.find(name);
  if (it != mDimensionTrims.end())
    return boost::optional<DimensionTrim>(it->second);
  else
    return boost::optional<DimensionTrim>();
}
}
}
}
