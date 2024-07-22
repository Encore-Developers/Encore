#include <libstud/uuid/uuid.hxx>

#include <CoreFoundation/CFUUID.h>

#include <cassert>
#include <cstring>   // memcpy()

using namespace std;

namespace stud
{
  void
  uuid_throw_weak (); // uuid.cxx

  uuid uuid_system_generator::
  generate (bool strong)
  {
    // The common way to generate a UUID on Mac OS is with the CFUUIDCreate()
    // function from the CoreFoundation framework. Interestingly, if we look
    // at the implementation (yes, the CF source code is available), we will
    // see that it uses the <uuid/uuid.h> API which looks like a customized
    // implementation from e2fsprogs that itself is a precursor to libuuid
    // from util-linux.
    //
    // More specifically (and at least as of CF version 1153.18), it calls
    // uuid_generate_random() unless the CFUUIDVersionNumber environment
    // variable is set to 1, in which case it calls uuid_generate_time(). It
    // also appears to serialize these calls so the implementation should be
    // thread-safe (see the Linux implementation for background; this is also
    // the reason why we don't want to use the low-level API directly even if
    // we provide our own synchronization: if other code in the process calls
    // CFUUIDCreate() then we will end up with a race).
    //
    // In theory the use of uuid_generate_random() is bad news since it will
    // produce weak pseudo-random UUIDs if no high-quality randomness is
    // available (unlike uuid_generate() which will only produce strong random
    // UUIDs falling back to the MAC/time-based ones; see uuid_generate(3) for
    // details).
    //
    // In practice (and at least as of Mac OS libc version 1244.30), however,
    // uuid_generate_random() uses arc4random(3) which reportedly produces
    // high-quality randomness (and uuid_generate() simply always calls it).
    //
    CFUUIDRef h (CFUUIDCreate (NULL));
    CFUUIDBytes d (CFUUIDGetUUIDBytes (h));
    CFRelease (h);

    uint8_t a[16];
    memcpy (a, &d, 16); // CFUUIDBytes is POD.

    uuid r (a);
    assert (r.variant () == uuid_variant::dce); // Sanity check.

    // If this is a MAC/time-based UUID, then it's possible the time was not
    // obtained in a collision-safe manner (looking at the implementation this
    // seems to be the case; see the Linux implementation for background).
    //
    if (strong && r.version () != uuid_version::random)
      uuid_throw_weak ();

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
