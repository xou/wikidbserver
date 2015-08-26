#include "parseutil.hpp"

#include <string>

// Generic string parsing /*{{{*/

bool urldecode(std::string& out, const std::string& in) {
  out.clear();
  out.reserve(in.size());
  bool success = true;
  for (size_t i = 0; i < in.size(); ++i) {
    if (in[i] == '+') {
      out.push_back(' ');
      continue;
    }

    if (in[i] != '%') {
      out.push_back(in[i]);
      continue;
    }

    // in[i] == '%'
    if (i + 1 < in.size() && in[i+1] == '%') {
      i += 1;
      out.push_back('%');
      continue;
    }

    if (i + 3 <= in.size()) {
      short high, low;
      if (parse_hex(high, in[i+1]) && parse_hex(low, in[i+2])) {
         out.push_back(high*16+low);
         i += 2;
         continue;
      }
    }

    out.push_back(in[i]);
    success = false;
  }
  return success;
}


bool split_one(string &first, string &rem, const string &in, char delim) {
  size_t pos = in.find(delim);
  first = in.substr(0, pos);
  if (pos == in.npos || pos == (in.size()-1)) {
    rem.clear();
    return false;
  }
  rem = in.substr(pos+1);
  return true; 
}

/*}}}*/
// DBPedia-related parse utilities /*{{{*/
// removes < and > from start and end of a token.
// returns true if <> was removed, false otherwise.
bool strip_tag(string &token) {
 if (token.size() > 2 && token[0] == '<' && token[token.size()-1] == '>') {
    token = token.substr(1, token.size()-2);
    return true;
 }
 return false;
}


const string DBPEDIA_RESOURCE_PREFIX("dbpedia.org/resource/");
// removes DBPREDIA_RESOURCE_PREFIX from beginning of resource and returns true, returns false
// if string didn't start with DBPEDIA_RESOURCE_PREFIX.
bool stripDbpediaPrefix(string& resource) {
  if (resource.find("http://") != 0)
    return false;
  size_t pos = resource.find(DBPEDIA_RESOURCE_PREFIX);
  if (pos >= 15) // accounts for http://simple.$DBPEDIA_RESOURCE_PREFIX
    return false;

  resource = resource.substr(pos+DBPEDIA_RESOURCE_PREFIX.size());
  return true;
}


/**
 * performs basic dbpedia resource normalization.
 * removes the < > tags, removes the dbpedia resource prefix, urldecodes (if possible).
 */
void abbr_ressource(string& source) {
  strip_tag(source);
  stripDbpediaPrefix(source);
  string urldecoded_source;
  
  if (!urldecode(urldecoded_source, source)) {
    cerr << "Warning: could not urldecode " << source << endl;
  } else {
    source = urldecoded_source;
  }
}


/**
 * parses a string: removes the @<langauge code> suffix,
 * removes quotes.
 */
void parse_nt_string(string &in) {
  if (in.size() > 2 && in.rfind("@") == in.size()-3) {
    in = in.substr(0, in.size()-3);
  }
  if (in.size() > 2 && in[0] == '"' && in[in.size()-1] == '"') {
    in = in.substr(1, in.size()-2);
  }
}
