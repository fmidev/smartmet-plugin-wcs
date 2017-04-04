#include "Plugin.h"
#include <spine/PluginTest.h>
#include <iostream>

using namespace std;

void prelude(SmartMet::Spine::Reactor& reactor)
{
  try
  {
    // Wait for engines to finish
    auto handlers = reactor.getURIMap();
    while (handlers.find("/wcs") == handlers.end())
    {
      sleep(1);
      handlers = reactor.getURIMap();
    }
    cout << endl << "Testing WCS plugin" << endl << "============================" << endl;
  }
  catch (...)
  {
    SmartMet::Spine::Exception exception(BCP, "Operation failed!", NULL);
    std::cerr << exception.what();
    //    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}

int main(int argc, char* argv[])
{
  try
  {
    SmartMet::Spine::Options options;
    options.configfile = argc == 1 ? "cnf/smartmet_test.conf" : argv[1];
    options.quiet = false;
    options.accesslogdir = "/tmp";

    try
    {
      return SmartMet::Spine::PluginTest::test(options, prelude);
    }
    catch (libconfig::ParseException& err)
    {
      std::cerr << "Exception '" << Fmi::current_exception_type()
                << "' thrown while parsing configuration file " << options.configfile
                << "' at line " << err.getLine() << ": " << err.getError();
      return 1;
    }
  }
  catch (...)
  {
    SmartMet::Spine::Exception exception(BCP, "Operation failed!", NULL);
    std::cerr << exception.what();
  }
}
