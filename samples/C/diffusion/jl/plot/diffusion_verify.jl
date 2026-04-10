using ArgParse
function parse_cmd()
    cfg = ArgParseSettings()
    @add_arg_table cfg begin
        "--pdf", "-p"
        help = "generate figure in PDF format"
        action = :store_true
        # "--padding"
        # help = "number of padded elements in z-direction"
        # arg_type = Int
        # default = 0
    end
    return parse_args(cfg)
end

using DataFrames
using CSV
function read_csv(file::String)
    csv = CSV.read(file, DataFrame)
    return csv
end

function get_performance_log(file::String)
    # read observed monitor
    result = read_csv(file)

    opt = result."Optimization_level"
    nx = result."nx"
    ny = result."ny"
    nz = result."nz"
    perf = result."Flop/s"

    return opt, nx, ny, nz, perf
end

using DataFrames
function read_results(file_list)
    opt_list = []
    nx_list = []
    ny_list = []
    nz_list = []
    perf_list = []
    for file in file_list
        opt, nx, ny, nz, perf = get_performance_log(file)
        append!(opt_list, opt)
        append!(nx_list, nx)
        append!(ny_list, ny)
        append!(nz_list, nz)
        append!(perf_list, perf)
    end
    df = DataFrame(opt=opt_list, nx=nx_list, ny=ny_list, nz=nz_list, n3=nx_list .* ny_list .* nz_list, perf=perf_list * 1.0e-9)

    return df
end

using Statistics
function split_results(df, summary, GPU, lang, mode, opt)
    n3 = sort(unique(df.n3[df.opt.==opt]))
    perf_max = []
    perf_med = []
    for ii = eachindex(n3)
        data = df[df.n3.==n3[ii].&&df.opt.==opt, :]."perf"
        append!(perf_max, maximum(data))
        append!(perf_med, Statistics.median(data))
        push!(summary, (GPU, lang, mode, opt, n3[ii], perf_max[end], perf_med[end], Statistics.mean(data), minimum(data), length(data)))
    end

    return n3, perf_max, perf_med, summary
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

# function plot_lp_multi(fig, at, x1, y1, x2, y2, x3, y3, x4, y4; point_id=0, line_id=0, color_id=0, label="", open_symbol=false)
#     plot_lp(fig, at[1], x1, y1, point_id=point_id, line_id=line_id, color_id=color_id, label=label, open_symbol=open_symbol)
#     plot_lp(fig, at[2], x2, y2, point_id=point_id, line_id=line_id, color_id=color_id, label=label, open_symbol=open_symbol)
#     plot_lp(fig, at[3], x3, y3, point_id=point_id, line_id=line_id, color_id=color_id, label=label, open_symbol=open_symbol)
#     plot_lp(fig, at[4], x4, y4, point_id=point_id, line_id=line_id, color_id=color_id, label=label, open_symbol=open_symbol)
# end

function plot_series(file_list, summary, GPU, lang, mode, max_of_perf_max, min_of_perf_max, max_of_perf_med, min_of_perf_med, fig, ax)
    df = read_results(file_list)
    for arg in [(opt="default", point_id=0, line_id=0, color_id=0), (opt="O1", point_id=1, line_id=1, color_id=1), (opt="O2", point_id=2, line_id=2, color_id=2), (opt="O3", point_id=3, line_id=3, color_id=3), (opt="Ofast", point_id=4, line_id=4, color_id=4)]
        num, perf_max, perf_med, summary = split_results(df, summary, GPU, lang, mode, arg.opt)
        max_of_perf_max = max(max_of_perf_max, maximum(perf_max))
        max_of_perf_med = max(max_of_perf_med, maximum(perf_med))
        min_of_perf_max = min(min_of_perf_max, minimum(perf_max))
        min_of_perf_med = min(min_of_perf_med, minimum(perf_med))

        plot_lp(fig, ax, num, perf_max, point_id=arg.point_id, line_id=arg.line_id, color_id=arg.color_id, label=arg.opt)
    end

    return summary, max_of_perf_max, min_of_perf_max, max_of_perf_med, min_of_perf_med
end

function set_xlabel(fig, at, label)
    at.set_xlabel(label, fontsize=fig.fs)
end
function set_ylabel(fig, at, label)
    at.set_ylabel(label, fontsize=fig.fs)
end

using Parameters
@with_kw mutable struct gpu_config
    name::String
    root::String
    has_openacc::Bool = false
    caption::String
    has_managed::Bool = false
    has_unified::Bool = false
    set_first_touch::Bool = false
end

