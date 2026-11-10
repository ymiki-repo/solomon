module util_results
# Reading and drawing the CSV files written by sh/common/sweep.sh (<host>_<target>_<arch>/<kind>/model<N>/diffusion_benchmark_*.csv)

using ArgParse
# options shared by the plotting scripts; kind: iterate or verify (the directory made by sh/<system>/<kind>.sh)
function add_common_args!(cfg::ArgParseSettings, kind::String)
    @add_arg_table! cfg begin
        "dirs"
        help = string("directories made by sh/<system>/", kind, ".sh (<host>_<target>_<arch>); default: every directory here that holds ", kind, "/")
        nargs = '*'
        "--caption"
        help = "caption of a DIR in the figures (give once per DIR, in the same order; default: the name of DIR)"
        action = :append_arg
        "--pdf", "-p"
        help = "generate figure in PDF format"
        action = :store_true
        "--usetex"
        help = "typeset the texts with LaTeX (needs latex, dvipng, and the LaTeX packages physics and siunitx)"
        action = :store_true
    end
    return cfg
end
# option --opt of the scripts drawing one optimization level
function add_opt_arg!(cfg::ArgParseSettings)
    @add_arg_table! cfg begin
        "--opt"
        help = "optimization level to draw: OPT_LEVEL of the Makefile (0 to 4) or its name in the CSV files (default, O1, O2, O3, Ofast, O3+ffast-math); give once, or once per DIR (default: 4)"
        action = :append_arg
    end
    return cfg
end

# value per DIR from an option given once per DIR (or once for all when allow_one)
function per_dir(values, num_dirs::Integer, option::String, default; allow_one::Bool=false)
    if isempty(values)
        return fill(default, num_dirs)
    end
    if length(values) == num_dirs
        return values
    end
    if allow_one && length(values) == 1
        return fill(values[begin], num_dirs)
    end
    error(string(option, " is given ", length(values), " times for ", num_dirs, " directories: give it once per directory", allow_one ? " (or once for all)" : ""))
end

# directories to draw: the arguments, or every directory here that holds kind/
function result_dirs(argv, kind::String)
    dirs = [rstrip(dir, '/') for dir in argv["dirs"]]
    if isempty(dirs)
        dirs = sort(filter(dir -> isdir(joinpath(dir, kind)), readdir()))
        if isempty(dirs)
            error(string("no directory made by sh/<system>/", kind, ".sh (holding ", kind, "/) in ", pwd(), ": give the directories as arguments, or run in the sample directory"))
        end
    end
    for dir in dirs
        if !isdir(dir)
            error(string("no directory ", dir, " (give the directories made by sh/<system>/", kind, ".sh)"))
        end
    end
    return dirs
end
captions(argv, dirs) = [something(caption, dir) for (caption, dir) in zip(per_dir(argv["caption"], length(dirs), "--caption", nothing), dirs)]

# OPT_LEVEL of the Makefile from the name written in the CSV files (OPT_LEVEL=4 is Ofast with nvc and O3+ffast-math otherwise)
const LEVELS = Dict("default" => 0, "O1" => 1, "O2" => 2, "O3" => 3, "Ofast" => 4, "O3+ffast-math" => 4)
function parse_level(str::AbstractString)
    if haskey(LEVELS, str)
        return LEVELS[str]
    end
    level = tryparse(Int, str)
    if isnothing(level) || !(0 <= level <= 4)
        error(string("--opt ", str, ": give 0 to 4 or one of ", join(sort(collect(keys(LEVELS)), by=key -> LEVELS[key]), ", ")))
    end
    return level
end
opt_levels(argv, num_dirs::Integer) = parse_level.(per_dir(argv["opt"], num_dirs, "--opt", "4", allow_one=true))

using CSV
using DataFrames
using Glob
# the reason of an error in one line (for a note)
reason(err) = first(split(sprint(showerror, err), '\n'))

