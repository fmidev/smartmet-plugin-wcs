#pragma once
#include <boost/noncopyable.hpp>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
namespace Xml
{
/**
 *   @brief Wraps Xerces-C library initialization and deinitialization
 */
class EnvInit : virtual private boost::noncopyable
{
 public:
  EnvInit();

  virtual ~EnvInit();
};
}
}
}
}
