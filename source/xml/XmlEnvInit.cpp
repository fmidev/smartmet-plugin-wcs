#include "xml/XmlEnvInit.h"
#include <macgyver/TypeName.h>
#include <xercesc/util/PlatformUtils.hpp>
#include <xqilla/utils/UTF8Str.hpp>
#include <xqilla/utils/XQillaPlatformUtils.hpp>
#include <sstream>
#include <stdexcept>

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
