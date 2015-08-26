#include <string>

#include "data.hpp"

// number of threads for label parsing and insertion
const size_t NUM_LABEL_THREADS = 2; // change to 4 for best performance with -O0
// number of threads for page link database insertion
const size_t ADD_LINK_THREADS = 2;
// number of threads for page link parsing
const size_t PARSE_LINK_THREADS = 4;

extern size_t nolabel;

/**
 * Read all labels from 'labelfile' (in .bz2 format) to the 'labels'
 * vector in wikidata and sort the data afterwards.
 */
void read_labels(WikiData &wikidata, string labelfile);

/**
 * Read all page links from 'linkfile' (in .bz2 format) to the 'links'
 * database. If incoming is set to 'true', backlinks will be inserted
 * as well.
 */
size_t read_page_links(WikiData &wikidata, const std::string& linkfile,
                       const bool incoming);
