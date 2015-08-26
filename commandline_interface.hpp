#include <set>
#include <iomanip>
#include <queue>
#include <boost/algorithm/string/trim.hpp>
#include "data.hpp"
#include "graph_bfs.hpp"
// Command-line querying /*{{{*/

using namespace std;

class CLI {
  typedef WikiData::ArticleID ArticleID;
  const WikiData &wikidata;
  GraphBFS::ArticleSet path_exclude_set;

  void dump_article_info(ArticleID idx) const {
    // the additional space is on purpose to make selection on command
    // line easier.
    cout << setw(9) << idx << " : "
         << wikidata.resource_by_id(idx) << " \"" << wikidata.label_by_id(idx)
         << '"' << endl;
  }

  void dump_pagelink(WikiData::Pagelink p) const {
    string marker = "[ - ]";
    if (WikiData::is_incoming(p))
      marker[1] = '<';
    if (WikiData::is_outgoing(p))
      marker[3] = '>';
    cout << marker << ' ';
    dump_article_info(WikiData::to_ArticleID(p));
  }


  void query_by_resource(const string &resource) const {
    ArticleID idx = wikidata.find_by_resource(resource);
    if (idx == (ArticleID)-1) {
      cout << "Resource " << resource << " not found." << endl;
    } else {
      dump_article_info(idx);
    }
  }


  void query_by_label(const string &label) const {
    ArticleID idx = wikidata.find_by_label(label);
    if (idx == (ArticleID)-1) {
      cout << "Label " << label << " not found." << endl;
    } else {
      dump_article_info(idx);
    }
  }


  void query_links(const WikiData::ArticleID article, bool include_outgoing = true, bool include_incoming = false) const {
    for (WikiData::Pagelink &p: wikidata.get_links(article, include_outgoing,
                                                            include_incoming)) {
      dump_pagelink(p);    
    }
  }


  void query_by_id(const WikiData::ArticleID article) const {
    wikidata.check_articleid(article);
    dump_article_info(article);
  }


  static void query_help() {
    cout << "valid commands are:" << endl;
    cout << " resource <resource>" << endl;
    cout << " label <label>" << endl;
    cout << " id <id>" << endl;
    cout << " outs <id>" << endl;
    cout << " ins <id>" << endl;
    cout << " inouts <id>" << endl;
    cout << " path <from> <to>" << endl;
    cout << " path* <from> <to>" << endl;
    cout << " path-undirected[*] <from> <to>" << endl;
    cout << " path-exclude-add <id>" << endl;
    cout << " path-exclude-clear" << endl;
  }


  void dump_path(const GraphBFS::Path &p) const {
    for (const auto& a: p) {
      dump_article_info(a);
    }
  }


  static bool abort_ask() {
    while (true) {
      cout << "[n]ext/[a]bort: " << flush;
      string cmd;
      getline(cin, cmd);
      if (!cmd.size() || cmd == "n")
        return false;
      if (cmd == "a")
        return true;
    }
  }


  void graph_interface(const string& cmd, const string &rem, bool undirected) {
      string from, to;
      split_one(from, to, rem);
      ArticleID from_idx = stoul(from);
      ArticleID to_idx = stoul(to);
      GraphBFS bfs(wikidata, path_exclude_set, from_idx, to_idx, undirected);

      while (true) {
        GraphBFS::Path next = bfs.next(); 
        if (!next.size())
          break;
        dump_path(next);

        if (cmd[cmd.size()-1] == '*') {
          if (abort_ask())
            return;
        } else {
          return;
        }
      }
  }

  void run_query(string& input) {
    boost::trim(input);

    string first, rem;
    split_one(first, rem, input);

    if (first == "resource") {
      query_by_resource(rem);
    } else if (first == "label") {
      query_by_label(rem);
    } else if (first == "id") {
      ArticleID idx = stoul(rem);
      query_by_id(idx);
    } else if (first == "outs") {
      ArticleID idx = stoul(rem);
      query_links(idx, true, false);
    } else if (first == "ins") {
      ArticleID idx = stoul(rem);
      query_links(idx, false, true);
    } else if (first == "inouts") { 
      ArticleID idx = stoul(rem);
      query_links(idx, true, true);
    } else if (first == "path" || first == "path*") {
      graph_interface(first, rem, false);
    } else if (first == "path-undirected" || first == "path-undirected*") {
      graph_interface(first, rem, true);
    } else if (first == "path-exclude-add") {
      ArticleID excl = stoul(rem);
      wikidata.check_articleid(excl);
      path_exclude_set.insert(excl); 
    } else if (first == "path-exclude-clear") {
      path_exclude_set.clear();
    } else {
      query_help();
    }
  }


  void query() {

    query_help();
    while (true) {
      try {
        cout << "> " << flush;
        string line;
        getline(cin, line);
        if (cin.eof())
          break;
        auto clock_start = chrono::system_clock::now();
        run_query(line);
        auto clock_stop = chrono::system_clock::now();
        cout << "[" << (chrono::duration_cast<chrono::milliseconds>(clock_stop-clock_start).count()/1000.0) << "s]" << endl;
      } catch (std::invalid_argument &e) {
        cerr << "Invalid argument [" << e.what() << "]" << endl;
      } catch (std::runtime_error& e) {
        cerr << "Runtimme Error:" <<  e.what() << endl;
      }
    }
  }

  public:
  CLI(const WikiData& wikidata) : wikidata(wikidata) {

  }

  void run() {
    query();
  }
};
/*}}}*/
// vim: foldmethod=marker