# results of a model in DIR (columns level, label, n3, perf in GFlop/s); nothing when no CSV file is found or none can be read
function read_results(dir::AbstractString, kind::String, model::Integer)
    files = glob(string(kind, "/model", model, "/diffusion_benchmark_*.csv"), dir)
    if isempty(files)
        return nothing
    end
    df = DataFrame(level=Int[], label=String[], n3=Int[], perf=Float64[])
    num_read = 0
    for file in files
        # a file that cannot be read (empty, or without a column used here) is skipped as a whole
        rows = similar(df, 0)
        try
            csv = CSV.read(file, DataFrame, types=Dict("Optimization_level" => String))
            for row in eachrow(csv)
                level = get(LEVELS, row."Optimization_level", nothing)
                if isnothing(level)
                    println(stderr, "note: unknown optimization level ", row."Optimization_level", " in ", file, " is skipped")
                    continue
                end
                # a run too short to be timed has NaN
                if isfinite(row."Flop/s")
                    push!(rows, (level, row."Optimization_level", row."nx" * row."ny" * row."nz", row."Flop/s" * 1.0e-9))
                end
            end
        catch err
            err isa InterruptException && rethrow()
            println(stderr, "note: ", file, " cannot be read (", reason(err), "); skipped")
            continue
        end
        append!(df, rows)
        num_read += 1
    end
    return num_read == 0 ? nothing : df
end

using Statistics
# maximum and median for each size at an optimization level; nothing when the level is absent
function split_results(df, level::Integer, summary, GPU, lang, mode)
    rows = df[df.level.==level, :]
    if isempty(rows.n3)
        return nothing
    end
    label = rows.label[begin]
    n3 = sort(unique(rows.n3))
    perf_max = Float64[]
    perf_med = Float64[]
    for size in n3
        data = rows[rows.n3.==size, :].perf
        push!(perf_max, maximum(data))
        push!(perf_med, Statistics.median(data))
        push!(summary, (GPU, lang, mode, label, size, perf_max[end], perf_med[end], Statistics.mean(data), minimum(data), length(data)))
    end
    return (n3=n3, max=perf_max, med=perf_med, label=label)
end
new_summary() = DataFrame(GPU=[], lang=[], mode=[], optimization=[], n3=[], perf_max=[], perf_med=[], perf_avg=[], perf_min=[], trials=[])

# lower and upper ends of the values shown in a set of panels
mutable struct Bounds
    lower::Float64
    upper::Float64
end
Bounds() = Bounds(Inf, -Inf)
function extend!(bounds::Bounds, values)
    if !isempty(values)
        bounds.lower = min(bounds.lower, minimum(values))
        bounds.upper = max(bounds.upper, maximum(values))
    end
    return bounds
end

using PyPlot
import ..util_pyplot
function set_ylim(at, bounds::Bounds)
    if isfinite(bounds.lower) && isfinite(bounds.upper) && bounds.lower < bounds.upper
        at.set_ylim(util_pyplot.scale_axis(bounds.lower, bounds.upper, logPlt=false))
    end
end

function plot_lp(fig, at, x, y; point_id=0, line_id=0, color_id=0, label="", open_symbol=false)
    if open_symbol
        at.plot(x, y, util_pyplot.call(fig.point, id=point_id), markersize=fig.ms, linestyle=util_pyplot.call(fig.line, id=line_id), linewidth=fig.lw, color=util_pyplot.call(fig.color, id=color_id), label=label, markerfacecolor="none")
    else
        at.plot(x, y, util_pyplot.call(fig.point, id=point_id), markersize=fig.ms, linestyle=util_pyplot.call(fig.line, id=line_id), linewidth=fig.lw, color=util_pyplot.call(fig.color, id=color_id), label=label)
    end
end