using DataFrames
using Statistics
using Glob
function main()
    # read options
    argv = parse_cmd()
    output_pdf = argv["pdf"]
    # padding = argv["padding"]

    # initialize matplotlib
    util_pyplot.config()

    Ngpu = 4
    gpu = Array{gpu_config,1}(undef, Ngpu)
    gpu[1] = gpu_config(name="mi210", root="mi210", caption="AMD Instinct MI210")
    gpu[2] = gpu_config(name="pvc", root="pvc", caption="Intel DC GPU Max 1100")
    gpu[3] = gpu_config(name="h100sxm", root="h100sxm", has_openacc=true, has_managed=true, caption="NVIDIA H100 SXM 80GB")
    gpu[4] = gpu_config(name="gh200", root="gh200", has_openacc=true, has_managed=true, has_unified=true, set_first_touch=true, caption="NVIDIA GH200 480GB")

    Ncomp = 4# OpenMP target (loop / distribute), OpenACC (kernels / parallel)
    compare_max = util_pyplot.set_Panel(nx=Ncomp, ny=Ngpu)

    # prepare summary CSV file
    summary = DataFrame(GPU=[], lang=[], mode=[], optimization=[], n3=[], perf_max=[], perf_med=[], perf_avg=[], perf_min=[], trials=[])

    for gpu_id in 1:Ngpu
        root = gpu[gpu_id].root
        name = gpu[gpu_id].name
        has_openacc = gpu[gpu_id].has_openacc
        # has_managed = gpu[gpu_id].has_managed
        # has_unified = gpu[gpu_id].has_unified
        # set_first_touch = gpu[gpu_id].set_first_touch

        max_of_perf_max::Float32 = 0.0
        min_of_perf_max = typemax(Float32)
        max_of_perf_med::Float32 = 0.0
        min_of_perf_med = typemax(Float32)

        summary, max_of_perf_max, min_of_perf_max, max_of_perf_med, min_of_perf_med = plot_series(glob(string(root, "/verify/model0/diffusion_benchmark_*.csv")), summary, gpu[gpu_id].caption, "omp (loop)", "data", max_of_perf_max, min_of_perf_max, max_of_perf_med, min_of_perf_med, compare_max, compare_max.ax[1, Ngpu-gpu_id+1])
        summary, max_of_perf_max, min_of_perf_max, max_of_perf_med, min_of_perf_med = plot_series(glob(string(root, "/verify/model1/diffusion_benchmark_*.csv")), summary, gpu[gpu_id].caption, "omp (distribute)", "data", max_of_perf_max, min_of_perf_max, max_of_perf_med, min_of_perf_med, compare_max, compare_max.ax[2, Ngpu-gpu_id+1])
        if has_openacc
            summary, max_of_perf_max, min_of_perf_max, max_of_perf_med, min_of_perf_med = plot_series(glob(string(root, "/verify/model2/diffusion_benchmark_*.csv")), summary, gpu[gpu_id].caption, "acc (kernels)", "data", max_of_perf_max, min_of_perf_max, max_of_perf_med, min_of_perf_med, compare_max, compare_max.ax[3, Ngpu-gpu_id+1])
            summary, max_of_perf_max, min_of_perf_max, max_of_perf_med, min_of_perf_med = plot_series(glob(string(root, "/verify/model3/diffusion_benchmark_*.csv")), summary, gpu[gpu_id].caption, "acc (parallel)", "data", max_of_perf_max, min_of_perf_max, max_of_perf_med, min_of_perf_med, compare_max, compare_max.ax[4, Ngpu-gpu_id+1])
        end

        for at in compare_max.ax[:, Ngpu-gpu_id+1]
            at.set_ylim(util_pyplot.scale_axis(min_of_perf_max, max_of_perf_max, logPlt=false))
        end
    end

    for fig in [compare_max]
        for at in fig.ax
            at.semilogx()
            at.set_xticks([32^3, 64^3, 128^3, 256^3, 512^3])
            at.grid()
        end
        for at in fig.ax[:, begin]
            at.set_xticklabels([L"$32^3$", L"$64^3$", L"$128^3$", L"$256^3$", L"$512^3$"])
            at.tick_params(axis="x", which="major", labelsize=0.9 * fig.fs)
            set_xlabel(fig, at, L"$N_x N_y N_z$")
        end
        for at in fig.ax[begin, :]
            set_ylabel(fig, at, L"\unit{GFlop/s}")
        end

        for ii in 1:fig.nx
            for jj in 1:fig.ny
                if ii == 1
                    maptag = string(gpu[Ngpu-jj+1].caption, ", omp (loop)")
                end
                if ii == 2
                    maptag = string(gpu[Ngpu-jj+1].caption, ", omp (distribute)")
                end
                if ii == 3
                    maptag = string(gpu[Ngpu-jj+1].caption, ", acc (kernels)")
                end
                if ii == 4
                    maptag = string(gpu[Ngpu-jj+1].caption, ", acc (parallel)")
                end
                id = (ii - 1) + fig.nx * (fig.ny - jj)
                if jj == 3
                    id -= 2
                end
                if jj <= 2
                    id -= 4
                end
                caption = string("(", Char(97 + id), ")")
                at = fig.ax[ii, jj]
                at.text(0.02, 0.98, string(caption, "~", maptag), color="black", fontsize=fig.fs * 0.8, horizontalalignment="left", verticalalignment="top", transform=at.transAxes, bbox=Dict("facecolor" => "white", "edgecolor" => "None", "alpha" => 0.75))
            end
            for ii in 3:4
                for jj in 3:4
                    fig.ax[ii, jj].set_visible(false)
                end
            end
        end

        # set label
        at = fig.ax[begin, end]
        handles, labels = at.get_legend_handles_labels()
        at.legend(handles[end:-1:begin], labels[end:-1:begin], numpoints=1, handlelength=2.0, loc="lower right", fontsize=fig.fs * 0.8)
        handles = nothing
        labels = nothing
    end

    # save figures
    if !ispath("fig")
        mkdir("fig")
    end
    compare_max.fig.savefig(string("fig/", "diffusion", "_verify", "_opt", ".png"), format="png", dpi=100, bbox_inches="tight")
    if output_pdf
        compare_max.fig.savefig(string("fig/", "diffusion", "_verify", "_opt", ".pdf"), format="pdf", bbox_inches="tight")
    end

    PyPlot.close("all")
    return nothing
end


main()
