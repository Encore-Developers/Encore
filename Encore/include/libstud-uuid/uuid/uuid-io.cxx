#include <libstud/uuid/uuid-io.hxx>

#include <ostream>
#include <istream>
#include <stdexcept> // invalid_argument

using namespace std;

namespace stud
{
  ostream&
  operator<< (ostream& os, const uuid& u)
  {
    return os << u.c_string ().data ();
  }

  istream&
  operator>> (istream& is, uuid& u)
  {
    u = uuid ();

    char s[37];
    if (is.read (s, 36))
    {
      s[36] ='\0';

      try
      {
        u = uuid (s);
      }
      catch (const invalid_argument&)
      {
        is.setstate (istream::failbit);
      }
    }

    return is;
  }
}
