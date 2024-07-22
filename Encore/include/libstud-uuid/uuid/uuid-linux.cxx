#include <libstud/uuid/uuid.hxx>

#include <errno.h>
#include <dlfcn.h>

#include <mutex>
#include <cassert>
#include <utility>      // move()
#include <system_error>

using namespace std;

namespace stud
{
  // While we can safely assume libuuid.so.1 is present on every Linux machine
  // (it is part of the essential util-linux package since version 2.15.1),
  // the development files (uuid.h, .so/.a symlinks) are a different story.
  // So for maximum portability we will use dlopen()/dlsym() to "link" to this
  // library without any development files.
  //
  // It appears that not all execution paths in uuid_generate() are thread-
  // safe (see Ubuntu bug #1005878). So for now the calls are serialized but
  // maybe this could be optimized (e.g., if we can be sure a thread-safe path
  // will be taken). Note also that we may still end up in trouble if someone
  // else in the process calls libuuid directly.
  //
  // Note also that the Linux kernel has support for generatin random UUIDs
  // which is exposed to userspace via /proc/sys/kernel/random/uuid. This
  // could be another implementation option (though it's not clear since which
  // version it is available, seem at least from the 2.6 days).
  //
  using lock = unique_lock<mutex>;

  static mutex uuid_mutex;

  // <uuid.h>
  //
  using uuid_t = unsigned char[16];

  static void (*uuid_generate)           (uuid_t);
  static int  (*uuid_generate_time_safe) (uuid_t);

  static void* libuuid;

  // Use a union to cleanly cast dlsym() result (void*) to a function pointer.
  //
  template <typename F>
  static inline F
  function_cast (void* p)
  {
    union { void* p; F f; } r;
    r.p = p;
    return r.f;
  };

  static inline void
  dlfail (string what)
  {
    what += ": ";
    what += dlerror ();
    throw system_error (ENOSYS, system_category (), move (what));
  };

  void uuid_system_generator::
  initialize ()
  {
    assert (libuuid == nullptr);

    libuuid = dlopen ("libuuid.so.1", RTLD_LAZY | RTLD_GLOBAL);

    if (libuuid == nullptr)
      dlfail ("unable to load libuuid.so.1");

    uuid_generate =
      function_cast<decltype(uuid_generate)> (
        dlsym (libuuid, "uuid_generate"));

    if (uuid_generate == nullptr)
      dlfail ("unable to lookup uuid_generate() in libuuid.so.1");

    uuid_generate_time_safe =
      function_cast<decltype(uuid_generate_time_safe)> (
        dlsym (libuuid, "uuid_generate_time_safe"));

    // Delay the failure until/if we need this function (it was only added in
    // 2011 so may not be available on older systems).
    //
    //if (uuid_generate_time_safe == nullptr)
    //  dlfail ("unable to lookup uuid_generate_time_safe() in libuuid.so.1");
  }

  void uuid_system_generator::
  terminate ()
  {
    assert (libuuid != nullptr);

    if (dlclose (libuuid) != 0)
      dlfail ("unable to unload libuuid.so.1");

    libuuid = nullptr;
  }

  void
  uuid_throw_weak (); // uuid.cxx

  uuid uuid_system_generator::
  generate (bool strong)
  {
    lock l (uuid_mutex);

    if (libuuid == nullptr)
      initialize ();

    uuid_t d;
    uuid_generate (d);

    uuid r (d);
    assert (r.variant () == uuid_variant::dce); // Sanity check.

    // According to the uuid_generate() documentation (and confirmed by the
    // implementation) it generates a random uuid if high-quality randomness
    // is available and a MAC/time-based one otherwise (in other words, it
    // should never generate a pseudo-random UUID).
    //
    if (strong && r.version () != uuid_version::random)
    {
      if (uuid_generate_time_safe == nullptr ||
          uuid_generate_time_safe (d) == -1)
        uuid_throw_weak ();

      r.assign (d);
      assert (r.variant () == uuid_variant::dce);
    }

    return r;
  }
}
