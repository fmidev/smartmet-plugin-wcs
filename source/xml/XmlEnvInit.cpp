#include "xml/XmlEnvInit.h"
#include <sstream>
#include <stdexcept>
#include <xqilla/utils/UTF8Str.hpp>
#include <xqilla/utils/XQillaPlatformUtils.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <macgyver/TypeName.h>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
namespace Xml
{
EnvInit::EnvInit()
{
  try
  {
    XQillaPlatformUtils::initialize();
  }
  catch (const xercesc::XMLException& err)
  {
    std::ostringstream msg;
    msg << METHOD_NAME << ": Error during Xerces-C initialization: " << UTF8(err.getMessage());
    throw std::runtime_error(msg.str());
  }

  // Disable Xerces-C net accessor for now
  xercesc::XMLPlatformUtils::fgNetAccessor = NULL;
}

EnvInit::~EnvInit()
{
  XQillaPlatformUtils::terminate();
}
}
}
}
}