# horizontal axis N_x N_y N_z: ticks at the cubes of the powers of 2 among the sizes drawn
function set_n3_axis(fig, at, n3)
    at.semilogx()
    edges = [n for n in unique(round.(Int, cbrt.(n3))) if ispow2(n) && n^3 in n3]
    if !isempty(edges)
        sort!(edges)
        at.set_xticks(edges .^ 3)
        at.set_xticklabels([string("\$", n, "^3\$") for n in edges])
    end
    at.tick_params(axis="x", which="major", labelsize=0.9 * fig.fs)
    at.grid()
end
xlabel() = L"$N_x N_y N_z$"

# caption of a panel at (x, y) from its upper left corner; texts: the ways to write it, the preferred first (e.g., in one line,
# then in two). The first that fits in the width of the panel is drawn; when none fits, the last is drawn with a smaller font,
# so that a long caption (the default is the name of the result directory) does not run into the next panel
function panel_caption(fig, at, x, y, texts, fontsize)
    caption = at.text(x, y, texts[begin], color="black", fontsize=fontsize, horizontalalignment="left", verticalalignment="top", transform=at.transAxes, bbox=Dict("facecolor" => "white", "edgecolor" => "None", "alpha" => 0.75))
    renderer = fig.fig.canvas.get_renderer()
    room = (1.0 - 2.0 * x) * at.get_window_extent(renderer).width
    width = 0.0
    for text in texts
        caption.set_text(text)
        width = caption.get_window_extent(renderer).width
        if width <= room
            return caption
        end
    end
    caption.set_fontsize(fontsize * room / width)
    return caption
end
# fraction of the panel height that a caption covers, from the top of the panel
function caption_depth(fig, at, caption)
    renderer = fig.fig.canvas.get_renderer()
    return 1.0 - caption.get_window_extent(renderer).transformed(at.transAxes.inverted()).y0
end

# raise the top of the vertical axis so that the data stay below a caption that covers the given fraction from the top
function make_room(at, depth::Float64)
    lower, upper = at.get_ylim()
    at.set_ylim(lower, upper + (upper - lower) * depth / (1.0 - depth))
end
ylabel(usetex::Bool) = usetex ? L"\unit{GFlop/s}" : "GFlop/s"

function save(fig, name::String, output_pdf::Bool)
    if !ispath("fig")
        mkdir("fig")
    end
    fig.fig.savefig(string("fig/", name, ".png"), format="png", dpi=100, bbox_inches="tight")
    if output_pdf
        fig.fig.savefig(string("fig/", name, ".pdf"), format="pdf", bbox_inches="tight")
    end
end

# the four ways of offloading (MODEL_ID % 4 of sh/common/sweep.sh)
const COLUMNS = ["omp (loop)", "omp (distribute)", "acc (kernels)", "acc (parallel)"]

