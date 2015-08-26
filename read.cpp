#include "read.hpp"

#include <thread>
#include <vector>
#include <tuple>
#include <algorithm>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include "producer_consumer_queue.hpp"
#include "bzreader.hpp"
#include "escaped_list_ignore.hpp"
#include "parseutil.hpp"


using namespace std;
using namespace boost::algorithm;
using namespace boost;

// stats: number of occurances where we didn't need to store the label seperately.
size_t nolabel = 0;

// Label parsing /*{{{*/
// add a line from the labels resource file to the database.
void add_label(WikiData& wikidata, const string& line, const size_t linenr) {
  if (!line.size() || line[0] == '#')
    return;
  escaped_list_separator_includeinvalid<char> ls('\\', ' ', '\"');
  tokenizer<escaped_list_separator_includeinvalid<char> > tok(line, ls);

  auto it = tok.begin();
  size_t i = 0;
  string resource, label;
  for (i = 0; i < 3 && it != tok.end(); ++i, ++it) {
    if (i == 0) {
      resource = *it;      
      abbr_ressource(resource);
    }
    if (i == 2) {
      label = *it;
      parse_nt_string(label);      
    }
  }
  if (i != 3) {
    cerr << "line [" << linenr << "] malformed: " << line << endl;
    return;
  }

  string denormalized = resource;
  wikipedia_denormalization(denormalized);
  if (denormalized != label) {
    resource.push_back('\0');
    resource.append(label);
  } else {
    nolabel++;
  }

  {
    unique_lock<mutex> lock(wikidata.labels_write);
    wikidata.labels.push_back(resource);
  }
}

size_t label_linecount = 1;
void add_label_thread(WikiData &wikidata, ProducerConsumerQueue<string> &q) {
  string line;
  while (q.pop(line)) {
    add_label(wikidata, line, label_linecount);
    label_linecount += 1;
    if (label_linecount % 1000000 == 0) {
      cout << "Read " << label_linecount << " labels. Queue is at " << q.size() << endl;
    }
  }
}

void read_labels(WikiData &wikidata, string labelfile = "labels_en.nt.bz2") {
  cout << "Reading labels from " << labelfile << endl;

  ProducerConsumerQueue<string> q(4096);
  vector<thread> threads;
  for (size_t i = 0; i < NUM_LABEL_THREADS; ++i) {
    threads.push_back(thread(add_label_thread, std::ref(wikidata), std::ref(q)));
  }
  BzReader r(labelfile);
  try {
    while (!r.done()) {
      q.push(r.readline());
    }
  } catch (const std::runtime_error &e) {
    cerr << e.what() << endl;
  }
  q.terminate_consumers();
  for (thread& t: threads) {
    t.join();
  }

  cout << "Reading finished, read " << label_linecount << " labels. Sorting." << endl;

  sort(wikidata.labels.begin(), wikidata.labels.end());
}

/*}}}*/
// Link parsing /*{{{*/
/**
 * Handles a configurable amount of add_link_locked calls in parallel.
 */
class LinkWriteDispatcher {
  typedef ProducerConsumerQueue<tuple<WikiData::ArticleID, WikiData::ArticleID, bool>> pcqueue_t;
  vector<thread> threads;
  vector<pcqueue_t*> prodcons;
  WikiData& wikidata;
  size_t n_threads;

  void add_link_thread(pcqueue_t *q) {
    tuple<WikiData::ArticleID, WikiData::ArticleID, bool> data;
    while (q->pop(data)) {
      wikidata.add_link_unsafe(get<0>(data),
          get<1>(data), get<2>(data));
    }
  }

public:
  const LinkWriteDispatcher& operator=(const LinkWriteDispatcher&other) = delete;
  LinkWriteDispatcher(const LinkWriteDispatcher&other) = delete;

  LinkWriteDispatcher(WikiData& wikidata, size_t n_threads) : 
      wikidata(wikidata), n_threads(n_threads) {
    for (size_t i = 0; i < n_threads; ++i) {
      prodcons.push_back(new pcqueue_t);
      threads.push_back(thread(&LinkWriteDispatcher::add_link_thread, this, prodcons[i]));
    }
  }

  ~LinkWriteDispatcher() {
    for (pcqueue_t* q: prodcons) {
      q->terminate_consumers(); 
    }
    for (thread& t: threads) {
      t.join();
    }
    for (pcqueue_t* q: prodcons) {
      delete q;
    }
  }

  void add_link(WikiData::ArticleID from, WikiData::ArticleID target,
        bool outgoing) {
    size_t thread_id = from % n_threads;
    prodcons[thread_id]->push(make_tuple(from, target, outgoing));
  }
};

void parse_add_pagelink(WikiData& wikidata, const string& line,
    LinkWriteDispatcher &l, bool add_incoming) { 
  if (!line.size() || line[0] == '#')
    return;
  vector<string> tokens;
  split(tokens, line, is_any_of(" ")); 
  if (tokens.size() != 4) {
    cerr << "Reading line " << line << ": Don't know what to do." << endl;
    return;
  }

  string source = tokens[0];
  abbr_ressource(source);

  string target = tokens[2];
  abbr_ressource(target);

  WikiData::ArticleID from_idx = wikidata.find_by_resource(source);
  if (from_idx == (WikiData::ArticleID)-1) {
    // missing links are actually pretty common. Just ignore 'em.
    return;
  }

  WikiData::ArticleID target_idx = wikidata.find_by_resource(target);
  if (target_idx == (WikiData::ArticleID)-1) {
    return;
  }

  l.add_link(from_idx, target_idx, true);
  if (add_incoming) {
    l.add_link(target_idx, from_idx, false);
  }
}

void parse_add_pagelink_thread(WikiData& wikidata, ProducerConsumerQueue<string>& in,
                               LinkWriteDispatcher& out, bool add_incoming) {
  string line;
  while (in.pop(line)) {
    parse_add_pagelink(wikidata, line, out, add_incoming);
  }
}


size_t read_page_links(WikiData &wikidata, const string& linkfile, const bool incoming = false) {
  BzReader r(linkfile);

  // TODO protect linecount with mutex
  size_t linecount = 0;

  ProducerConsumerQueue<string> q;

  LinkWriteDispatcher addlink_dispatch(wikidata, ADD_LINK_THREADS);
  
  vector<thread> threads;
  for (size_t i = 0; i < PARSE_LINK_THREADS; ++i) {
    threads.push_back(thread(parse_add_pagelink_thread,
                             std::ref(wikidata), std::ref(q),
                             std::ref(addlink_dispatch), incoming)); 
  }

  while (!r.done()) {
    linecount += 1;
    try {
      q.push(r.readline());
    } catch (const std::runtime_error &c) {
      cerr << c.what() << endl;
      break;
    }
    if (linecount % 1000000 == 0) {
      cout << "Read: " << linecount << endl;
    }
  }
  
  q.terminate_consumers();
  for (thread& t: threads) {
    t.join(); 
  }

  return linecount;
}

/*}}}*/
