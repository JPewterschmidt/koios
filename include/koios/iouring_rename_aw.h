#ifndef KOIOS_IOURING_RENAME_AW_H
#define KOIOS_IOURING_RENAME_AW_H

#include <filesystem>

#include "koios/macros.h"
#include "koios/iouring_aw.h"

#include "toolpex/unique_posix_fd.h"

namespace koios::uring
{
    struct rename_stuff
    {
        rename_stuff(::std::string from, ::std::string to)
            : m_from_fullpath{ ::std::move(from) }, 
              m_to_fullpath{ ::std::move(to) }
        {
        }

    protected:
        ::std::string m_from_fullpath{};
        ::std::string m_to_fullpath{};
    };

    class rename : public iouring_aw, private rename_stuff
    {
    public:
        rename(const ::std::filesystem::path& from, 
               const ::std::filesystem::path& to);
    };

    class renameat : public iouring_aw, private rename_stuff
    {
    public:
        renameat(const toolpex::unique_posix_fd& olddir, const ::std::filesystem::path& from, 
                 const toolpex::unique_posix_fd& newdir, const ::std::filesystem::path& to, 
                 int flags = 0);
    };

    class renameat_noreplace : public iouring_aw, private rename_stuff
    {
    public:
        renameat_noreplace(const toolpex::unique_posix_fd& olddir, 
                           const ::std::filesystem::path& from, 
                           const toolpex::unique_posix_fd& newdir, 
                           const ::std::filesystem::path& to);
    };
}

#endif
