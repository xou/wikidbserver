#include <iostream>
#include <string>
#include <boost/program_options.hpp>
#include "data.hpp"
#include "commandline_interface.hpp"
#include "read.hpp"

using namespace std;
namespace po = boost::program_options;


int main(int argc, char ** argv) {
  po::options_description desc("Options");
  desc.add_options()
    ("help", "this help message")
    ("labels", po::value<string>(), "labels file (required)")
    ("links", po::value<string>(), "page link file")
    ("inlinks", "add incoming links");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help") || !vm.count("labels")) {
    cout << desc << endl;
    return 1;
  }

  string labelsfile = vm["labels"].as<string>();
  string linkfile = "";
  if (vm.count("links")) {
    linkfile = vm["links"].as<string>();
  }

  bool incoming = false;
  if (vm.count("inlinks"))
    incoming = true;

  WikiData data;
  auto clock_start = chrono::system_clock::now();
  read_labels(data, labelsfile);
  auto clock_labels_done = chrono::system_clock::now();
  cout << "Loading " << data.labels.size() << " labels took " << 
    chrono::duration_cast<chrono::seconds>(clock_labels_done-clock_start).count()
    << " seconds. " << endl;
  if (linkfile.size()) {
    data.links.resize(data.labels.size());
    size_t n_pagelinks = read_page_links(data, linkfile, incoming);
    auto clock_pagelinks_done = chrono::system_clock::now();
    cout << "Loading " << n_pagelinks << " page links took " <<
      chrono::duration_cast<chrono::seconds>(clock_pagelinks_done - clock_labels_done).count()
      << " seconds. " << endl;
  }

  // duplicate output to make it easier to find.
  cout << "Loading " << data.labels.size() << " labels took " << 
    chrono::duration_cast<chrono::seconds>(clock_labels_done-clock_start).count()
    << " seconds. " << endl;
  cout << "Label compression removed " << nolabel << " labels." << endl;
  

  CLI cli(data);
  cli.run();
  return 0;
}

// vim: foldmethod=marker