# Draws a grid of panels: a row for each DIR (the first DIR at the top), a column for each way of offloading (COLUMNS).
# lines: the lines of a panel, each (offset, level, mode, label, point_id, line_id, color_id): the model is (column - 1) + offset,
#   level is the optimization level (nothing: the level of the DIR in dir_levels), and label is the legend (nothing: the name of the level).
# stats: :max and/or :med (a figure for each, named <name>_<stat> or <name> for a single stat).
# Panels without data are hidden. Returns the summary of the statistics.
function draw_grid(argv, kind::String, lines, stats, name::String; dir_levels=nothing, legend_each_row::Bool=false)
    usetex = argv["usetex"]
    dirs = result_dirs(argv, kind)
    caption = captions(argv, dirs)
    Ndir = length(dirs)
    Ncol = length(COLUMNS)
    summary = new_summary()

    # read: data[(dir_id, column)] = list of (line, statistics)
    data = Dict{Tuple{Int,Int},Vector{Any}}()
    for (dir_id, dir) in enumerate(dirs)
        # each model is read once (several lines share it), so that its notes are printed once
        frames = Dict{Int,Any}()
        for column in 1:Ncol
            data[(dir_id, column)] = []
            for line in lines
                model = (column - 1) + line.offset
                if !haskey(frames, model)
                    frames[model] = read_results(dir, kind, model)
                    if isnothing(frames[model])
                        println(stderr, "note: no CSV file that can be read in ", joinpath(dir, kind, string("model", model)), "; ", COLUMNS[column], " (", line.mode, ") is not drawn for ", dir)
                    end
                end
                df = frames[model]
                if isnothing(df)
                    continue
                end
                level = something(line.level, isnothing(dir_levels) ? nothing : dir_levels[dir_id])
                res = split_results(df, level, summary, caption[dir_id], COLUMNS[column], line.mode)
                if isnothing(res)
                    println(stderr, "note: no result at optimization level ", level, " in ", joinpath(dir, kind, string("model", model)), " (found: ", join(unique(df.label), ", "), ")")
                    continue
                end
                push!(data[(dir_id, column)], (line, res))
            end
        end
        if all(isempty(data[(dir_id, column)]) for column in 1:Ncol)
            error(string("nothing to draw for ", dir, ": looked for ", joinpath(dir, kind, "model<N>/diffusion_benchmark_*.csv")))
        end
    end
    n3_all = unique(vcat([res.n3 for list in values(data) for (_, res) in list]...))

    sep = usetex ? "~" : " "
    for stat in stats
        fig = util_pyplot.set_Panel(nx=Ncol, ny=Ndir)
        letter = 0
        for (dir_id, dir) in enumerate(dirs)
            jj = Ndir - dir_id + 1
            bounds = Bounds()
            depth = 0.0 # how far a caption in two lines reaches down in this row
            for column in 1:Ncol
                at = fig.ax[column, jj]
                if isempty(data[(dir_id, column)])
                    at.set_visible(false)
                    continue
                end
                for (line, res) in data[(dir_id, column)]
                    plot_lp(fig, at, res.n3, getfield(res, stat), point_id=line.point_id, line_id=line.line_id, color_id=line.color_id, label=something(line.label, res.label))
                    extend!(bounds, getfield(res, stat))
                end
                head = string("(", Char(97 + letter), ")", sep, util_pyplot.plain_text(caption[dir_id], usetex=usetex))
                text = panel_caption(fig, at, 0.02, 0.98, [string(head, ", ", COLUMNS[column]), string(head, ",\n", COLUMNS[column])], fig.fs * 0.8)
                if occursin('\n', text.get_text())
                    depth = max(depth, caption_depth(fig, at, text))
                end
                letter += 1
            end
            for column in 1:Ncol
                set_ylim(fig.ax[column, jj], bounds)
                if depth > 0.0 && fig.ax[column, jj].get_visible()
                    make_room(fig.ax[column, jj], depth)
                end
            end
        end
        for at in fig.ax
            set_n3_axis(fig, at, n3_all)
        end
        # labels of the horizontal axis on the lowest panel shown in each column
        for column in 1:Ncol
            shown = [jj for jj in 1:Ndir if fig.ax[column, jj].get_visible()]
            if !isempty(shown)
                at = fig.ax[column, minimum(shown)]
                at.tick_params(labelbottom=true)
                at.set_xlabel(xlabel(), fontsize=fig.fs)
            end
        end
        for jj in 1:Ndir
            fig.ax[begin, jj].set_ylabel(ylabel(usetex), fontsize=fig.fs)
        end
        # legends: the first panel shown in the top row (or in every row)
        for jj in (legend_each_row ? (Ndir:-1:1) : (Ndir:Ndir))
            shown = [column for column in 1:Ncol if fig.ax[column, jj].get_visible()]
            if !isempty(shown)
                at = fig.ax[minimum(shown), jj]
                handles, labels = at.get_legend_handles_labels()
                at.legend(handles[end:-1:begin], labels[end:-1:begin], numpoints=1, handlelength=2.0, loc="lower right", fontsize=fig.fs * 0.8)
            end
        end
        save(fig, length(stats) == 1 ? name : string(name, "_", stat), argv["pdf"])
        PyPlot.close(fig.fig)
    end
    return summary
end

end
