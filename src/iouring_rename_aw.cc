#include "koios/iouring_rename_aw.h"
#include <liburing.h>

namespace koios::uring
{
    rename::rename(const ::std::filesystem::path& from, const ::std::filesystem::path& to)
        : rename_stuff{ from, to }
    {
        ::io_uring_prep_rename(sqe_ptr(), m_from_fullpath.c_str(), m_to_fullpath.c_str());
    }

    renameat::renameat(const toolpex::unique_posix_fd& olddir, const ::std::filesystem::path& from, 
                       const toolpex::unique_posix_fd& newdir, const ::std::filesystem::path& to, 
                       int flags)
        : rename_stuff{ from, to }
    {
        ::io_uring_prep_renameat(
            sqe_ptr(), 
            olddir, m_from_fullpath.c_str(), 
            newdir, m_to_fullpath.c_str(), 
            flags
        );
    }

    renameat_noreplace::renameat_noreplace(
            const toolpex::unique_posix_fd& olddir, const ::std::filesystem::path& from, 
            const toolpex::unique_posix_fd& newdir, const ::std::filesystem::path& to)
        : rename_stuff{ from, to }
    {
        ::io_uring_prep_renameat(
            sqe_ptr(), 
            olddir, m_from_fullpath.c_str(), 
            newdir, m_to_fullpath.c_str(), 
            RENAME_NOREPLACE
        );
    }
}
