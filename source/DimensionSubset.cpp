#include "DimensionSubset.h"
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
namespace spirit = boost::spirit;
namespace phoenix = boost::phoenix;

DimensionSubset::DimensionSubset(const ChildrenNameSet& allowedChildrens)
    : mAllowedChildrens(allowedChildrens)
{
}

DimensionSubset::~DimensionSubset()
{
}

void DimensionSubset::setAbbreviations(const Abbreviations::Shared& abbreviations)
{
  mAbbreviations = abbreviations;
}

const Abbreviations::Shared& DimensionSubset::getAbbreviations() const
{
  return mAbbreviations;
}

void DimensionSubset::setSubset(const xercesc::DOMElement* element)
{
  if (not mAbbreviations)
  {
    std::ostringstream msg;
    msg << "Axis labels not found. DimensionSubset operation is not possible.";
    throw WcsException(WcsException::INVALID_SUBSETTING, msg.str());
  }

  auto childrens = Xml::get_child_elements(*element, WCS_NAMESPACE_URI, "*");

  for (const xercesc::DOMElement* child : childrens)
  {
    const auto name =
        Xml::check_name_info(child, WCS_NAMESPACE_URI, mAllowedChildrens, "Wcs::DimensionSubset");

    Element childElement(child);

    // Validate axis label
    if (name == "Dimension")
      mAbbreviations->getDistance(childElement.getValue().to_string());

    setElement(std::move(childElement));
  }
}

void DimensionSubset::setSubset(const std::string&)
{
}

Element::Value DimensionSubset::get(const std::string& name) const
{
  auto it = mElements.find(name);
  if (it != mElements.end())
    return it->second.getValue();

  return Value();
}

void DimensionSubset::setElement(const Element&& element)
{
  mElements.emplace(element.getName(), std::move(element));
}

DimensionSlice::DimensionSlice() : DimensionSubset({"Dimension", "SlicePoint"})
{
}

DimensionSlice::~DimensionSlice()
{
}

void DimensionSlice::setSubset(const xercesc::DOMElement* element)
{
  DimensionSubset::setSubset(element);
}

void DimensionSlice::setSubset(const std::string& ss)
{
  std::ostringstream msg;
  if (not getAbbreviations())
  {
    msg << "Axis labels not found. DimensionSlice operation is not possible.";
    throw WcsException(WcsException::INVALID_SUBSETTING, msg.str());
  }
  msg << "Invalid DimensionSlice subset '" << ss << "'.";

  using boost::spirit::qi::phrase_parse;
  using boost::spirit::qi::_1;
  using boost::spirit::qi::string;
  using boost::spirit::qi::char_;
  using boost::spirit::qi::double_;
  using boost::spirit::qi::int_;
  using boost::spirit::qi::lexeme;
  using boost::phoenix::ref;
  using phoenix::push_back;

  std::string valueStr;
  std::string name;
  double valueDouble = std::numeric_limits<float>::max();
  bool isValid = phrase_parse(
      ss.begin(),
      ss.end(),
      (lexeme[+(char_[push_back(boost::phoenix::ref(name), _1)] - char_('(')) >> char_('(')] >>
       ((lexeme[char_('"') >> +((char_ - '"')[push_back(boost::phoenix::ref(valueStr), _1)]) >>
                char_('"')]) |
        (double_[boost::phoenix::ref(valueDouble) = _1])) >>
       char_(')')),
      spirit::ascii::space);

  if (not isValid)
    return;

  Abbreviations::Distance distance = getAbbreviations()->getDistance(name);

  if (std::isfinite(valueDouble) and std::numeric_limits<float>::lowest() < valueDouble and
      valueDouble < std::numeric_limits<float>::max())
  {
    if (distance == 3)
    {
      boost::posix_time::ptime epoch = ptimeFromUnixTime(static_cast<int64_t>(valueDouble));
      if (epoch.is_not_a_date_time())
      {
        msg << " Epoch is not a date time.";
        throw WcsException(WcsException::INVALID_SUBSETTING, msg.str());
      }
      setElement(Element(Value(name), "Dimension"));
      setElement(Element(Value(epoch), "SlicePoint"));
    }
    else if (distance == 0 or distance == 1 or distance == 2)
    {
      setElement(Element(Value(name), "Dimension"));
      setElement(Element(Value(valueDouble), "SlicePoint"));
    }
    else
    {
      msg << " Not a valid dimension name.";
      throw WcsException(WcsException::INVALID_SUBSETTING, msg.str());
    }
  }
  else if (not valueStr.empty() and distance == 3)
  {
    boost::posix_time::ptime epoch = Spine::parse_xml_time(valueStr);
    if (epoch.is_not_a_date_time())
    {
      msg << " Epoch is not a date time.";
      throw WcsException(WcsException::INVALID_SUBSETTING, msg.str());
    }
    setElement(Element(Value(name), "Dimension"));
    setElement(Element(Value(epoch), "SlicePoint"));
  }
  else
  {
    throw WcsException(WcsException::INVALID_SUBSETTING, msg.str());
  }
}

