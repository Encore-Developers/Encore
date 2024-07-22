#include <libstud/uuid/uuid.hxx>

#include <sys/uuid.h>

#include <errno.h>

#include <cassert>
#include <cstring>      // memcpy()
#include <system_error>

using namespace std;

namespace stud
{
  void
  uuid_throw_weak (); // uuid.cxx

  uuid uuid_system_generator::
  generate (bool strong)
  {
    // While FreeBSD shares the uuid_*() (<uuid.h>, uuid(3)) API with other
    // BSDs, its documentation is quite light on the kind of UUID we get and
    // with what guarantees. The implementation of uuid_create() simply calls
    // the uuidgen() system call (<sys/uuid.h>, uuidgen(2)) which has some
    // details.
    //
    // Specifically (and as of FreeBSD 11.2), we get a version 1 (MAC/time-
    // based) UUID and it seems there is provision for getting the time in a
    // collision-safe manner:
    //
    // "According to the algorithm of generating time-based UUIDs, this will
    //  also force a new random clock sequence, thereby increasing the
    //  likelihood for the identifier to be unique."
    //
    // So we will assume a MAC/time-based UUID is strong. We also assume a
    // random UUID is strong as well in case in the future this will becomes
    // the default (as seems to be the trend); presumably, FreeBSD folks are
    // smart enough not to start return random UUIDs without a good source of
    // randomness, at least not by default.
    //
    // When it comes to NetBSD, there is this HISTORY note in the uuidgen(2)
    // man page:
    //
    // "It was changed to use version 4 UUIDs, i.e. randomly generated UUIDs,
    // in NetBSD 8.0."
    //
    // And we will assume random NetBSD UUIDs are strong.
    //
    struct ::uuid d;

    if (uuidgen (&d, 1) != 0)
      throw system_error (errno, system_category ());

    uuid r;

    // This is effectively just memcpy() but let's reference the member names
    // in case anything changes on either side.
    //
    r.time_low = d.time_low;
    r.time_mid = d.time_mid;
    r.time_hiv = d.time_hi_and_version;
    r.clock_seq_hir = d.clock_seq_hi_and_reserved;
    r.clock_seq_low = d.clock_seq_low;
    memcpy (r.node, d.node, 6);

    assert (r.variant () == uuid_variant::dce); // Sanity check.

    if (strong)
    {
      switch (r.version ())
      {
      case uuid_version::time:
      case uuid_version::random: break;
      default:                   uuid_throw_weak ();
      }
    }

    return r;
  }

  void uuid_system_generator::
  initialize ()
  {
  }

  void uuid_system_generator::
  terminate ()
  {
  }
}
