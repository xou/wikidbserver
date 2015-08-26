#pragma once
#include <string>
#include <iostream> // required by urldecode. Probably not required (return true/false instead of cerr).
using namespace std;


/**
 * Performs basic url decoding (%20 -> ...).
 */
bool urldecode(std::string& out, const std::string& in);

/**
 * splits the input string 'in' into a "first" element and everything after it.
 */
bool split_one(string &first, string &rem, const string &in, char delim = ' ');

/**
 * performs basic dbpedia resource normalization.
 * removes the < > tags, removes the dbpedia resource prefix, urldecodes (if possible).
 */
void abbr_ressource(string& source); 

/**
 * parses a string: removes the @<langauge code> suffix,
 * removes quotes.
 */
void parse_nt_string(string &in);

inline bool parse_hex(short &tgt, char c) {
  if (c >= '0' && c <= '9') {
    tgt = c - '0';
    return true;
  }
  if (c >= 'a' && c <= 'f') {
    tgt = 10 + (c - 'a');
    return true;
  }
  if (c >= 'A' && c <= 'F') {
    tgt = 10 + (c - 'A');
    return true; 
  }
  return false;
}

inline void wikipedia_normalization(string &label) {
  for (size_t i = 0; i < label.size(); ++i) {
    if (label[i] == ' ')
      label[i] = '_';
  }
}

inline void wikipedia_denormalization(string &resource) {
  for (size_t i = 0; i < resource.size(); ++i) {
    if (resource[i] == '_')
      resource[i] = ' ';
  } 
}

/*}}}*/
// vim: foldmethod=marker
