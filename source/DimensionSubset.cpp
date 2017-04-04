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

Element::Element(const xercesc::DOMElement* element)
{
  std::pair<std::string, std::string> nameInfo = Xml::get_name_info(element);
  auto value = boost::algorithm::trim_copy(Xml::extract_text(*element));
  if (not value.empty())
  {
    mValue = Value(value);
    mName = nameInfo.first;
  }
  else
  {
    std::ostringstream msg;
    msg << "Empty value detected in '" << nameInfo.first << "'.";
    throw WcsException(WcsException::INVALID_SUBSETTING, msg.str());
  }
}

Element::Element(const Value& value, const NameType& name) : mValue(value), mName(name)
{
  if (mValue == Value())
  {
    std::ostringstream msg;
    msg << "Empty value detected in '" << mName << "'.";
    throw WcsException(WcsException::INVALID_SUBSETTING, msg.str());
  }
}

DimensionSubset::DimensionSubset(const xercesc::DOMElement* element,
                                 const std::set<NameType>& allowedChildrens)
{
  auto childrens = Xml::get_child_elements(*element, WCS_NAMESPACE_URI, "*");

  for (const xercesc::DOMElement* child : childrens)
  {
    const auto name =
        Xml::check_name_info(child, WCS_NAMESPACE_URI, allowedChildrens, "Wcs::DimensionSubset");

    setElement(std::move(Element(child)));
  }
}

DimensionSubset::DimensionSubset(const std::string& ss)
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

DimensionSlice::DimensionSlice(const xercesc::DOMElement* element)
    : DimensionSubset(element, {"Dimension", "SlicePoint"})
{
}

DimensionSlice::DimensionSlice(const std::string& ss) : DimensionSubset(ss)
{
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
  char name[2] = "";
  double valueDouble = std::numeric_limits<float>::max();
  bool isValid = phrase_parse(
      ss.begin(),
      ss.end(),
      (char_[ref(*name) = _1] >> char_('(') >>
       ((lexeme[char_('"') >> +((char_ - '"')[push_back(boost::phoenix::ref(valueStr), _1)]) >>
                char_('"')]) |
        (double_[boost::phoenix::ref(valueDouble) = _1])) >>
       char_(')')),
      spirit::ascii::space);

  if (not isValid)
    goto invalidSubset;

  if (std::isfinite(valueDouble) and std::numeric_limits<float>::lowest() < valueDouble and
      valueDouble < std::numeric_limits<float>::max())
  {
    if (name[0] == 't')
    {
      boost::posix_time::ptime epoch = ptimeFromUnixTime(static_cast<int64_t>(valueDouble));
      if (epoch.is_not_a_date_time())
      {
        goto invalidSubset;
      }
      setElement(Element(Value(name), "Dimension"));
      setElement(Element(Value(epoch), "SlicePoint"));
    }
    else if (name[0] == 'x' or name[0] == 'y' or name[0] == 'z')
    {
      setElement(Element(Value(name), "Dimension"));
      setElement(Element(Value(valueDouble), "SlicePoint"));
    }
    else
    {
      goto invalidSubset;
    }
  }
  else if (not valueStr.empty())
  {
    boost::posix_time::ptime epoch = Spine::parse_xml_time(valueStr);
    if (epoch.is_not_a_date_time())
    {
      std::ostringstream msg;
      msg << "Invalid subset '" << ss << "'.";
      throw WcsException(WcsException::INVALID_SUBSETTING, msg.str());
    }
    setElement(Element(Value(name), "Dimension"));
    setElement(Element(Value(epoch), "SlicePoint"));
  }
  else
  {
    goto invalidSubset;
  }

  return;

invalidSubset:
  std::ostringstream msg;
  msg << "Invalid DimensionSlice subset '" << ss << "'.";
  throw WcsException(WcsException::INVALID_SUBSETTING, msg.str());
}

