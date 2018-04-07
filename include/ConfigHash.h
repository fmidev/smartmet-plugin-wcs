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
/** \brief Create CTPP::CDT hash from a libconfig configuration.
 */
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

  /** \brief Get the processed result.
   *  \return Processed configuration or setting as a hash container.
   */
  const CTPP::CDT& getHash() const;

  /** \brief Set a default language which must be given in every MultiLanguageString group.
   *  Text mapped to the default language will be used if text to a requested language
   *  does not exist (setLanguage method). If the default is not set, all the
   *  MultiLanguageString groups will be ignored as a context of MultiLanguageString class
   *  and the content will be processed as a normal setting.
   *  \param[in] language A default language which must be given in MultiLanguageString group.
   */
  void setDefaultLanguage(const Language& language);

  /** \brief Set a language which will be used in result.
   *  The language will be used instead of the default when MultiLanguageString groups
   *  are processed. If a text for the language is not found, the default language will be used.
   *  \param[in] language The language to favour against the default.
   */
  void setLanguage(const Language& language);

  /** \brief Process a full configuration
   *  \param config A full congfiguration.
   */
  void process(const libconfig::Config& config);

  /** \brief Process a setting which may be a part of the full configuration.
   *  \param setting A setting in configuration
   */
  void process(const libconfig::Setting& setting);

  /** \brief Test if a configuration exist in the object.
   *  Use dot notation for a path e.g. "Capability.ServiceIdentification.Title".
   *  \param path A path from the root of a config or a setting
   *  \retval true If every node of the path exists.
   *  \retval false If some node of the path does not exist.
   */
  bool exists(const Path& path);

 private:
  const ConfigHash& operator=(const ConfigHash& other) = delete;

  void store(const libconfig::Setting& setting, CTPP::CDT& hash);

  CTPP::CDT mHash;
  Language mDefaultLanguage;
  Language mLanguage;
};
}  // namespace WCS
}  // namespace Plugin
}  // namespace SmartMet
