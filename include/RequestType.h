#pragma once

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
enum RequestType
{
  GET_CAPABILITIES,
  DESCRIBE_COVERAGE,
  GET_COVERAGE,
  UNDEFINED  // UNDEFINED indicates the end of enum list.
};
}
}
}
