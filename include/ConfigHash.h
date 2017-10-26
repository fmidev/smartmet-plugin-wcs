#pragma once

#include "Config.h"

#include <ctpp2/CDT.hpp>
#include <spine/ConfigBase.h>
#include <memory>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
class ConfigHash
{
 public:
  using Path = std::string;
  using Language = std::string;
  using Shared = std::shared_ptr<ConfigHash>;
  using Unique = std::unique_ptr<ConfigHash>;

  explicit ConfigHash();
  explicit ConfigHash(const ConfigHash& other);
  virtual ~ConfigHash();

  const CTPP::CDT& getHash() const;

  // Set language to search from MultiLanguageString setting.
  void setDefaultLanguage(const Language& language);
  void setLanguage(const Language& language);

  void process(const libconfig::Config& config);
  void process(const libconfig::Setting& setting);

  bool exists(const Path& path);

 private:
  const ConfigHash& operator=(const ConfigHash& other) = delete;

  void store(const libconfig::Setting& setting, CTPP::CDT& hash);

  CTPP::CDT mHash;
  Language mDefaultLanguage;
  Language mLanguage;
};
}
}
}
