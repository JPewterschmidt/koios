-- Lua script to read files recursively and find TODO literals

function findTODO(path)
    for entry in io.open('ls -1 "'..path..'"'):lines() do
        local fullpath = path..'/'..entry
        local mode = io.open('stat -c "%a" "'..fullpath..'"'):read()
        
        if mode and mode:sub(1, 1) == 'd' then
            -- If it's a directory, recurse into it
            findTODO(fullpath)
        elseif mode and mode:sub(4, 4) == 'x' then
            -- If it's an executable file, attempt to read it
            local file = io.open(fullpath, 'r')
            if file then
                local content = file:read('*all')
                if content:find('TODO') then
                    print('TODO found in: '..fullpath)
                end
                file:close()
            end
        end
    end
end
