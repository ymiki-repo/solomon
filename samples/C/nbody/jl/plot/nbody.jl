# Draws the performance measured by sh/<system>/scaling.sh; run in the sample directory:
#   julia --project=jl jl/plot/nbody.jl [options] [DIR ...]
# DIR: a directory made by scaling.sh (<host>_<target>_<arch>, holding scaling/model<N>/nbody/log/*.csv);
#      without DIR, every directory in the current directory that holds scaling/ is drawn
# Writes the figures to plot/ (not fig/, which the job scripts use and take for products of an earlier run) and the
# statistics to nbody_stats.csv.
using ArgParse
function parse_cmd()
    cfg = ArgParseSettings(description="Draw the performance measured by sh/<system>/scaling.sh (run in the sample directory)")
    @add_arg_table cfg begin
        "dirs"
        help = "directories made by scaling.sh (<host>_<target>_<arch>); default: every directory here that holds scaling/"
        nargs = '*'
        "--caption"
        help = "caption of a DIR in the figures (give once per DIR, in the same order; default: the name of DIR)"
        action = :append_arg
        "--reference", "-r"
        help = "glob pattern of the CSV files (columns N and N_pairs/s) of a reference implementation to compare with, e.g., CUDA (give once per DIR, in the same order); adds the panels of the ratio to it"
        action = :append_arg
        "--reference-label"
        help = "name of the reference implementation in the legends (give once, or once per DIR)"
        action = :append_arg
        "--num", "-n"
        help = "number of data points (the largest N) to show the sustained performance"
        arg_type = Int
        default = 4
        "--pdf", "-p"
        help = "generate figure in PDF format"
        action = :store_true
        "--usetex"
        help = "typeset the texts with LaTeX (needs latex, dvipng, and the LaTeX packages physics and siunitx)"
        action = :store_true
    end
    return parse_args(cfg)
end

using Glob
# files matching a glob pattern (relative to the current directory, or absolute)
function find_files(pattern::AbstractString)
    return isabspath(pattern) ? glob(lstrip(pattern, '/'), "/") : glob(pattern)
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

using DataFrames
using CSV
using Statistics
struct Result
    N::Vector{Int}
    max::Vector{Float64}
    med::Vector{Float64}
end

# the reason of an error in one line (for a note)
reason(err) = first(split(sprint(showerror, err), '\n'))

# reads N and N_pairs/s of the CSV files; returns nothing for no files, or when none can be read
function read_results(file_list, summary, GPU, lang, flag)
    num_list = Int[]
    perf_list = Float64[]
    num_read = 0
    for file in file_list
        # a file that cannot be read (empty, or without the columns) is skipped as a whole
        try
            csv = CSV.read(file, DataFrame)
            num = convert(Vector{Int}, csv."N")
            perf = convert(Vector{Float64}, csv."N_pairs/s")
            append!(num_list, num)
            append!(perf_list, perf)
            num_read += 1
        catch err
            err isa InterruptException && rethrow()
            println(stderr, "note: ", file, " cannot be read (", reason(err), "); skipped")
        end
    end
    if num_read == 0
        return nothing
    end

    num = sort(unique(num_list))
    perf_max = Float64[]
    perf_med = Float64[]
    for N in num
        data = perf_list[num_list.==N]
        push!(perf_max, maximum(data))
        push!(perf_med, Statistics.median(data))
        push!(summary, (GPU, lang, flag, N, perf_max[end], perf_med[end], Statistics.mean(data), minimum(data), length(data)))
    end
    return Result(num, perf_max, perf_med)
end

# the largest num values of N
function sustained(res::Result, num::Integer)
    range = lastindex(res.N)-num+1:lastindex(res.N)
    return Result(res.N[range], res.max[range], res.med[range])
end

# values of N within [lower, upper]
function clip(res::Result, lower::Integer, upper::Integer)
    keep = lower .<= res.N .<= upper
    return Result(res.N[keep], res.max[keep], res.med[keep])
end

# ratio of the statistic (:max or :med) of res to the same statistic of ref, at the values of N in both
function ratio(res::Result, ref::Result, stat::Symbol)
    common = intersect(res.N, ref.N)
    at(N, list) = [findfirst(==(n), list) for n in N]
    return common, getfield(res, stat)[at(common, res.N)] ./ getfield(ref, stat)[at(common, ref.N)]
end

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
include("../util/pyplot.jl")

