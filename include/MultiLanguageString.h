#pragma once

#include <spine/ConfigBase.h>
#include <map>
#include <memory>
#include <string>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
/**
@verbatim
abstract: { eng = "Multilanguage string"; fin = "Monikielinen teksti"; }
@endverbatim
 */
class MultiLanguageString
{
 public:
  using Shared = std::shared_ptr<MultiLanguageString>;
  using Language = std::string;
  using Message = std::string;
  using Content = std::map<Language, Message>;

  MultiLanguageString(const Language& defaultLanguage, libconfig::Setting& setting);
  virtual ~MultiLanguageString();
  static Shared create(const Language& defaultLanguage, libconfig::Setting& setting);
  const Message& get() const;
  const Message& get(const Language& language) const;
  inline Language getDefaultLanguage() const { return mDefaultLanguage; }
  inline const Content& getContent() const { return mData; }

 private:
  const Language mDefaultLanguage;
  Content mData;
};
}
}
}
