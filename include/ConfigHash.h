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
  using Language = std::string;
  using Shared = std::shared_ptr<ConfigHash>;
  using Unique = std::unique_ptr<ConfigHash>;

  explicit ConfigHash();
  virtual ~ConfigHash();

  const CTPP::CDT& getHash() const;
  void set(const libconfig::Config& config);
  void set(const libconfig::Setting& setting);
  void setLanguage(const Language& language);

  bool exists(const std::string& key);

 private:
  ConfigHash(const ConfigHash& other) = delete;
  const ConfigHash& operator=(const ConfigHash& other) = delete;

  void store(const libconfig::Setting& setting, CTPP::CDT& hash);

  CTPP::CDT mHash;
  Language mLanguage;
};
}
}
}
