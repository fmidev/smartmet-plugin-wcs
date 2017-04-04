#include "WcsConvenience.h"
#include "WcsException.h"
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/spirit/include/qi.hpp>
#include <macgyver/TimeParser.h>
#include <cctype>
#include <sstream>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
std::string xmlEscape(const std::string& src)
{
  std::ostringstream output;

  for (char c : src)
  {
    switch (c)
    {
      case '<':
        output << "&lt;";
        break;
      case '>':
        output << "&gt;";
        break;
      case '&':
        output << "&amp;";
        break;
      case '\'':
        output << "&apos;";
        break;
      case '"':
        output << "&quot;";
        break;
      default:
        output << c;
        break;
    }
  }

  return output.str();
}

std::string getMandatoryHeader(const SmartMet::Spine::HTTP::Request& request,
                               const std::string& name)
{
  auto value = request.getHeader(name);
  if (not value)
  {
    std::ostringstream msg;
    msg << "Mandatory HTTP request header " << name << " missing";
    throw WcsException(WcsException::OPERATION_PARSING_FAILED, msg.str());
  }

  return *value;
}

void checkTimeInterval(const boost::posix_time::ptime& start,
                       const boost::posix_time::ptime& end,
                       double maxHours)
{
  namespace pt = boost::posix_time;

  if (start > end)
  {
    std::ostringstream msg;
    msg << "Invalid time interval: starttime is greater than endtime"
        << " (found starttime='" << pt::to_simple_string(start) << "' endtime='"
        << pt::to_simple_string(end) << "'";
    throw WcsException(WcsException::OPERATION_PARSING_FAILED, msg.str());
  }

  pt::time_duration intervalLength = end - start;
  int miHours = static_cast<int>(std::floor(maxHours));
  int miSec = static_cast<int>(std::floor(3600.0 * (maxHours - miHours)));
  if (intervalLength > pt::hours(miHours) + pt::seconds(miSec))
  {
    std::ostringstream msg;
    msg << "Too long time interval '" << pt::to_simple_string(start) << "' to '"
        << pt::to_simple_string(end) << "' specified"
        << " (no more than " << maxHours << " hours allowed)";
    throw WcsException(WcsException::OPERATION_PROCESSING_FAILED, msg.str());
  }
}

void assertUnreachable(const char* file, int line)
{
  std::ostringstream msg;
  msg << "INTERNAL ERROR in " << file << " at line " << line << ": not supposed to get here";
  throw std::runtime_error(msg.str());
}

std::string asString(const std::vector<Spine::Value>& src)
{
  std::ostringstream ost;
  ost << "(ValueArray ";
  for (const auto& item : src)
  {
    ost << item;
  }
  ost << ")";
  return ost.str();
}

std::string asString(const boost::variant<Spine::Value, std::vector<Spine::Value> >& src)
{
  std::ostringstream ost;
  if (src.which() == 0)
  {
    ost << boost::get<Spine::Value>(src);
  }
  else
  {
    ost << asString(boost::get<std::vector<Spine::Value> >(src));
  }
  return ost.str();
}

std::string removeTrailing0(const std::string& src)
{
  std::string tmp = boost::algorithm::trim_copy(src);
  if ((tmp.find_first_not_of("+-.0123456789") == std::string::npos) and
      (tmp.find('.') != std::string::npos))
  {
    const char* c1 = tmp.c_str();
    const char* c2 = c1 + tmp.length();
    while (*(c2 - 1) == '0' and std::isdigit(*(c2 - 2)))
      c2--;
    return std::string(c1, c2);
  }
  else
  {
    return src;
  }
}

NFmiPoint transform(const std::unique_ptr<OGRCoordinateTransformation>& transformation,
                    const NFmiPoint& p)
{
  double x = p.X();
  double y = p.Y();
  if (not transformation->Transform(1, &x, &y))
  {
    std::ostringstream msg;
    msg << "Transformation failed from '" << transformation->GetSourceCS()->GetEPSGGeogCS()
        << "' to '" << transformation->GetTargetCS()->GetEPSGGeogCS() << "'.";
    WcsException err(WcsException::NO_APPLICABLE_CODE, msg.str());
    throw err;
  }

  return NFmiPoint(x, y);
}

std::int64_t unixTimeFromPtime(const boost::posix_time::ptime& epoch)
{
  static const long refJd = boost::gregorian::date(1970, 1, 1).julian_day();
  long long jd = epoch.date().julian_day();
  long seconds = epoch.time_of_day().total_seconds();
  INT_64 secEpoch = 86400LL * (jd - refJd) + seconds;
  return secEpoch;
}

boost::posix_time::ptime ptimeFromUnixTime(const std::int64_t& unixTime)
{
  boost::gregorian::date d(1970, 1, 1);
  return (boost::posix_time::ptime(d) + boost::posix_time::seconds(unixTime));
}

NFmiPoint swap(const NFmiPoint& p, const bool& swap)
{
  return (swap ? NFmiPoint(p.Y(), p.X()) : p);
}
}
}
}
