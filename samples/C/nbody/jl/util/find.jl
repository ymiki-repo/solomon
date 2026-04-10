module util_find

using FilePaths
using Glob

function latest_series()
    list = glob("dat/*.txt")
    latest = 0
    candidate = list[begin]
    for file in list
        date = stat(file).mtime
        if date > latest
            latest = date
            candidate = file
        end
    end
    return filename(Path(candidate))
end

end