function plot_lp(fig, at, x, y; point_id=0, line_id=0, color_id=0, label="", open_symbol=false)
    if open_symbol
        at.plot(x, y, util_pyplot.call(fig.point, id=point_id), markersize=fig.ms, linestyle=util_pyplot.call(fig.line, id=line_id), linewidth=fig.lw, color=util_pyplot.call(fig.color, id=color_id), label=label, markerfacecolor="none")
    else
        at.plot(x, y, util_pyplot.call(fig.point, id=point_id), markersize=fig.ms, linestyle=util_pyplot.call(fig.line, id=line_id), linewidth=fig.lw, color=util_pyplot.call(fig.color, id=color_id), label=label)
    end
end

function set_xlabel(fig, at, label)
    at.set_xlabel(label, fontsize=fig.fs)
end
function set_ylabel(fig, at, label)
    at.set_ylabel(label, fontsize=fig.fs)
end

function set_ylim(at, bounds::Bounds; logPlt::Bool)
    if isfinite(bounds.lower) && isfinite(bounds.upper) && bounds.lower < bounds.upper
        at.set_ylim(util_pyplot.scale_axis(bounds.lower, bounds.upper, logPlt=logPlt))
    end
end

# models made by scaling.sh, in the order of drawing (the legend shows them in the reverse order)
const SERIES = [
    (model=6, lang="acc (parallel)", flag="base", point_id=2, line_id=2, color_id=2),
    (model=4, lang="acc (kernels)", flag="base", point_id=4, line_id=3, color_id=2),
    (model=2, lang="omp (distribute)", flag="base", point_id=0, line_id=2, color_id=1),
    (model=0, lang="omp (loop)", flag="base", point_id=1, line_id=3, color_id=1),
    (model=7, lang="acc (parallel)", flag="fast", point_id=2, line_id=2, color_id=2),
    (model=5, lang="acc (kernels)", flag="fast", point_id=4, line_id=3, color_id=2),
    (model=3, lang="omp (distribute)", flag="fast", point_id=0, line_id=2, color_id=1),
    (model=1, lang="omp (loop)", flag="fast", point_id=1, line_id=3, color_id=1),
]
const REFERENCE_STYLE = (point_id=3, line_id=1, color_id=0)

# figures: (scope, flag, stat); scope is :full (every N) or :sustained (the largest N)
const FIGURES = [(scope=scope, flag=flag, stat=stat) for scope in (:full, :sustained) for flag in ("base", "fast") for stat in (:max, :med)]
# the directory of the figures: not fig/, which the job scripts (sh/common/*.sh) use for the figures of a run
const FIGURE_DIR = "plot"
figure_name(key, suffix) = string(FIGURE_DIR, "/nbody", key.scope == :sustained ? "_sustained" : "", "_", key.flag, "_", key.stat, suffix)

