#include "data.hpp"
#include <algorithm>
#include <vector>
#include <set>
#include <queue>

using namespace std;

/**
 * A basic breadth-first search, with basic trivial cycle avoidance
 */
class GraphBFS {

protected:
  const WikiData& wikidata;
  typedef WikiData::ArticleID ArticleID;

  const ArticleID from;
  const ArticleID to;

public:
  typedef vector<ArticleID> Path;
  typedef set<ArticleID> ArticleSet;


protected:
  ArticleSet& exclude_set;

  const ArticleID VISITED_BIT =  ~(((ArticleID)-1) >> 1);

  const ArticleID ARTICLE_MASK = -1 & ~VISITED_BIT /*& ~ADJACENT_BIT*/;
  const ArticleID UNVISITED = ARTICLE_MASK;
  
  vector<ArticleID> data;
  queue<ArticleID> work;
  
  template<typename T>
  static constexpr T get_msb() {
    return ~((T)-1 >> 1);
  }


  template<typename T>
  static void set_msb(T &d) {
    d |= get_msb<T>();
  }


  template<typename T>
  static bool test_msb(T &d) {
    return d & get_msb<T>();
  }


  bool is_visited(ArticleID article) const {
    return data[article] & VISITED_BIT;
  }


  void set_visited(ArticleID article) {
    data[article] |= VISITED_BIT;
  }


  ArticleID get_parent(ArticleID article) const {
    return data[article] & ARTICLE_MASK;
  }


  /**
   *Set parent. Does not override previously existing parent unless 'force' is set.
   */
  void set_parent(ArticleID article, ArticleID parent, bool force = false) {
    if (force || get_parent(article) > data.size()) {
      data[article] = (data[article] & ~ARTICLE_MASK) | (ARTICLE_MASK & parent);
    }
  }


  Path backtrack(ArticleID root, ArticleID current) const {
    vector<ArticleID> path;
    path.push_back(current);
    do {
      current = get_parent(current);
      path.push_back(current);
    } while (current != root);
    
    reverse(path.begin(), path.end());
    return path;
  }


  bool undirected;
public:
  GraphBFS(const WikiData& wikidata, ArticleSet& path_exclude_set,
      ArticleID from, ArticleID to, bool undirected=false)
    : wikidata(wikidata), from(from), to(to),
      exclude_set(path_exclude_set), undirected(undirected) {

    data.clear();
    data.resize(wikidata.links.size(), UNVISITED);

    wikidata.check_articleid_linkdb(from);
    wikidata.check_articleid_linkdb(to);

    if (exclude_set.count(to)) {
      throw std::runtime_error("Error: 'to' node is contained in the excluded nodes.");
    }
    
    work.push(from);
    set_visited(from);
  }

  /**
   * Returns the next shortest path, an empty path if no further paths exist.
   */
  Path next() { 
    while (!work.empty()) {
      ArticleID currentArticle = work.front();
      work.pop();

      for (const WikiData::Pagelink& l: wikidata.links[currentArticle]) {
        if (!undirected && !WikiData::is_outgoing(l))
          continue;

        ArticleID nextArticle = WikiData::to_ArticleID(l);

        if (nextArticle == to) {
          set_parent(nextArticle, currentArticle, true);
          return backtrack(from, to);
        } else {
          if (is_visited(nextArticle))
            continue;
          if (exclude_set.count(nextArticle))
            continue;
          set_parent(nextArticle, currentArticle);
          set_visited(nextArticle);
          work.push(nextArticle);
        }
      }
    }
    return Path();
  }


};

