#include "Abbreviations.h"
#include "WcsException.h"

#include <algorithm>
#include <iterator>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
Abbreviations::Abbreviations()
{
}

Abbreviations::Abbreviations(const Vector& valueVector)
{
  for (const auto& value : valueVector)
    insert(value);
}

Abbreviations::~Abbreviations()
{
}

Abbreviations::Abbreviation Abbreviations::getAbbreviation(const Distance& distance) const
{
  int maxDistance = mAbbreviations.size();
  if (maxDistance > distance)
    return mAbbreviations.at(distance);
  return Abbreviation();
}

const Abbreviations::Vector& Abbreviations::getAbbreviationVector() const
{
  return mAbbreviations;
}

Abbreviations::Distance Abbreviations::getDistance(const Abbreviation& abbreviation) const
{
  auto it = std::find(std::begin(mAbbreviations), std::end(mAbbreviations), abbreviation);
  if (it == std::end(mAbbreviations))
  {
    std::ostringstream msg;
    msg << "Dimension abbreviation '" << abbreviation << "' not found. Values available are:";
    for (const auto& item : mAbbreviations)
      msg << " '" << item << "'";
    throw WcsException(WcsException::INVALID_SUBSETTING, msg.str());
  }
  return std::distance(std::begin(mAbbreviations), it);
}

void Abbreviations::insert(const Abbreviation& value)
{
  Vector splitVector;
  boost::algorithm::split(splitVector, value, boost::algorithm::is_any_of(" "));
  if (splitVector.size() != 1)
  {
    std::ostringstream msg;
    msg << "Empty or multiple-part dimension abbreviation '" << value << "' detected.";
    throw WcsException(WcsException::INVALID_AXIS_LABEL, msg.str());
  }

  auto it = std::find(std::begin(mAbbreviations), std::end(mAbbreviations), value);
  if (it != std::end(mAbbreviations))
  {
    std::ostringstream msg;
    msg << "Duplicate dimension abbreviation '" << value << "' detected.";
    throw WcsException(WcsException::INVALID_AXIS_LABEL, msg.str());
  }
  mAbbreviations.push_back(value);
}
}
}
}
