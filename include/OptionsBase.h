#pragma once

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
class OptionsBase
{
 public:
  OptionsBase() {}
  virtual ~OptionsBase() {}

 protected:
 private:
  OptionsBase(const OptionsBase& other) = delete;
  OptionsBase& operator=(const OptionsBase& other) = delete;
};
}
}
}
