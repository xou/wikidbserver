#pragma once

#include "parseutil.hpp"
#include <vector>
#include <mutex>
#include <stdexcept>
#include <functional>
#include <memory>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/iterator/filter_iterator.hpp>

using namespace std;

class WikiData {
public:
  typedef uint32_t Pagelink;
  typedef uint32_t ArticleID;

  // Not really a technical requirement, rather a protection - copying
  // potentially multiple GB of data is most likely a programming error.
  // Moving is ok though.
  WikiData(const WikiData& other) = delete;
  WikiData& operator=(const WikiData& other) = delete;
  WikiData() { };

  /**
   * For convenience and performance, allow direct access
   * to internal data structures from outside this class.
   */

  /**
   * Pages can be addressed by both a resource (URL) or
   * label (Title).
   * "Compressed" labels is an attempt at memory efficient
   * storage of this relation. If wikipedia_denormalize returns
   * the exact Title, it is not stored separately.
   * Otherwise, the CompressedLabel consists of the resource
   * followed by the label, delimited by a '\0' character.
   */
  typedef string CompressedLabel;
  vector<CompressedLabel> labels;
  mutex labels_write;


  static string get_resource(const CompressedLabel& compressed) { 
    size_t zero = compressed.find('\0');
    if (zero == string::npos) {
      return compressed;
    }
    return compressed.substr(0, zero);
  }


  static string get_label(const CompressedLabel& compressed) {
    size_t zero = compressed.find('\0');
    if (zero == string::npos) {
      string ret = compressed;
      wikipedia_denormalization(ret);
      return ret;
    }
    return compressed.substr(zero+1);
  }


  /**
   * returns the ArticleID by the given resource, or -1.
   */
  ArticleID find_by_resource(const string& resource) const { 
    auto it = lower_bound(labels.begin(), labels.end(), resource);
    if (it == labels.end() || get_resource(*it) != resource) {
      return -1;
    }
    return it - labels.begin();
  }

  
  /**
   * returens the ArticleID corresponding to the given label.
   * returns -1 on failure.
   */
  ArticleID find_by_label(const string &label) const {
    string normalized = label;
    wikipedia_normalization(normalized);
    auto it = lower_bound(labels.begin(), labels.end(), normalized);
    if (it == labels.end() || get_label(*it) != label) {
      return -1;
    }
    return it - labels.begin();
  }


  /**
   * Throws std::runtime_error if the article was not found.
   */
  string label_by_id(ArticleID article) const {
    check_articleid(article);
    return get_label(labels[article]);
  }


  /**
   * Throws std::runtime_error if the article was not found.
   */
  string resource_by_id(ArticleID article) const {
    check_articleid(article);
    return get_resource(labels[article]);
  }


  bool idx_to_label(string &out, const ArticleID article) const {
    if (article >= labels.size()) {
      out.clear();
      return false;
    }
    out = get_label(labels[article]);
    return true;
  }


  void check_articleid(ArticleID article) const {
    if (article < labels.size())
      return;
    throw std::runtime_error("Article ID not found: " + to_string(article));
  }

  /**
   * Primary index in links is the article id (index in labels).
   * nested vector contains the page links. In each element, 
   * the 30 most significant bits describe the article id.
   * the LSB is set when the page link is outgoing (from the primary id),
   * the second bit is set when the link is incoming.
   *
   * LSB (instead of MSBs) have been chosen to allow lookups for 
   * a specific lookup to happen in O(log n), usint standard sorting.
   * Lookups for all out/in links are only slowed
   * down by, on average, a factor of 2.
   */
  vector<vector<Pagelink>> links;

  /**
   * Adds an incoming or outgoing link to article 'from'.
   * This function is not threadsafe for multiple parallel calls with the same
   * 'from'.
   */
  void add_link_unsafe(const ArticleID from, ArticleID target, bool outgoing) {
    if (from >= links.size())
      return;


    auto it = lower_bound(links[from].begin(), links[from].end(), to_pagelink(target));
    if (it == links[from].end() || !is_link_to_article(*it, target)) {
      links[from].insert(it, to_pagelink(target, outgoing, !outgoing));
    } else {
      *it = *it | to_pagelink(0, outgoing, !outgoing);
    }
  }


  /**
   * return a vector of all links from (outgoing) and/or to (incoming)
   * the source node.
   * throws std::runtime_error when used without a link database
   * or invalid source id.
   */
  vector<Pagelink> get_links(ArticleID source, bool outgoing=true,
                             bool incoming=false) const {
    vector<ArticleID> ret;
    check_articleid_linkdb(source);
    for (const Pagelink &p: links[source]) {
      if ((incoming && is_incoming(p)) ||
          (outgoing && is_outgoing(p))) {
        ret.push_back(p);
      }
    }
    return ret;
  }

  /**
   * Check whether the pagelink referes to a given article.
   */
  static bool is_link_to_article(const Pagelink pagelink, const ArticleID article) {
    if ((pagelink >> 2) == article)
      return true;
    return false;
  }

  static inline bool is_outgoing(const Pagelink pagelink) {
    return pagelink & 0x1;
  }

  static inline bool is_incoming(const Pagelink pagelink) {
    return pagelink & 0x2;
  }

  static inline ArticleID to_ArticleID(const Pagelink pagelink) {
    return pagelink >> 2;
  }

  static inline Pagelink to_pagelink(const ArticleID article, bool outgoing = false,
      bool incoming = false) {
    return ((article << 2) | ((int)incoming << 1) | ((int)outgoing));
  }


  void check_articleid_linkdb(ArticleID article) const {
    if (article < links.size())
      return;
    if (links.size() == 0) {
      throw std::runtime_error("Cannot use this function without page link database loaded");
    }
    throw std::runtime_error("Invalid id for use with page links: " + to_string(article));
  }


  bool outlink_exists(const ArticleID& root, const ArticleID& other) const {
    check_articleid_linkdb(root);
    Pagelink other_pl = to_pagelink(other);
    auto it = lower_bound(links[root].begin(), links[root].end(), other_pl);
    return (is_link_to_article(*it, other) && is_outgoing(*it));
  }
};
