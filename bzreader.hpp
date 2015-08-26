#pragma once
#include <string>
#include <cstring>
#include <stdexcept>
#include <bzlib.h>
#include <fcntl.h>
#include <errno.h>

using namespace std;
/**
 * Simple wrapper around libbz2, because i ran into this bug:
 * http://stackoverflow.com/questions/3167109/exceptions-from-boostiostreamscopy
 */
class BzReader {
  FILE * _file;
  void * _bzfile;
  const char linedelim = '\n';
  const static size_t buffsize = 4096;
  char buffer[buffsize];

  int nunused = 0;
  const static size_t max_unused = 5000; // BZ_MAX_UNUSED as of bzlib2 1.0.5
  void* unused = NULL;
  char* unused_buffer[max_unused];

  size_t buffer_end = buffsize;
  size_t next_read = buffsize;

  bool eof = false;

  public:
  BzReader(string filename) {
    _file = fopen(filename.c_str(), "r");
    if (_file == NULL) {
      throw std::runtime_error("Unable to open file " + filename + ", errno=" + to_string(errno) + " (" + strerror(errno) + ")");
    }
    initialize_bzstream();
  }

  private:
  void initialize_bzstream() {
    int error;
    _bzfile = BZ2_bzReadOpen( &error, _file, 0 /* verbosity */,  0 /* low memory usage flag */,
        unused_buffer, nunused /* = 0 */);
    if (error != BZ_OK) {
      int bz_err = error;
      BZ2_bzReadClose( &error, _bzfile );
      throw std::runtime_error("Unable to decompress file: bz2_bzReadOpen error " + to_string(bz_err));
    }
  }

  // returns false if there isn't any more data.
  bool read_more() {
    if (eof)
      return false;
    int bzerror = BZ_OK;
    int nBuf = BZ2_bzRead(&bzerror, _bzfile, buffer, buffsize);
    if (bzerror != BZ_OK && bzerror != BZ_STREAM_END) {
      int bzerror_bak = bzerror;
      int errno_bak = errno;
      throw std::runtime_error("Reading failed: bz2_bzRead yielded error code " + to_string(bzerror_bak) + " close: -> " + to_string(bzerror) + " errno is " + to_string(errno_bak));
    }

    if (bzerror == BZ_STREAM_END) {
      if (feof(_file)) {
        // we're really at the end now.
        eof = true;
        if (nBuf == 0) {
          buffer_end = 0;
          return false;
        }
        buffer_end = nBuf;
        return true;
      }
      BZ2_bzReadGetUnused(&bzerror, _bzfile,
          &unused, &nunused);
      memcpy(unused_buffer, unused, nunused);
      BZ2_bzReadClose(&bzerror, _bzfile);
      initialize_bzstream();
    }

    buffer_end = nBuf;
    return true; 
  }

  void close_stream() {
    int bzerror;
    BZ2_bzReadClose(&bzerror, _bzfile);
  }

  public:
  ~BzReader() {
    close_stream();
    fclose(_file);
  }

  string readline() {
    string ret;
    ret.reserve(buffsize);
    while (true) {
      while (next_read < buffer_end) {
        if (buffer[next_read] == linedelim) {
          next_read++;
          return ret;
        }
        ret.push_back(buffer[next_read]);
        next_read++;
      }
      
      if (!read_more()) {
        return ret;
      }
      next_read = 0;
    }

    throw std::runtime_error("How did i get here?");
  }


  bool done() const {
    return eof;
  }
};