# draws the figures of a key of FIGURES: one for each DIR and one with all DIRs (with the ratio to the reference when given)
# noted: the (DIR, model, scope) whose note on the ratio was printed (the figures of :max and :med share it)
function draw(key, dirs, captions, results, reference, reference_labels, N_upper::Integer, noted::Set; usetex::Bool, output_pdf::Bool)
    Ngpu = length(dirs)
    compare = any(!isnothing, reference)
    ny = compare ? 2 : 1
    sep = usetex ? "~" : " "
    select = key.scope == :sustained ? (res -> sustained(res, N_upper)) : identity
    comparison = util_pyplot.set_Panel(nx=Ngpu, ny=ny)
    perf_bounds = Bounds()
    ratio_bounds = Bounds()
    for (gpu_id, dir) in enumerate(dirs)
        single = util_pyplot.set_Panel()
        ref = isnothing(reference[gpu_id]) ? nothing : select(reference[gpu_id])
        for (fig, at) in [(single, single.ax[begin]), (comparison, comparison.ax[gpu_id, end])]
            if !isnothing(ref)
                plot_lp(fig, at, ref.N, getfield(ref, key.stat); REFERENCE_STYLE..., label=util_pyplot.plain_text(reference_labels[gpu_id], usetex=usetex), open_symbol=true)
            end
            for series in SERIES
                if series.flag == key.flag && haskey(results[gpu_id], series.model)
                    res = select(results[gpu_id][series.model])
                    plot_lp(fig, at, res.N, getfield(res, key.stat), point_id=series.point_id, line_id=series.line_id, color_id=series.color_id, label=series.lang)
                end
            end
        end
        if !isnothing(ref)
            extend!(perf_bounds, getfield(ref, key.stat))
        end
        for series in SERIES
            if series.flag == key.flag && haskey(results[gpu_id], series.model)
                res = select(results[gpu_id][series.model])
                extend!(perf_bounds, getfield(res, key.stat))
                if !isnothing(ref)
                    N, val = ratio(res, ref, key.stat)
                    if isempty(N)
                        if !((dir, series.model, key.scope) in noted)
                            push!(noted, (dir, series.model, key.scope))
                            println(stderr, "note: ", series.lang, " (", series.flag, ") in ", dir, " has no N in common with the reference", key.scope == :sustained ? string(" among the largest ", N_upper, " values of N") : "", "; its ratio is not drawn")
                        end
                        continue
                    end
                    plot_lp(comparison, comparison.ax[gpu_id, begin], N, val, point_id=series.point_id, line_id=series.line_id, color_id=series.color_id, label=series.lang)
                    extend!(ratio_bounds, val)
                end
            end
        end

        # figure of a DIR
        at = single.ax[begin]
        if key.scope == :full
            at.loglog()
        else
            at.semilogx()
            at.yaxis.set_major_formatter(PyPlot.matplotlib.ticker.FuncFormatter(util_pyplot.scientific))
        end
        at.grid()
        set_xlabel(single, at, L"$N$")
        set_ylabel(single, at, "Number of interactions per second")
        handles, labels = at.get_legend_handles_labels()
        at.legend(handles[end:-1:begin], labels[end:-1:begin], numpoints=1, handlelength=2.0, loc="best", fontsize=0.8 * single.fs)
        name = replace(dir, r"[/\\]" => "_")
        single.fig.savefig(string(figure_name(key, "_"), name, ".png"), format="png", dpi=100, bbox_inches="tight")
        if output_pdf
            single.fig.savefig(string(figure_name(key, "_"), name, ".pdf"), format="pdf", bbox_inches="tight")
        end
        PyPlot.close(single.fig)
    end

    # figure of all DIRs
    for at in comparison.ax[:, end]
        set_ylim(at, perf_bounds, logPlt=key.scope == :full)
        key.scope == :full ? at.loglog() : at.semilogx()
    end
    if compare
        for ii in 1:comparison.nx
            at = comparison.ax[ii, begin]
            set_ylim(at, ratio_bounds, logPlt=false)
            at.semilogx()
            # the panels of a DIR show the same range of N
            at.set_xlim(comparison.ax[ii, end].get_xlim())
        end
    end
    if key.scope == :sustained
        for ii in 1:comparison.nx
            for jj in 1:comparison.ny
                comparison.ax[ii, jj].tick_params(axis="x", which="minor", labelbottom=false)
            end
            comparison.ax[ii, begin].tick_params(axis="x", which="minor", labelbottom=true, labelsize=0.6 * comparison.fs, pad=0.3 * comparison.fs)
        end
        comparison.ax[begin, ny].yaxis.set_major_formatter(PyPlot.matplotlib.ticker.FuncFormatter(util_pyplot.scientific))
    end
    for at in comparison.ax
        at.grid()
    end
    set_ylabel(comparison, comparison.ax[begin, ny], "Number of interactions per second")
    if compare
        set_ylabel(comparison, comparison.ax[begin, begin], "Ratio to the reference")
    end
    for ii in 1:comparison.nx
        set_xlabel(comparison, comparison.ax[ii, begin], L"$N$")
        for jj in 1:comparison.ny
            maptag = jj == comparison.ny ? util_pyplot.plain_text(captions[ii], usetex=usetex) : ""
            id = (ii - 1) + comparison.nx * (comparison.ny - jj)
            at = comparison.ax[ii, jj]
            at.text(0.02, 0.98, string("(", Char(97 + id), ")", sep, maptag), color="black", fontsize=comparison.fs * 0.8, horizontalalignment="left", verticalalignment="top", transform=at.transAxes, bbox=Dict("facecolor" => "white", "edgecolor" => "None", "alpha" => 0.75))
        end
        at = comparison.ax[ii, end]
        handles, labels = at.get_legend_handles_labels()
        at.legend(handles[end:-1:begin], labels[end:-1:begin], numpoints=1, handlelength=2.0, bbox_to_anchor=(0.99, 0.67), loc="upper right", fontsize=comparison.fs * 0.75)
    end
    comparison.fig.savefig(figure_name(key, ".png"), format="png", dpi=100, bbox_inches="tight")
    if output_pdf
        comparison.fig.savefig(figure_name(key, ".pdf"), format="pdf", bbox_inches="tight")
    end
    PyPlot.close(comparison.fig)
