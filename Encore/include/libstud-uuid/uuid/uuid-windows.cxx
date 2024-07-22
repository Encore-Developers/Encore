#include <libstud/uuid/uuid.hxx>

#include <rpc.h>   // UuidCreate*()
#include <errno.h>

#include <cassert>
#include <system_error>

using namespace std;

namespace stud
{
  void uuid::
  assign (const _GUID& d)
  {
    time_low  = d.Data1;
    time_mid  = d.Data2;
    time_hiv  = d.Data3;
    clock_seq_hir = d.Data4[0];
    clock_seq_low = d.Data4[1];
    memcpy (node, &d.Data4[2], 6);
  }

  template<>
  LIBSTUD_UUID_SYMEXPORT _GUID uuid::
  guid<_GUID> () const
  {
    _GUID r;
    r.Data1 = time_low;
    r.Data2 = time_mid;
    r.Data3 = time_hiv;
    r.Data4[0] = clock_seq_hir;
    r.Data4[1] = clock_seq_low;
    memcpy (&r.Data4[2], node, 6);
    return r;
  }

  void
  uuid_throw_weak (); // uuid.cxx

  uuid uuid_system_generator::
  generate (bool strong)
  {
    // The common way to generate a UUID on Windows is with the CoCreateGuid()
    // function which, according to the documentation, calls UuidCreate().
    // There is some talk of it somehow generating stronger UUIDs but it is
    // probably bogus (it most likely simply returns an error if UuidCreate()
    // returns a "weak" UUID).
    //
    // UuidCreate(), on the other hand, has various interesting return codes
    // as well as the UuidCreateSequential() variant which always generates a
    // MAC/time-based UUID (according to the documentation, since Windows 2000
    // UuidCreate() always generates a random UUID). So let's use that.
    //
    // While the documentation doesn't explicitly state this, presumably
    // UuidCreate() uses a "decent" source of randomness. See this post for
    // some background:
    //
    // https://stackoverflow.com/questions/35366368/does-uuidcreate-use-a-csprng
    //
    // So we assume a random UUID returned by UuidCreate() is strong. What to
    // do if it's not is a tricky question (is it even possible?): we can call
    // UuidCreateSequential() to generate a MAC/time-based UUID but it's
    // possible the time was not obtained in a collision-safe manner (see the
    // Linux implementation for background). However, the documentation
    // suggests that it is safe ("... guaranteed to be unique among all UUIDs
    // generated on the computer"). And so we assume it is.
    //
    auto rpcfail = [strong] (RPC_STATUS s)
    {
      if (s != RPC_S_UUID_LOCAL_ONLY)
      {
        char m [DCE_C_ERROR_STRING_LEN];

        throw system_error (
          ENOSYS,
          system_category (),
          (DceErrorInqTextA (s, reinterpret_cast<RPC_CSTR> (m)) == RPC_S_OK
           ? m
           : "unknown RPC error"));
      }
      else if (strong)
        uuid_throw_weak ();
    };

    UUID d;
    RPC_STATUS s (UuidCreate (&d));

    if (s != RPC_S_OK)
      rpcfail (s);

    uuid r (d);
    assert (r.variant () == uuid_variant::dce); // Sanity check.

    if (strong && r.version () != uuid_version::random)
    {
      s = UuidCreateSequential (&d);

      if (s != RPC_S_OK)
        rpcfail (s);

      r.assign (d);

      // UuidCreateSequential() on Wine returns the Microsoft variant.
      //
      if (r.variant () != uuid_variant::dce ||
          r.version () != uuid_version::time)
        rpcfail (RPC_S_UUID_LOCAL_ONLY);
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
