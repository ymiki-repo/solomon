# Draws the performance measured by sh/<system>/iterate.sh at an optimization level; run in the sample directory:
#   julia --project=jl jl/plot/diffusion.jl [options] [DIR ...]
# DIR: a directory made by iterate.sh (<host>_<target>_<arch>, holding iterate/model<N>/diffusion_benchmark_*.csv);
#      without DIR, every directory in the current directory that holds iterate/ is drawn
# A panel for each DIR with the four ways of offloading (models 0 to 3, explicit data transfer);
# writes fig/diffusion_{max,med}.png and fig/diffusion_stats.csv.
using ArgParse
using CSV
using PyPlot
include("../util/pyplot.jl")
include("../util/results.jl")

# models in the order of drawing (the legend shows them in the reverse order)
const SERIES = [
    (model=3, point_id=2, line_id=2, color_id=2),
    (model=2, point_id=4, line_id=3, color_id=2),
    (model=1, point_id=0, line_id=2, color_id=1),
    (model=0, point_id=1, line_id=3, color_id=1),
]

function main()
    cfg = ArgParseSettings(description="Draw the performance measured by sh/<system>/iterate.sh at an optimization level (run in the sample directory)")
    util_results.add_common_args!(cfg, "iterate")
    util_results.add_opt_arg!(cfg)
    argv = parse_args(cfg)
    usetex = argv["usetex"]
    util_pyplot.config(usetex=usetex)

    dirs = util_results.result_dirs(argv, "iterate")
    caption = util_results.captions(argv, dirs)
    level = util_results.opt_levels(argv, length(dirs))
    Ngpu = length(dirs)
    summary = util_results.new_summary()

    # read measured performance
    data = [[] for _ in 1:Ngpu]
    for (gpu_id, dir) in enumerate(dirs)
        for series in SERIES
            lang = util_results.COLUMNS[begin+series.model]
            df = util_results.read_results(dir, "iterate", series.model)
            if isnothing(df)
                println(stderr, "note: no CSV file that can be read in ", joinpath(dir, "iterate", string("model", series.model)), "; ", lang, " is not drawn for ", dir)
                continue
            end
            res = util_results.split_results(df, level[gpu_id], summary, caption[gpu_id], lang, "explicit")
            if isnothing(res)
                println(stderr, "note: no result at optimization level ", level[gpu_id], " in ", joinpath(dir, "iterate", string("model", series.model)), " (found: ", join(unique(df.label), ", "), ")")
                continue
            end
            push!(data[gpu_id], (series, lang, res))
        end
        if isempty(data[gpu_id])
            error(string("nothing to draw for ", dir, " at optimization level ", level[gpu_id], ": looked for ", joinpath(dir, "iterate/model<0-3>/diffusion_benchmark_*.csv")))
        end
    end
    n3_all = unique(vcat([res.n3 for list in data for (_, _, res) in list]...))

    sep = usetex ? "~" : " "
    for stat in (:max, :med)
        fig = util_pyplot.set_Panel(nx=Ngpu)
        bounds = util_results.Bounds()
        for gpu_id in 1:Ngpu
            at = fig.ax[gpu_id]
            for (series, lang, res) in data[gpu_id]
                util_results.plot_lp(fig, at, res.n3, getfield(res, stat), point_id=series.point_id, line_id=series.line_id, color_id=series.color_id, label=lang)
                util_results.extend!(bounds, getfield(res, stat))
            end
        end
        for (gpu_id, at) in enumerate(fig.ax)
            util_results.set_ylim(at, bounds)
            util_results.set_n3_axis(fig, at, n3_all)
            at.set_xlabel(util_results.xlabel(), fontsize=fig.fs)
            util_results.panel_caption(fig, at, 0.03, 0.97, [string("(", Char(97 + gpu_id - 1), ")", sep, util_pyplot.plain_text(caption[gpu_id], usetex=usetex))], fig.fs * 0.9)
            handles, labels = at.get_legend_handles_labels()
            at.legend(handles[end:-1:begin], labels[end:-1:begin], numpoints=1, handlelength=2.0, loc="lower right", fontsize=fig.fs * 0.8)
        end
        fig.ax[begin].set_ylabel(util_results.ylabel(usetex), fontsize=fig.fs)
        util_results.save(fig, string("diffusion_", stat), argv["pdf"])
        PyPlot.close(fig.fig)
    end

    # output summary CSV file
    summary |> CSV.write(string("fig/", "diffusion", "_stats", ".csv"), delim=',', writeheader=true)
    return nothing
end


if abspath(PROGRAM_FILE) == @__FILE__
    main()
end