end

function main()
    # read options
    argv = parse_cmd()
    output_pdf = argv["pdf"]
    usetex = argv["usetex"]
    N_upper = argv["num"]
    if N_upper < 1
        error(string("--num ", N_upper, ": give a positive number of data points"))
    end

    dirs = [rstrip(dir, '/') for dir in argv["dirs"]]
    if isempty(dirs)
        dirs = sort(filter(dir -> isdir(joinpath(dir, "scaling")), readdir()))
        if isempty(dirs)
            error(string("no directory made by scaling.sh (holding scaling/) in ", pwd(), ": give the directories as arguments, or run in the sample directory"))
        end
    end
    Ngpu = length(dirs)
    captions = [something(caption, dir) for (caption, dir) in zip(per_dir(argv["caption"], Ngpu, "--caption", nothing), dirs)]
    references = per_dir(argv["reference"], Ngpu, "--reference", nothing)
    compare = !isempty(argv["reference"])
    reference_labels = per_dir(argv["reference-label"], Ngpu, "--reference-label", "reference", allow_one=true)

    # read measured performance
    summary = DataFrame(GPU=[], lang=[], flag=[], N=[], perf_max=[], perf_med=[], perf_avg=[], perf_min=[], trials=[])
    results = Vector{Dict{Int,Result}}(undef, Ngpu)
    reference = Vector{Union{Result,Nothing}}(nothing, Ngpu)
    for (gpu_id, dir) in enumerate(dirs)
        if !isdir(dir)
            error(string("no directory ", dir, " (give the directories made by scaling.sh)"))
        end
        caption = captions[gpu_id]
        results[gpu_id] = Dict{Int,Result}()
        for series in SERIES
            res = read_results(glob(string("scaling/model", series.model, "/nbody/log/*.csv"), dir), summary, caption, series.lang, series.flag)
            if isnothing(res)
                println(stderr, "note: no CSV file that can be read in ", joinpath(dir, string("scaling/model", series.model, "/nbody/log/")), "; ", series.lang, " (", series.flag, ") is not drawn for ", dir)
                continue
            end
            if N_upper > length(res.N)
                error(string("--num ", N_upper, " exceeds the ", length(res.N), " values of N of ", series.lang, " (", series.flag, ") in ", dir, ": give --num from 1 to ", length(res.N)))
            end
            results[gpu_id][series.model] = res
        end
        if isempty(results[gpu_id])
            error(string("no result of scaling.sh in ", dir, ": looked for ", joinpath(dir, "scaling/model<0-7>/nbody/log/*.csv")))
        end
        if compare
            files = find_files(references[gpu_id])
            if isempty(files)
                error(string("no file matches --reference ", references[gpu_id], " (for ", dir, ")"))
            end
            res = read_results(files, summary, caption, reference_labels[gpu_id], "reference")
            if isnothing(res)
                error(string("no file of --reference ", references[gpu_id], " (for ", dir, ") can be read"))
            end
            # compare in the range of N of the results of Solomon
            res = clip(res, minimum(r.N[begin] for r in values(results[gpu_id])), maximum(r.N[end] for r in values(results[gpu_id])))
            if isempty(res.N)
                error(string("the files of --reference ", references[gpu_id], " have no N in the range of the results in ", dir))
            end
            if N_upper > length(res.N)
                error(string("--num ", N_upper, " exceeds the ", length(res.N), " values of N of the reference (", references[gpu_id], ") in the range of the results in ", dir, ": give --num from 1 to ", length(res.N)))
            end
            reference[gpu_id] = res
        end
    end

    # initialize matplotlib
    util_pyplot.config(usetex=usetex)
    if !ispath(FIGURE_DIR)
        mkdir(FIGURE_DIR)
    end
    noted = Set{Tuple{String,Int,Symbol}}()
    for key in FIGURES
        draw(key, dirs, captions, results, reference, reference_labels, N_upper, noted, usetex=usetex, output_pdf=output_pdf)
    end

    # output summary CSV file
    summary |> CSV.write(string("nbody", "_stats", ".csv"), delim=',', writeheader=true)
    return nothing
end


if abspath(PROGRAM_FILE) == @__FILE__
    main()
end
