#include <boost/token_functions.hpp>

using namespace boost;

  //===========================================================================
  // this is a modified version of boost/token_functions.hpp/escaped_list_separator.
  // instead of throwing an "invalid escape sequence", this version includes the
  // "invalid" escape sequence into the output token.
  // This allows unicode sequences, such as "\\u1234" to be included in the
  // resulting string.


// The out of the box GCC 2.95 on cygwin does not have a char_traits class.
// MSVC does not like the following typename
  template <class Char,
    class Traits = BOOST_DEDUCED_TYPENAME std::basic_string<Char>::traits_type >
  class escaped_list_separator_includeinvalid {

  private:
    typedef std::basic_string<Char,Traits> string_type;
    struct char_eq {
      Char e_;
      char_eq(Char e):e_(e) { }
      bool operator()(Char c) {
        return Traits::eq(e_,c);
      }
    };
    string_type  escape_;
    string_type  c_;
    string_type  quote_;
    bool last_;

    bool is_escape(Char e) {
      char_eq f(e);
      return std::find_if(escape_.begin(),escape_.end(),f)!=escape_.end();
    }
    bool is_c(Char e) {
      char_eq f(e);
      return std::find_if(c_.begin(),c_.end(),f)!=c_.end();
    }
    bool is_quote(Char e) {
      char_eq f(e);
      return std::find_if(quote_.begin(),quote_.end(),f)!=quote_.end();
    }
    template <typename iterator, typename Char_>
    bool do_escape(iterator& next,iterator end,Char_& nextc) {
      if (++next == end)
        BOOST_THROW_EXCEPTION(escaped_list_error(std::string("cannot end with escape")));
      if (Traits::eq(*next,'n')) {
        nextc = '\n';
        return true;
      }
      else if (is_quote(*next)) {
        nextc = *next;
        return true;
      }
      else if (is_c(*next)) {
        nextc = *next;
        return true;
      }
      else if (is_escape(*next)) {
        nextc = *next;
        return true;
      }
      else {
        nextc = *next;
        return false;
      }
    }

    public:

    explicit escaped_list_separator_includeinvalid(Char  e = '\\',
                                    Char c = ',',Char  q = '\"')
      : escape_(1,e), c_(1,c), quote_(1,q), last_(false) { }

    escaped_list_separator_includeinvalid(string_type e, string_type c, string_type q)
      : escape_(e), c_(c), quote_(q), last_(false) { }

    void reset() {last_=false;}

    template <typename InputIterator, typename Token>
    bool operator()(InputIterator& next,InputIterator end,Token& tok) {
      bool bInQuote = false;
      tok = Token();

      if (next == end) {
        if (last_) {
          last_ = false;
          return true;
        }
        else
          return false;
      }
      last_ = false;
      for (;next != end;++next) {
        if (is_escape(*next)) {
          Char backup = *next;
          Char nextc;
          if (do_escape(next,end,nextc)) {
            tok += nextc;
          } else {
            // invalid escape sequence, include verbatim
            tok += backup;
            tok += nextc;
          }
        }
        else if (is_c(*next)) {
          if (!bInQuote) {
            // If we are not in quote, then we are done
            ++next;
            // The last character was a c, that means there is
            // 1 more blank field
            last_ = true;
            return true;
          }
          else tok+=*next;
        }
        else if (is_quote(*next)) {
          bInQuote=!bInQuote;
        }
        else {
          tok += *next;
        }
      }
      return true;
    }
  };