DimensionTrim::DimensionTrim(const xercesc::DOMElement* element)
    : DimensionSubset(element, {"Dimension", "TrimLow", "TrimHigh"})
{
}

DimensionTrim::DimensionTrim(const std::string& ss) : DimensionSubset(ss)
{
  using boost::spirit::qi::phrase_parse;
  using boost::spirit::qi::_1;
  using boost::spirit::qi::string;
  using boost::spirit::qi::char_;
  using boost::spirit::qi::double_;
  using boost::spirit::qi::int_;
  using boost::spirit::qi::lexeme;
  using boost::phoenix::ref;
  using phoenix::push_back;

  char name[2] = "";
  double firstValueDouble = std::numeric_limits<float>::max();
  double secondValueDouble = std::numeric_limits<float>::max();
  std::string firstValueStr;
  std::string secondValueStr;
  bool isValid = phrase_parse(
      ss.begin(),
      ss.end(),
      (char_[ref(*name) = _1] >> char_('(') >>
       ((lexeme[char_('"') >> +((char_ - '"')[push_back(boost::phoenix::ref(firstValueStr), _1)]) >>
                char_('"') >> char_(',') >> char_('"') >>
                +((char_ - '"')[push_back(boost::phoenix::ref(secondValueStr), _1)]) >>
                char_('"')]) |
        (double_[boost::phoenix::ref(firstValueDouble) = _1]) >> char_(',') >>
            (double_[boost::phoenix::ref(secondValueDouble) = _1])) >>
       char_(')')),
      spirit::ascii::space);

  if (not isValid)
    goto invalidSubset;

  if (std::isfinite(firstValueDouble) and std::isfinite(secondValueDouble) and
      std::numeric_limits<float>::lowest() < firstValueDouble and
      firstValueDouble < std::numeric_limits<float>::max() and
      std::numeric_limits<float>::lowest() < secondValueDouble and
      secondValueDouble < std::numeric_limits<float>::max())
  {
    if (name[0] == 'x' or name[0] == 'y' or name[0] == 'z')
    {
      setElement(Element(Value(name), "Dimension"));
      setElement(Element(Value(firstValueDouble), "TrimLow"));
      setElement(Element(Value(secondValueDouble), "TrimHigh"));
    }
    else if (name[0] == 't')
    {
      boost::posix_time::ptime epoch1 = ptimeFromUnixTime(static_cast<int64_t>(firstValueDouble));
      boost::posix_time::ptime epoch2 = ptimeFromUnixTime(static_cast<int64_t>(secondValueDouble));
      if (epoch1.is_not_a_date_time() or epoch2.is_not_a_date_time() or (epoch2 < epoch1))
      {
        goto invalidSubset;
      }
      setElement(Element(Value(name), "Dimension"));
      setElement(Element(Value(epoch1), "TrimLow"));
      setElement(Element(Value(epoch2), "TrimHigh"));
    }
    else
    {
      goto invalidSubset;
    }
  }
  else if (not firstValueStr.empty() and not secondValueStr.empty() and name[0] == 't')
  {
    boost::posix_time::ptime epoch1 = Spine::parse_xml_time(firstValueStr);
    boost::posix_time::ptime epoch2 = Spine::parse_xml_time(secondValueStr);
    if (epoch1.is_not_a_date_time() or epoch2.is_not_a_date_time() or (epoch2 < epoch1))
    {
      goto invalidSubset;
    }
    setElement(Element(Value(name), "Dimension"));
    setElement(Element(Value(epoch1), "TrimLow"));
    setElement(Element(Value(epoch2), "TrimHigh"));
  }
  else
  {
    goto invalidSubset;
  }

  return;

invalidSubset:
  /*
  std::ostringstream msg;
  msg << "Invalid DimensionTrim subset '" << ss << "'.";
  throw WcsException(WcsException::INVALID_SUBSETTING, msg.str() );
  */
  return;
}
}
}
}
