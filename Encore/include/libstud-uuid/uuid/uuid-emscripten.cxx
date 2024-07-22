#include <libstud/uuid/uuid.hxx>

#include <uuid/uuid.h>

#include <cassert>

using namespace std;

namespace stud
{
  void
  uuid_throw_weak (); // uuid.cxx

  uuid uuid_system_generator::
  generate (bool strong)
  {
    // Emscripten provides the <uuid/uuid.h> header that defines a subset of
    // the Linux libuuid interface. It is implemented in src/library_uuid.js.
    //
    // At the time of this writing, the implementation always generates a
    // random (version 4) UUID. It attempts to use (supposedly) high quality
    // randomness if available but falls back to (presumably) poor quality
    // Math.random otherwise. Since there is no indication of which source was
    // used, we cannot assume the result is strong.
    //
    // NOTE: update tests if changing this.
    //
    if (strong)
      uuid_throw_weak ();

    uuid_t d;
    uuid_generate (d);

    uuid r (d);
    assert (r.variant () == uuid_variant::dce); // Sanity check.

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
