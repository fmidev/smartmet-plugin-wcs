#include "ConfigHash.h"

#include "WcsException.h"

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
ConfigHash::ConfigHash() : mHash(CTPP::CDT())
{
}

ConfigHash::~ConfigHash()
{
}

const CTPP::CDT& ConfigHash::getHash() const
{
  return mHash;
}

bool ConfigHash::exists(const std::string& key)
{
  try
  {
    if (mHash.Exists(key))
      return true;

    return false;
  }
  catch (const CTPP::CTPPException& err)
  {
    throw Spine::Exception(BCP, err.what(), NULL);
  }
  catch (...)
  {
    throw Spine::Exception(BCP, "Unknown error while trying to find a key from hash", NULL);
  }
}

void ConfigHash::set(const libconfig::Config& config)
{
  try
  {
    // Clear the old data and store the new one.
    libconfig::Setting& root = config.getRoot();
    mHash = CTPP::CDT();
    store(root, mHash);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Config set to ConfigHash failed!", NULL);
  }
}

void ConfigHash::set(const libconfig::Setting& setting)
{
  try
  {
    mHash = CTPP::CDT();
    store(setting, mHash);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Setting set to ConfigHash failed!", NULL);
  }
}

void ConfigHash::store(const libconfig::Setting& setting, CTPP::CDT& targetHash)
{
  try
  {
    const char* name = setting.getName();
    int type = setting.getType();

    if (type == libconfig::Setting::TypeGroup)
    {
      int N = setting.getLength();
      for (int i = 0; i < N; i++)
      {
        if (name)
          store(setting[i], targetHash[name]);
        else
          store(setting[i], targetHash[i]);
      }
    }
    else if (type == libconfig::Setting::TypeList)
    {
      int N = setting.getLength();
      for (int i = 0; i < N; i++)
      {
        if (name)
          store(setting[i], targetHash[name][i]);
        else
          store(setting[i], targetHash[i]);
      }
    }
    else if (type == libconfig::Setting::TypeArray)
    {
      int N = setting.getLength();
      for (int i = 0; i < N; i++)
      {
        if (name)
          store(setting[i], targetHash[name][i]);
        else
          store(setting[i], targetHash[i]);
      }
    }
    else if (type == libconfig::Setting::TypeInt)
    {
      int value = setting;
      if (name)
        targetHash[name] = value;
      else
        targetHash = value;
    }
    else if (type == libconfig::Setting::TypeInt64)
    {
      int64_t value = setting;
      if (name)
        targetHash[name] = value;
      else
        targetHash = value;
    }
    else if (type == libconfig::Setting::TypeFloat)
    {
      double value = setting;
      char buffer[80];
      snprintf(buffer, sizeof(buffer), "%.16g", value);
      if (name)
        targetHash[name] = buffer;
      else
        targetHash = buffer;
    }
    else if (type == libconfig::Setting::TypeString)
    {
      const std::string value = setting;
      if (name)
        targetHash[name] = value;
      else
        targetHash = value;
    }
    else if (type == libconfig::Setting::TypeBoolean)
    {
      const bool value = setting;
      if (name)
        targetHash[name] = (value ? "true" : "false");
      else
        targetHash = (value ? "true" : "false");
    }
    else
    {
      // Error: unknown type
      targetHash[0] = '?';
    }
  }
  catch (...)
  {
    std::ostringstream msg;
    msg << "ConfigHash  failed. "
        << "Setting type: '" << setting.getType() << "' ";
    if (const char* name = setting.getName())
      msg << " name: '" << name << "'.";

    throw SmartMet::Spine::Exception(BCP, msg.str(), NULL);
  }
}
}
}
}