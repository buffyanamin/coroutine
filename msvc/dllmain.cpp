// ---------------------------------------------------------------------------
//
//  Author
//      Park DongHa     | luncliff@gmail.com
//
//  License
//      CC BY 4.0
//
// ---------------------------------------------------------------------------
#include <cassert>
#include <magic/date_time.hpp>

#ifdef _WIN32
#define PROCEDURE
#else
#define PROCEDURE __attribute__((constructor))
#endif

namespace magic {
PROCEDURE void on_load(void *) noexcept(false) {
  // this function is reserved for
  // future initialization setup
}
} // namespace magic

#ifdef _WIN32
#include <sdkddkver.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// https://msdn.microsoft.com/en-us/library/windows/desktop/ms682583(v=vs.85).aspx
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID) {
  try {
    if (fdwReason == DLL_PROCESS_ATTACH)
      magic::on_load(hinstDLL);

    return TRUE;
  } catch (const std::runtime_error &e) {
    ::perror(e.what());
  } catch (const std::exception &e) {
    ::perror(e.what());
  }
  return FALSE;
}

#endif // _WIN32
