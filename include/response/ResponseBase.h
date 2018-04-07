#pragma once

#include "OptionsBase.h"
#include <spine/SmartMetEngine.h>
#include <memory>
#include <ostream>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
class ResponseBase
{
 public:
  ResponseBase() {}
  virtual ~ResponseBase() {}
  virtual void setEngine(const SmartMet::Spine::SmartMetEngine* engine) = 0;
  virtual void setOptions(const std::shared_ptr<OptionsBase> options) { mOptionsBase = options; }
  virtual void get(std::ostream& output) = 0;

 protected:
  virtual std::shared_ptr<OptionsBase> getOptions() const { return mOptionsBase; }

 private:
  std::shared_ptr<OptionsBase> mOptionsBase;
};
}  // namespace WCS
}  // namespace Plugin
}  // namespace SmartMet
