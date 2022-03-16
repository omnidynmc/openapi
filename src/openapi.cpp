#include <cassert>
#include <exception>
#include <iostream>
#include <new>
#include <string>

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "App.h"
#include "App_Log.h"

openapi::App *app;

void usage() {
  fprintf(stderr, "Usage: %s -d [-v] [-c <config_file_path>] [-b vhost]\n", app->prompt().c_str());
  exit(1);
} // usage

int main(int argc, char **argv) {
  bool debug = false;
  char *config = NULL;
  char *prompt = NULL;
  char ch;

  umask(077);

  // remember prompt
  if ((prompt = strrchr(argv[0], '/')))
    prompt++;
  else
    prompt = argv[0];

  // parse command line option
  while ((ch = getopt(argc, argv, "vdc:")) != -1) {
    switch (ch) {
      case 'c':
        config = optarg;
        break;
      case 'd':
        debug = true;
        break;
      case 'v':
//        (void) version();
        exit(0);
        break;
      case '?':
      default:
        (void) usage();
        break;
    } // switch
  } // while

  // parse mysql record id for configuration
  argc -= optind;
  argv += optind;

  try {
    app = new openapi::App(prompt, (config) ? config : "etc/openapi.conf");
    app->logger(new openapi::App_Log(""));
    app->set_debug(debug);
  } // try
  catch(std::bad_alloc xa) {
    assert(false);
  } // catch

  app->start();
  app->run();
  app->stop();

  delete app;

} // main
