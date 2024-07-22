#include <libstud/uuid/uuid.hxx>

#include <uuid.h>

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
    // The OpenBSD uuid_*() (<uuid.h>, uuid_compare(3)) API generates version
    // 4 UUIDs (i.e. randomly generated) at least from version 6.4. For now we
    // will assume that only random ones are strong.
    //
    // Here we assume uuid_t has the same definition as in FreeBSD/NetBSD (it
    // is defined in <sys/uuid.h>).
    //
    uuid_t d;
    uint32_t s;
    uuid_create (&d, &s);

    // None of the uuid_s_* errors seem plausible for this function so let's
    // return the generic "not supported" error code.
    //
    if (s != uuid_s_ok)
      throw system_error (ENOSYS, system_category ());

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