DimensionTrim::DimensionTrim() : DimensionSubset({"Dimension", "TrimLow", "TrimHigh"})
{
}

DimensionTrim::~DimensionTrim()
{
}

void DimensionTrim::setSubset(const xercesc::DOMElement* element)
{
  DimensionSubset::setSubset(element);
}

void DimensionTrim::setSubset(const std::string& ss)
{
  std::ostringstream msg;

  if (not getAbbreviations())
  {
    msg << "Axis labels not found. DimensionTrim operation is not possible.";
    throw WcsException(WcsException::INVALID_SUBSETTING, msg.str());
  }
  msg << "Invalid DimensionTrim subset '" << ss << "'.";

  using boost::spirit::qi::phrase_parse;
  using boost::spirit::qi::_1;
  using boost::spirit::qi::string;
  using boost::spirit::qi::char_;
  using boost::spirit::qi::double_;
  using boost::spirit::qi::int_;
  using boost::spirit::qi::lexeme;
  using boost::phoenix::ref;
  using phoenix::push_back;

  std::string name;
  double firstValueDouble = std::numeric_limits<float>::max();
  double secondValueDouble = std::numeric_limits<float>::max();
  std::string firstValueStr;
  std::string secondValueStr;
  bool isValid = phrase_parse(
      ss.begin(),
      ss.end(),
      (lexeme[+(char_[push_back(boost::phoenix::ref(name), _1)] - char_('(')) >> char_('(')] >>
       ((lexeme[char_('"') >> +((char_ - '"')[push_back(boost::phoenix::ref(firstValueStr), _1)]) >>
                char_('"') >> char_(',') >> char_('"') >>
                +((char_ - '"')[push_back(boost::phoenix::ref(secondValueStr), _1)]) >>
                char_('"')]) |
        (double_[boost::phoenix::ref(firstValueDouble) = _1]) >> char_(',') >>
            (double_[boost::phoenix::ref(secondValueDouble) = _1])) >>
       char_(')')),
      spirit::ascii::space);

  if (not isValid)
    return;

  Abbreviations::Distance distance = getAbbreviations()->getDistance(name);

  if (std::isfinite(firstValueDouble) and std::isfinite(secondValueDouble) and
      std::numeric_limits<float>::lowest() < firstValueDouble and
      firstValueDouble < std::numeric_limits<float>::max() and
      std::numeric_limits<float>::lowest() < secondValueDouble and
      secondValueDouble < std::numeric_limits<float>::max())
  {
    if (distance == 0 or distance == 1 or distance == 2)
    {
      setElement(Element(Value(name), "Dimension"));
      setElement(Element(Value(firstValueDouble), "TrimLow"));
      setElement(Element(Value(secondValueDouble), "TrimHigh"));
    }
    else if (distance == 3)
    {
      boost::posix_time::ptime epoch1 = ptimeFromUnixTime(static_cast<int64_t>(firstValueDouble));
      boost::posix_time::ptime epoch2 = ptimeFromUnixTime(static_cast<int64_t>(secondValueDouble));
      if (epoch1.is_not_a_date_time() or epoch2.is_not_a_date_time() or (epoch2 < epoch1))
      {
        msg << " Epoch is not a date time or epoch2 < epoch1.";
        throw WcsException(WcsException::INVALID_SUBSETTING, msg.str());
      }
      setElement(Element(Value(name), "Dimension"));
      setElement(Element(Value(epoch1), "TrimLow"));
      setElement(Element(Value(epoch2), "TrimHigh"));
    }
    else
    {
      msg << " Not a valid dimension name.";
      throw WcsException(WcsException::INVALID_SUBSETTING, msg.str());
    }
  }
  else if (not firstValueStr.empty() and not secondValueStr.empty() and distance == 3)
  {
    boost::posix_time::ptime epoch1 = Spine::parse_xml_time(firstValueStr);
    boost::posix_time::ptime epoch2 = Spine::parse_xml_time(secondValueStr);
    if (epoch1.is_not_a_date_time() or epoch2.is_not_a_date_time() or (epoch2 < epoch1))
    {
      msg << " Epoch is not a date time or epoch2 < epoch1.";
      throw WcsException(WcsException::INVALID_SUBSETTING, msg.str());
    }
    setElement(Element(Value(name), "Dimension"));
    setElement(Element(Value(epoch1), "TrimLow"));
    setElement(Element(Value(epoch2), "TrimHigh"));
  }
  else
  {
    throw WcsException(WcsException::INVALID_SUBSETTING, msg.str());
  }
}
}
}
}
