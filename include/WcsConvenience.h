#pragma once

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <gdal/ogr_spatialref.h>
#include <newbase/NFmiParameterName.h>
#include <newbase/NFmiPoint.h>
#include <spine/HTTP.h>
#include <spine/Parameter.h>
#include <spine/Reactor.h>
#include <spine/SmartMet.h>
#include <spine/Value.h>
#include <iostream>
#include <libconfig.h++>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

// FIXME: update compiler versions for which we need __attribute__((__may_alias__)) as needed.
#if defined(__GNUC__)
#if (__GNUC__ == 4 && __GNUC_MINOR__ == 4)
#define MAY_ALIAS __attribute__((__may_alias__))
#endif
#endif /* defined(__GNUC__) */

#ifndef MAY_ALIAS
#define MAY_ALIAS
#endif

namespace SmartMet
{
namespace Plugin
{
inline bool special(const Spine::Parameter& theParam)
{
  switch (theParam.type())
  {
    case Spine::Parameter::Type::Data:
    case Spine::Parameter::Type::Landscaped:
      return false;
    case Spine::Parameter::Type::DataDerived:
    case Spine::Parameter::Type::DataIndependent:
      return true;
  }
  // ** NOT REACHED **
  return true;
}

/**
 *   @brief Add Spine::Parameter to the vector and return index of freshly appended vector item
 */
inline std::size_t addParam(std::vector<Spine::Parameter>& dest,
                            const std::string& name,
                            Spine::Parameter::Type type,
                            FmiParameterName number = kFmiBadParameter)
{
  Spine::Parameter param(name, type, number);
  dest.push_back(param);
  return dest.size() - 1;
}

namespace WCS
{
/**
 *   @brief Escape XML string
 */
std::string xmlEscape(const std::string& src);

/**
 *   @brief Provides range check when assigning to shorter integer type
 *
 *   Throws std::runtime error when out of range
 *
 *   Usually only first template parameter (destination type)
 *   must be specified.
 */
template <typename DestIntType, typename SourceIntType>
DestIntType castIntType(const SourceIntType value)
{
  if ((value < std::numeric_limits<DestIntType>::min() ||
       (value > std::numeric_limits<DestIntType>::max())))
  {
    std::ostringstream msg;
    msg << "Value " << value << " is out of range " << std::numeric_limits<DestIntType>::min()
        << "..." << std::numeric_limits<DestIntType>::max();
    throw std::runtime_error(msg.str());
  }
  else
  {
    return value;
  }
}

using SmartMet::Spine::string2bool;
using SmartMet::Spine::string2ptime;

std::string getMandatoryHeader(const SmartMet::Spine::HTTP::Request& request,
                               const std::string& name);

template <typename ParamType>
ParamType getParam(const SmartMet::Spine::HTTP::Request& request,
                   const std::string& name,
                   ParamType defaultValue)
{
  auto value = request.getParameter(name);
  if (value)
    return boost::lexical_cast<ParamType>(*value);
  else
    return defaultValue;
}

void checkTimeInterval(const boost::posix_time::ptime& start,
                       const boost::posix_time::ptime& end,
                       double maxHours);

void assertUnreachable(const char* file, int line) __attribute__((noreturn));

std::string asString(const std::vector<Spine::Value>& src);

std::string asString(const boost::variant<Spine::Value, std::vector<Spine::Value> >& src);

std::string removeTrailing0(const std::string& src);

NFmiPoint transform(const std::unique_ptr<OGRCoordinateTransformation>& transformation,
                    const NFmiPoint& p);

std::int64_t unixTimeFromPtime(const boost::posix_time::ptime& epoch);

boost::posix_time::ptime ptimeFromUnixTime(const std::int64_t& unixTime);

NFmiPoint swap(const NFmiPoint& p, const bool& swap);

const std::string rotLatLonWKT =
    "PROJCS[\"Plate_Carree\","
    "GEOGCS[\"unnamed ellipse\", DATUM[\"D_unknown\", SPHEROID[\"Unknown\",6371000,0]], "
    "PRIMEM[\"Greenwich\",0],"
    "UNIT[\"Degree\",0.017453292519943295]],"
    "PROJECTION[\"Plate_Carree\"],"
    "PARAMETER[\"latitude_of_origin\",0],"
    "PARAMETER[\"central_meridian\",-40],"
    "PARAMETER[\"false_easting\",639367],"
    "PARAMETER[\"false_northing\",1473324],"
    "EXTENSION[\"PROJ4\",\"+proj=ob_tran +o_proj=eqc +lon_0=0 +o_lat_p=30 +a=57.29578 +wktext\"],"
    "UNIT[\"Degree\",0.017453292519943295]]";
}  // namespace WCS
}  // namespace Plugin
}  // namespace SmartMet

#define ASSERT_UNREACHABLE assertUnreachable(__FILE__, __LINE__)
