#include "MultiLanguageString.h"
#include <boost/algorithm/string.hpp>
#include <macgyver/StringConversion.h>
#include <macgyver/TypeName.h>
#include <spine/Exception.h>
#include <sstream>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
MultiLanguageString::MultiLanguageString(const Language& defaultLanguage,
                                         const libconfig::Setting& setting)
    : mDefaultLanguage(Fmi::ascii_tolower_copy(defaultLanguage)), mData()
{
  try
  {
    bool found_default = false;

    if (setting.getType() != libconfig::Setting::TypeGroup)
    {
      std::ostringstream msg;
      msg << "Libconfig group expected instead of '";
      SmartMet::Spine::ConfigBase::dump_setting(msg, setting);
      msg << "'";
      throw SmartMet::Spine::Exception(BCP, msg.str());
    }

    const int num_items = setting.getLength();
    for (int i = 0; i < num_items; i++)
    {
      libconfig::Setting& item = setting[i];
      if (item.getType() != libconfig::Setting::TypeString)
      {
        std::ostringstream msg;
        msg << "Libconfig string expected instead of '";
        SmartMet::Spine::ConfigBase::dump_setting(msg, item);
        msg << "' while reading item '";
        SmartMet::Spine::ConfigBase::dump_setting(msg, setting);
        msg << "'";
        throw SmartMet::Spine::Exception(BCP, msg.str());
      }

      const Message value = item;
      const Language tmp = item.getName();
      const Language name = Fmi::ascii_tolower_copy(tmp);
      if (name == this->mDefaultLanguage)
        found_default = true;
      mData[name] = value;
    }

    if (not found_default)
    {
      std::ostringstream msg;
      msg << "The string for the default language '" << this->mDefaultLanguage
          << "' is not found in '";
      SmartMet::Spine::ConfigBase::dump_setting(msg, setting);
      throw SmartMet::Spine::Exception(BCP, msg.str());
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception::Trace(BCP, "Operation failed!");
  }
}

MultiLanguageString::Shared MultiLanguageString::create(const Language& defaultLanguage,
                                                        libconfig::Setting& setting)
{
  try
  {
    Shared result(new MultiLanguageString(defaultLanguage, setting));
    return result;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception::Trace(BCP, "Operation failed!");
  }
}

MultiLanguageString::~MultiLanguageString() {}

const MultiLanguageString::Message& MultiLanguageString::get() const
{
  return get(mDefaultLanguage);
}

const MultiLanguageString::Message& MultiLanguageString::get(const Language& language) const
{
  try
  {
    auto pos = mData.find(Fmi::ascii_tolower_copy(language));
    if (pos == mData.end())
    {
      return get(mDefaultLanguage);
    }
    else
    {
      return pos->second;
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception::Trace(BCP, "Operation failed!");
  }
}
}  // namespace WCS
}  // namespace Plugin
}  // namespace SmartMet
