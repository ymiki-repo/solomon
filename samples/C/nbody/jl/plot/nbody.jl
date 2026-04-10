using ArgParse
function parse_cmd()
    cfg = ArgParseSettings()
    @add_arg_table cfg begin
        "--pdf", "-p"
        help = "generate figure in PDF format"
        action = :store_true
        "--compare", "-c"
        help = "compare with the fastest result (CUDA/HIP/SYCL)"
        action = :store_true
        "--num", "-n"
        help = "number of data points to show the sustained performance"
        arg_type = Int
        default = 4
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

    num = result."N"
    perf = result."N_pairs/s"

    return num, perf
end

using DataFrames
using Statistics
function read_results(file_list, summary, GPU, lang, flag)
    num_list = []
    perf_list = []
    for file in file_list
        num, perf = get_performance_log(file)
        append!(num_list, num)
        append!(perf_list, perf)
    end
    df = DataFrame(num=num_list, perf=perf_list)

    num = sort(unique(df.num))
    perf_max = []
    perf_med = []
    for ii = eachindex(num)
        data = df[df.num.==num[ii], :]."perf"
        append!(perf_max, maximum(data))
        append!(perf_med, Statistics.median(data))
        push!(summary, (GPU, lang, flag, num[ii], perf_max[end], perf_med[end], Statistics.mean(data), minimum(data), length(data)))
    end

    return num, perf_max, perf_med, summary
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

using Parameters
@with_kw mutable struct gpu_config
    name::String
    root::String
    has_openacc::Bool = false
    caption::String
    num_cores::Float32
    best_env_label::String
    fastest_mode::String = "ftz"
end

using DataFrames
using Statistics
using Glob
function main()
    # read options
    argv = parse_cmd()
    output_pdf = argv["pdf"]
    compare = argv["compare"]
    N_upper = argv["num"]

    # initialize matplotlib
    util_pyplot.config()

    Ngpu = 4
    # Ngpu = 3
    gpu = Array{gpu_config,1}(undef, Ngpu)
    gpu[1] = gpu_config(name="h100sxm", root="mercury/h100sxm", has_openacc=true, caption="NVIDIA H100 SXM 80GB", num_cores=128 * 132, best_env_label="SYCL (icpx)")
    gpu[2] = gpu_config(name="gh200", root="mercury/gh200", has_openacc=true, caption="NVIDIA GH200 480GB", num_cores=128 * 132, best_env_label="CUDA")
    gpu[3] = gpu_config(name="mi210", root="milan2/mi210", caption="AMD Instinct MI210", num_cores=64 * 104, best_env_label="SYCL (acpp, packed FP32)", fastest_mode="default")
    gpu[4] = gpu_config(name="pvc", root="spr2/pvc", caption="Intel Data Center GPU Max 1100", num_cores=16 * 448, best_env_label="SYCL (icpx)", fastest_mode="default")

    ny = compare ? 2 : 1

    compare_base_max = util_pyplot.set_Panel(nx=Ngpu, ny=ny)
    compare_base_med = util_pyplot.set_Panel(nx=Ngpu, ny=ny)
    compare_fast_max = util_pyplot.set_Panel(nx=Ngpu, ny=ny)
    compare_fast_med = util_pyplot.set_Panel(nx=Ngpu, ny=ny)
    compare_sustained_base_max = util_pyplot.set_Panel(nx=Ngpu, ny=ny)
    compare_sustained_base_med = util_pyplot.set_Panel(nx=Ngpu, ny=ny)
    compare_sustained_fast_max = util_pyplot.set_Panel(nx=Ngpu, ny=ny)
    compare_sustained_fast_med = util_pyplot.set_Panel(nx=Ngpu, ny=ny)

    max_of_perf_base_max::Float32 = 0.0
    max_of_perf_base_med::Float32 = 0.0
    max_of_perf_fast_max::Float32 = 0.0
    max_of_perf_fast_med::Float32 = 0.0
    min_of_perf_base_max = typemax(Float32)
    min_of_perf_base_med = typemax(Float32)
    min_of_perf_fast_max = typemax(Float32)
    min_of_perf_fast_med = typemax(Float32)
    max_of_sustained_perf_base_max::Float32 = 0.0
    max_of_sustained_perf_base_med::Float32 = 0.0
    max_of_sustained_perf_fast_max::Float32 = 0.0
    max_of_sustained_perf_fast_med::Float32 = 0.0
    min_of_sustained_perf_base_max = typemax(Float32)
    min_of_sustained_perf_base_med = typemax(Float32)
    min_of_sustained_perf_fast_max = typemax(Float32)
    min_of_sustained_perf_fast_med = typemax(Float32)

    if compare
        max_of_perf_base_max_speedup::Float32 = 0.0
        max_of_perf_base_med_speedup::Float32 = 0.0
        max_of_perf_fast_max_speedup::Float32 = 0.0
        max_of_perf_fast_med_speedup::Float32 = 0.0
        min_of_perf_base_max_speedup = typemax(Float32)
        min_of_perf_base_med_speedup = typemax(Float32)
        min_of_perf_fast_max_speedup = typemax(Float32)
        min_of_perf_fast_med_speedup = typemax(Float32)
        max_of_sustained_perf_base_max_speedup::Float32 = 0.0
        max_of_sustained_perf_base_med_speedup::Float32 = 0.0
        max_of_sustained_perf_fast_max_speedup::Float32 = 0.0
        max_of_sustained_perf_fast_med_speedup::Float32 = 0.0
        min_of_sustained_perf_base_max_speedup = typemax(Float32)
        min_of_sustained_perf_base_med_speedup = typemax(Float32)
        min_of_sustained_perf_fast_max_speedup = typemax(Float32)
        min_of_sustained_perf_fast_med_speedup = typemax(Float32)
    end

    # prepare summary CSV file
    summary = DataFrame(GPU=[], lang=[], flag=[], N=[], perf_max=[], perf_med=[], perf_avg=[], perf_min=[], trials=[])

    for gpu_id in 1:Ngpu
        root = gpu[gpu_id].root
        has_openacc = gpu[gpu_id].has_openacc
        name = gpu[gpu_id].name
        fastest_mode = gpu[gpu_id].fastest_mode

        # read measured performance
        N_base_omp_loop, perf_base_max_omp_loop, perf_base_med_omp_loop, summary = read_results(glob(string(root, "/scaling/model0/nbody/log/*.csv")), summary, gpu[gpu_id].caption, "omp (loop)", "base")
        N_fast_omp_loop, perf_fast_max_omp_loop, perf_fast_med_omp_loop, summary = read_results(glob(string(root, "/scaling/model1/nbody/log/*.csv")), summary, gpu[gpu_id].caption, "omp (loop)", "fast")
        N_base_omp_dist, perf_base_max_omp_dist, perf_base_med_omp_dist, summary = read_results(glob(string(root, "/scaling/model2/nbody/log/*.csv")), summary, gpu[gpu_id].caption, "omp (distribute)", "base")
        N_fast_omp_dist, perf_fast_max_omp_dist, perf_fast_med_omp_dist, summary = read_results(glob(string(root, "/scaling/model3/nbody/log/*.csv")), summary, gpu[gpu_id].caption, "omp (distribute)", "fast")
        max_of_perf_base_max = max(max_of_perf_base_max, maximum(perf_base_max_omp_dist), maximum(perf_base_max_omp_loop))
        max_of_perf_base_med = max(max_of_perf_base_med, maximum(perf_base_med_omp_dist), maximum(perf_base_med_omp_loop))
        min_of_perf_base_max = min(min_of_perf_base_max, minimum(perf_base_max_omp_dist), minimum(perf_base_max_omp_loop))
        min_of_perf_base_med = min(min_of_perf_base_med, minimum(perf_base_med_omp_dist), minimum(perf_base_med_omp_loop))
        max_of_perf_fast_max = max(max_of_perf_fast_max, maximum(perf_fast_max_omp_dist), maximum(perf_fast_max_omp_loop))
        max_of_perf_fast_med = max(max_of_perf_fast_med, maximum(perf_fast_med_omp_dist), maximum(perf_fast_med_omp_loop))
        min_of_perf_fast_max = min(min_of_perf_fast_max, minimum(perf_fast_max_omp_dist), minimum(perf_fast_max_omp_loop))
        min_of_perf_fast_med = min(min_of_perf_fast_med, minimum(perf_fast_med_omp_dist), minimum(perf_fast_med_omp_loop))
        # prepare arrays for sustained performance
        N_sustained_base_omp_loop = N_base_omp_loop[end-N_upper+1:end]
        N_sustained_fast_omp_loop = N_fast_omp_loop[end-N_upper+1:end]
        N_sustained_base_omp_dist = N_base_omp_dist[end-N_upper+1:end]
        N_sustained_fast_omp_dist = N_fast_omp_dist[end-N_upper+1:end]
        sustained_perf_base_max_omp_loop = perf_base_max_omp_loop[end-N_upper+1:end]
        sustained_perf_fast_max_omp_loop = perf_fast_max_omp_loop[end-N_upper+1:end]
        sustained_perf_base_max_omp_dist = perf_base_max_omp_dist[end-N_upper+1:end]
        sustained_perf_fast_max_omp_dist = perf_fast_max_omp_dist[end-N_upper+1:end]
        sustained_perf_base_med_omp_loop = perf_base_med_omp_loop[end-N_upper+1:end]
        sustained_perf_fast_med_omp_loop = perf_fast_med_omp_loop[end-N_upper+1:end]
        sustained_perf_base_med_omp_dist = perf_base_med_omp_dist[end-N_upper+1:end]
        sustained_perf_fast_med_omp_dist = perf_fast_med_omp_dist[end-N_upper+1:end]
        max_of_sustained_perf_base_max = max(max_of_sustained_perf_base_max, maximum(sustained_perf_base_max_omp_dist), maximum(sustained_perf_base_max_omp_loop))
        max_of_sustained_perf_base_med = max(max_of_sustained_perf_base_med, maximum(sustained_perf_base_med_omp_dist), maximum(sustained_perf_base_med_omp_loop))
        min_of_sustained_perf_base_max = min(min_of_sustained_perf_base_max, minimum(sustained_perf_base_max_omp_dist), minimum(sustained_perf_base_max_omp_loop))
        min_of_sustained_perf_base_med = min(min_of_sustained_perf_base_med, minimum(sustained_perf_base_med_omp_dist), minimum(sustained_perf_base_med_omp_loop))
        max_of_sustained_perf_fast_max = max(max_of_sustained_perf_fast_max, maximum(sustained_perf_fast_max_omp_dist), maximum(sustained_perf_fast_max_omp_loop))
        max_of_sustained_perf_fast_med = max(max_of_sustained_perf_fast_med, maximum(sustained_perf_fast_med_omp_dist), maximum(sustained_perf_fast_med_omp_loop))
        min_of_sustained_perf_fast_max = min(min_of_sustained_perf_fast_max, minimum(sustained_perf_fast_max_omp_dist), minimum(sustained_perf_fast_max_omp_loop))
        min_of_sustained_perf_fast_med = min(min_of_sustained_perf_fast_med, minimum(sustained_perf_fast_med_omp_dist), minimum(sustained_perf_fast_med_omp_loop))
        if has_openacc
            N_base_acc_ker, perf_base_max_acc_ker, perf_base_med_acc_ker, summary = read_results(glob(string(root, "/scaling/model4/nbody/log/*.csv")), summary, gpu[gpu_id].caption, "acc (kernels)", "base")
            N_fast_acc_ker, perf_fast_max_acc_ker, perf_fast_med_acc_ker, summary = read_results(glob(string(root, "/scaling/model5/nbody/log/*.csv")), summary, gpu[gpu_id].caption, "acc (kernels)", "fast")
            N_base_acc_par, perf_base_max_acc_par, perf_base_med_acc_par, summary = read_results(glob(string(root, "/scaling/model6/nbody/log/*.csv")), summary, gpu[gpu_id].caption, "acc (parallel)", "base")
            N_fast_acc_par, perf_fast_max_acc_par, perf_fast_med_acc_par, summary = read_results(glob(string(root, "/scaling/model7/nbody/log/*.csv")), summary, gpu[gpu_id].caption, "acc (parallel)", "fast")
            max_of_perf_base_max = max(max_of_perf_base_max, maximum(perf_base_max_acc_ker), maximum(perf_base_max_acc_par))
            max_of_perf_base_med = max(max_of_perf_base_med, maximum(perf_base_med_acc_ker), maximum(perf_base_med_acc_par))
            min_of_perf_base_max = min(min_of_perf_base_max, minimum(perf_base_max_acc_ker), minimum(perf_base_max_acc_par))
            min_of_perf_base_med = min(min_of_perf_base_med, minimum(perf_base_med_acc_ker), minimum(perf_base_med_acc_par))
            max_of_perf_fast_max = max(max_of_perf_fast_max, maximum(perf_fast_max_acc_ker), maximum(perf_fast_max_acc_par))
            max_of_perf_fast_med = max(max_of_perf_fast_med, maximum(perf_fast_med_acc_ker), maximum(perf_fast_med_acc_par))
            min_of_perf_fast_max = min(min_of_perf_fast_max, minimum(perf_fast_max_acc_ker), minimum(perf_fast_max_acc_par))
            min_of_perf_fast_med = min(min_of_perf_fast_med, minimum(perf_fast_med_acc_ker), minimum(perf_fast_med_acc_par))
            # prepare arrays for sustained performance
            N_sustained_base_acc_ker = N_base_acc_ker[end-N_upper+1:end]
            N_sustained_fast_acc_ker = N_fast_acc_ker[end-N_upper+1:end]
            N_sustained_base_acc_par = N_base_acc_par[end-N_upper+1:end]
            N_sustained_fast_acc_par = N_fast_acc_par[end-N_upper+1:end]
            sustained_perf_base_max_acc_ker = perf_base_max_acc_ker[end-N_upper+1:end]
            sustained_perf_fast_max_acc_ker = perf_fast_max_acc_ker[end-N_upper+1:end]
            sustained_perf_base_max_acc_par = perf_base_max_acc_par[end-N_upper+1:end]
            sustained_perf_fast_max_acc_par = perf_fast_max_acc_par[end-N_upper+1:end]
            sustained_perf_base_med_acc_ker = perf_base_med_acc_ker[end-N_upper+1:end]
            sustained_perf_fast_med_acc_ker = perf_fast_med_acc_ker[end-N_upper+1:end]
            sustained_perf_base_med_acc_par = perf_base_med_acc_par[end-N_upper+1:end]
            sustained_perf_fast_med_acc_par = perf_fast_med_acc_par[end-N_upper+1:end]
            max_of_sustained_perf_base_max = max(max_of_sustained_perf_base_max, maximum(sustained_perf_base_max_acc_ker), maximum(sustained_perf_base_max_acc_par))
            max_of_sustained_perf_base_med = max(max_of_sustained_perf_base_med, maximum(sustained_perf_base_med_acc_ker), maximum(sustained_perf_base_med_acc_par))
            min_of_sustained_perf_base_max = min(min_of_sustained_perf_base_max, minimum(sustained_perf_base_max_acc_ker), minimum(sustained_perf_base_max_acc_par))
            min_of_sustained_perf_base_med = min(min_of_sustained_perf_base_med, minimum(sustained_perf_base_med_acc_ker), minimum(sustained_perf_base_med_acc_par))
            max_of_sustained_perf_fast_max = max(max_of_sustained_perf_fast_max, maximum(sustained_perf_fast_max_acc_ker), maximum(sustained_perf_fast_max_acc_par))
            max_of_sustained_perf_fast_med = max(max_of_sustained_perf_fast_med, maximum(sustained_perf_fast_med_acc_ker), maximum(sustained_perf_fast_med_acc_par))
            min_of_sustained_perf_fast_max = min(min_of_sustained_perf_fast_max, minimum(sustained_perf_fast_max_acc_ker), minimum(sustained_perf_fast_max_acc_par))
            min_of_sustained_perf_fast_med = min(min_of_sustained_perf_fast_med, minimum(sustained_perf_fast_med_acc_ker), minimum(sustained_perf_fast_med_acc_par))
        end
        if compare
            N_best, perf_max_best, perf_med_best, summary = read_results(glob(string(root, "/fastest/scaling/", fastest_mode, "/log/nbody_benchmark_*.csv")), summary, gpu[gpu_id].caption, gpu[gpu_id].best_env_label, "best")
            num_cut_lower = Integer(log2(minimum(N_best) / minimum(N_base_omp_dist)))
            if num_cut_lower > 0
                deleteat!(N_best, 1:num_cut_lower)
                deleteat!(perf_max_best, 1:num_cut_lower)
                deleteat!(perf_med_best, 1:num_cut_lower)
            end
            num_cut_upper = Integer(log2(maximum(N_best) / maximum(N_base_omp_dist)))
            if num_cut_upper > 0
                N_all = length(N_best)
                N_head = N_all - num_cut_upper + 1
                deleteat!(N_best, N_head:N_all)
                deleteat!(perf_max_best, N_head:N_all)
                deleteat!(perf_med_best, N_head:N_all)
            end
            max_of_perf_base_max = max(max_of_perf_base_max, maximum(perf_max_best))
            max_of_perf_base_med = max(max_of_perf_base_med, maximum(perf_med_best))
            min_of_perf_base_max = min(min_of_perf_base_max, minimum(perf_max_best))
            min_of_perf_base_med = min(min_of_perf_base_med, minimum(perf_med_best))
            max_of_perf_fast_max = max(max_of_perf_fast_max, maximum(perf_max_best))
            max_of_perf_fast_med = max(max_of_perf_fast_med, maximum(perf_med_best))
            min_of_perf_fast_max = min(min_of_perf_fast_max, minimum(perf_max_best))
            min_of_perf_fast_med = min(min_of_perf_fast_med, minimum(perf_med_best))
            max_of_perf_base_max_speedup = max(max_of_perf_base_max_speedup, maximum(perf_base_max_omp_dist ./ perf_max_best), maximum(perf_base_max_omp_loop ./ perf_max_best))
            max_of_perf_base_med_speedup = max(max_of_perf_base_med_speedup, maximum(perf_base_med_omp_dist ./ perf_med_best), maximum(perf_base_med_omp_loop ./ perf_med_best))
            max_of_perf_fast_max_speedup = max(max_of_perf_fast_max_speedup, maximum(perf_fast_max_omp_dist ./ perf_max_best), maximum(perf_fast_max_omp_loop ./ perf_max_best))
            max_of_perf_fast_med_speedup = max(max_of_perf_fast_med_speedup, maximum(perf_fast_med_omp_dist ./ perf_med_best), maximum(perf_fast_med_omp_loop ./ perf_med_best))
            min_of_perf_base_max_speedup = min(min_of_perf_base_max_speedup, minimum(perf_base_max_omp_dist ./ perf_max_best), minimum(perf_base_max_omp_loop ./ perf_max_best))
            min_of_perf_base_med_speedup = min(min_of_perf_base_med_speedup, minimum(perf_base_med_omp_dist ./ perf_med_best), minimum(perf_base_med_omp_loop ./ perf_med_best))
            min_of_perf_fast_max_speedup = min(min_of_perf_fast_max_speedup, minimum(perf_fast_max_omp_dist ./ perf_max_best), minimum(perf_fast_max_omp_loop ./ perf_max_best))
            min_of_perf_fast_med_speedup = min(min_of_perf_fast_med_speedup, minimum(perf_fast_med_omp_dist ./ perf_med_best), minimum(perf_fast_med_omp_loop ./ perf_med_best))

            # prepare arrays for sustained performance
            N_sustained_best = N_best[end-N_upper+1:end]
            sustained_perf_max_best = perf_max_best[end-N_upper+1:end]
            sustained_perf_med_best = perf_med_best[end-N_upper+1:end]
            max_of_sustained_perf_base_max = max(max_of_sustained_perf_base_max, maximum(sustained_perf_max_best))
            max_of_sustained_perf_base_med = max(max_of_sustained_perf_base_med, maximum(sustained_perf_med_best))
            min_of_sustained_perf_base_max = min(min_of_sustained_perf_base_max, minimum(sustained_perf_max_best))
            min_of_sustained_perf_base_med = min(min_of_sustained_perf_base_med, minimum(sustained_perf_med_best))
            max_of_sustained_perf_fast_max = max(max_of_sustained_perf_fast_max, maximum(sustained_perf_max_best))
            max_of_sustained_perf_fast_med = max(max_of_sustained_perf_fast_med, maximum(sustained_perf_med_best))
            min_of_sustained_perf_fast_max = min(min_of_sustained_perf_fast_max, minimum(sustained_perf_max_best))
            min_of_sustained_perf_fast_med = min(min_of_sustained_perf_fast_med, minimum(sustained_perf_med_best))
            max_of_sustained_perf_base_max_speedup = max(max_of_sustained_perf_base_max_speedup, maximum(sustained_perf_base_max_omp_dist ./ sustained_perf_max_best), maximum(sustained_perf_base_max_omp_loop ./ sustained_perf_max_best))
            max_of_sustained_perf_base_med_speedup = max(max_of_sustained_perf_base_med_speedup, maximum(sustained_perf_base_med_omp_dist ./ sustained_perf_med_best), maximum(sustained_perf_base_med_omp_loop ./ sustained_perf_med_best))
            max_of_sustained_perf_fast_max_speedup = max(max_of_sustained_perf_fast_max_speedup, maximum(sustained_perf_fast_max_omp_dist ./ sustained_perf_max_best), maximum(sustained_perf_fast_max_omp_loop ./ sustained_perf_max_best))
            max_of_sustained_perf_fast_med_speedup = max(max_of_sustained_perf_fast_med_speedup, maximum(sustained_perf_fast_med_omp_dist ./ sustained_perf_med_best), maximum(sustained_perf_fast_med_omp_loop ./ sustained_perf_med_best))
            min_of_sustained_perf_base_max_speedup = min(min_of_sustained_perf_base_max_speedup, minimum(sustained_perf_base_max_omp_dist ./ sustained_perf_max_best), minimum(sustained_perf_base_max_omp_loop ./ sustained_perf_max_best))
            min_of_sustained_perf_base_med_speedup = min(min_of_sustained_perf_base_med_speedup, minimum(sustained_perf_base_med_omp_dist ./ sustained_perf_med_best), minimum(sustained_perf_base_med_omp_loop ./ sustained_perf_med_best))
            min_of_sustained_perf_fast_max_speedup = min(min_of_sustained_perf_fast_max_speedup, minimum(sustained_perf_fast_max_omp_dist ./ sustained_perf_max_best), minimum(sustained_perf_fast_max_omp_loop ./ sustained_perf_max_best))
            min_of_sustained_perf_fast_med_speedup = min(min_of_sustained_perf_fast_med_speedup, minimum(sustained_perf_fast_med_omp_dist ./ sustained_perf_med_best), minimum(sustained_perf_fast_med_omp_loop ./ sustained_perf_med_best))

            if has_openacc
                max_of_perf_base_max_speedup = max(max_of_perf_base_max_speedup, maximum(perf_base_max_acc_ker ./ perf_max_best), maximum(perf_base_max_acc_par ./ perf_max_best))
                max_of_perf_base_med_speedup = max(max_of_perf_base_med_speedup, maximum(perf_base_med_acc_ker ./ perf_med_best), maximum(perf_base_med_acc_par ./ perf_med_best))
                max_of_perf_fast_max_speedup = max(max_of_perf_fast_max_speedup, maximum(perf_fast_max_acc_ker ./ perf_max_best), maximum(perf_fast_max_acc_par ./ perf_max_best))
                max_of_perf_fast_med_speedup = max(max_of_perf_fast_med_speedup, maximum(perf_fast_med_acc_ker ./ perf_med_best), maximum(perf_fast_med_acc_par ./ perf_med_best))
                min_of_perf_base_max_speedup = min(min_of_perf_base_max_speedup, minimum(perf_base_max_acc_ker ./ perf_max_best), minimum(perf_base_max_acc_par ./ perf_max_best))
                min_of_perf_base_med_speedup = min(min_of_perf_base_med_speedup, minimum(perf_base_med_acc_ker ./ perf_med_best), minimum(perf_base_med_acc_par ./ perf_med_best))
                min_of_perf_fast_max_speedup = min(min_of_perf_fast_max_speedup, minimum(perf_fast_max_acc_ker ./ perf_max_best), minimum(perf_fast_max_acc_par ./ perf_max_best))
                min_of_perf_fast_med_speedup = min(min_of_perf_fast_med_speedup, minimum(perf_fast_med_acc_ker ./ perf_med_best), minimum(perf_fast_med_acc_par ./ perf_med_best))

                max_of_sustained_perf_base_max_speedup = max(max_of_sustained_perf_base_max_speedup, maximum(sustained_perf_base_max_acc_ker ./ sustained_perf_max_best), maximum(sustained_perf_base_max_acc_par ./ sustained_perf_max_best))
                max_of_sustained_perf_base_med_speedup = max(max_of_sustained_perf_base_med_speedup, maximum(sustained_perf_base_med_acc_ker ./ sustained_perf_med_best), maximum(sustained_perf_base_med_acc_par ./ sustained_perf_med_best))
                max_of_sustained_perf_fast_max_speedup = max(max_of_sustained_perf_fast_max_speedup, maximum(sustained_perf_fast_max_acc_ker ./ sustained_perf_max_best), maximum(sustained_perf_fast_max_acc_par ./ sustained_perf_max_best))
                max_of_sustained_perf_fast_med_speedup = max(max_of_sustained_perf_fast_med_speedup, maximum(sustained_perf_fast_med_acc_ker ./ sustained_perf_med_best), maximum(sustained_perf_fast_med_acc_par ./ sustained_perf_med_best))
                min_of_sustained_perf_base_max_speedup = min(min_of_sustained_perf_base_max_speedup, minimum(sustained_perf_base_max_acc_ker ./ sustained_perf_max_best), minimum(sustained_perf_base_max_acc_par ./ sustained_perf_max_best))
                min_of_sustained_perf_base_med_speedup = min(min_of_sustained_perf_base_med_speedup, minimum(sustained_perf_base_med_acc_ker ./ sustained_perf_med_best), minimum(sustained_perf_base_med_acc_par ./ sustained_perf_med_best))
                min_of_sustained_perf_fast_max_speedup = min(min_of_sustained_perf_fast_max_speedup, minimum(sustained_perf_fast_max_acc_ker ./ sustained_perf_max_best), minimum(sustained_perf_fast_max_acc_par ./ sustained_perf_max_best))
                min_of_sustained_perf_fast_med_speedup = min(min_of_sustained_perf_fast_med_speedup, minimum(sustained_perf_fast_med_acc_ker ./ sustained_perf_med_best), minimum(sustained_perf_fast_med_acc_par ./ sustained_perf_med_best))
            end
        end

        perf_base_max = util_pyplot.set_Panel()
        perf_base_med = util_pyplot.set_Panel()
        perf_fast_max = util_pyplot.set_Panel()
        perf_fast_med = util_pyplot.set_Panel()
        sustained_perf_base_max = util_pyplot.set_Panel()
        sustained_perf_base_med = util_pyplot.set_Panel()
        sustained_perf_fast_max = util_pyplot.set_Panel()
        sustained_perf_fast_med = util_pyplot.set_Panel()

        # best performance (w/o optimization flag: full scale | specific GPU, GPU comparison)
        for target in [(fig=perf_base_max, ax=perf_base_max.ax[begin]), (fig=compare_base_max, ax=compare_base_max.ax[gpu_id, end])]
            if compare
                plot_lp(target.fig, target.ax, N_best, perf_max_best, point_id=3, line_id=1, color_id=0, label=gpu[gpu_id].best_env_label, open_symbol=true)
            end
            if has_openacc
                plot_lp(target.fig, target.ax, N_base_acc_par, perf_base_max_acc_par, point_id=2, line_id=2, color_id=2, label="acc (parallel)")
                plot_lp(target.fig, target.ax, N_base_acc_ker, perf_base_max_acc_ker, point_id=4, line_id=3, color_id=2, label="acc (kernels)")
            end
            plot_lp(target.fig, target.ax, N_base_omp_dist, perf_base_max_omp_dist, point_id=0, line_id=2, color_id=1, label="omp (distribute)")
            plot_lp(target.fig, target.ax, N_base_omp_loop, perf_base_max_omp_loop, point_id=1, line_id=3, color_id=1, label="omp (loop)")
        end
        # best performance (w/o optimization flag: sustained performance | specific GPU, GPU comparison)
        for target in [(fig=sustained_perf_base_max, ax=sustained_perf_base_max.ax[begin]), (fig=compare_sustained_base_max, ax=compare_sustained_base_max.ax[gpu_id, end])]
            if compare
                plot_lp(target.fig, target.ax, N_sustained_best, sustained_perf_max_best, point_id=3, line_id=1, color_id=0, label=gpu[gpu_id].best_env_label, open_symbol=true)
            end
            if has_openacc
                plot_lp(target.fig, target.ax, N_sustained_base_acc_par, sustained_perf_base_max_acc_par, point_id=2, line_id=2, color_id=2, label="acc (parallel)")
                plot_lp(target.fig, target.ax, N_sustained_base_acc_ker, sustained_perf_base_max_acc_ker, point_id=4, line_id=3, color_id=2, label="acc (kernels)")
            end
            plot_lp(target.fig, target.ax, N_sustained_base_omp_dist, sustained_perf_base_max_omp_dist, point_id=0, line_id=2, color_id=1, label="omp (distribute)")
            plot_lp(target.fig, target.ax, N_sustained_base_omp_loop, sustained_perf_base_max_omp_loop, point_id=1, line_id=3, color_id=1, label="omp (loop)")
        end
        # best performance (w/ optimization flag: full scale | specific GPU, GPU comparison)
        for target in [(fig=perf_fast_max, ax=perf_fast_max.ax[begin]), (fig=compare_fast_max, ax=compare_fast_max.ax[gpu_id, end])]
            if compare
                plot_lp(target.fig, target.ax, N_best, perf_max_best, point_id=3, line_id=1, color_id=0, label=gpu[gpu_id].best_env_label, open_symbol=true)
            end
            if has_openacc
                plot_lp(target.fig, target.ax, N_fast_acc_par, perf_fast_max_acc_par, point_id=2, line_id=2, color_id=2, label="acc (parallel)")
                plot_lp(target.fig, target.ax, N_fast_acc_ker, perf_fast_max_acc_ker, point_id=4, line_id=3, color_id=2, label="acc (kernels)")
            end
            plot_lp(target.fig, target.ax, N_fast_omp_dist, perf_fast_max_omp_dist, point_id=0, line_id=2, color_id=1, label="omp (distribute)")
            plot_lp(target.fig, target.ax, N_fast_omp_loop, perf_fast_max_omp_loop, point_id=1, line_id=3, color_id=1, label="omp (loop)")
        end
        # best performance (w/ optimization flag: sustained performance | specific GPU, GPU comparison)
        for target in [(fig=sustained_perf_fast_max, ax=sustained_perf_fast_max.ax[begin]), (fig=compare_sustained_fast_max, ax=compare_sustained_fast_max.ax[gpu_id, end])]
            if compare
                plot_lp(target.fig, target.ax, N_sustained_best, sustained_perf_max_best, point_id=3, line_id=1, color_id=0, label=gpu[gpu_id].best_env_label, open_symbol=true)
            end
            if has_openacc
                plot_lp(target.fig, target.ax, N_sustained_fast_acc_par, sustained_perf_fast_max_acc_par, point_id=2, line_id=2, color_id=2, label="acc (parallel)")
                plot_lp(target.fig, target.ax, N_sustained_fast_acc_ker, sustained_perf_fast_max_acc_ker, point_id=4, line_id=3, color_id=2, label="acc (kernels)")
            end
            plot_lp(target.fig, target.ax, N_sustained_fast_omp_dist, sustained_perf_fast_max_omp_dist, point_id=0, line_id=2, color_id=1, label="omp (distribute)")
            plot_lp(target.fig, target.ax, N_sustained_fast_omp_loop, sustained_perf_fast_max_omp_loop, point_id=1, line_id=3, color_id=1, label="omp (loop)")
        end

        # median performance (w/o optimization flag: full scale | specific GPU, GPU comparison)
        for target in [(fig=perf_base_med, ax=perf_base_med.ax[begin]), (fig=compare_base_med, ax=compare_base_med.ax[gpu_id, end])]
            if compare
                plot_lp(target.fig, target.ax, N_best, perf_med_best, point_id=3, line_id=1, color_id=0, label=gpu[gpu_id].best_env_label, open_symbol=true)
            end
            if has_openacc
                plot_lp(target.fig, target.ax, N_base_acc_par, perf_base_med_acc_par, point_id=2, line_id=2, color_id=2, label="acc (parallel)")
                plot_lp(target.fig, target.ax, N_base_acc_ker, perf_base_med_acc_ker, point_id=4, line_id=3, color_id=2, label="acc (kernels)")
            end
            plot_lp(target.fig, target.ax, N_base_omp_dist, perf_base_med_omp_dist, point_id=0, line_id=2, color_id=1, label="omp (distribute)")
            plot_lp(target.fig, target.ax, N_base_omp_loop, perf_base_med_omp_loop, point_id=1, line_id=3, color_id=1, label="omp (loop)")
        end
        # median performance (w/o optimization flag: sustained performance | specific GPU, GPU comparison)
        for target in [(fig=sustained_perf_base_med, ax=sustained_perf_base_med.ax[begin]), (fig=compare_sustained_base_med, ax=compare_sustained_base_med.ax[gpu_id, end])]
            if compare
                plot_lp(target.fig, target.ax, N_sustained_best, sustained_perf_med_best, point_id=3, line_id=1, color_id=0, label=gpu[gpu_id].best_env_label, open_symbol=true)
            end
            if has_openacc
                plot_lp(target.fig, target.ax, N_sustained_base_acc_par, sustained_perf_base_med_acc_par, point_id=2, line_id=2, color_id=2, label="acc (parallel)")
                plot_lp(target.fig, target.ax, N_sustained_base_acc_ker, sustained_perf_base_med_acc_ker, point_id=4, line_id=3, color_id=2, label="acc (kernels)")
            end
            plot_lp(target.fig, target.ax, N_sustained_base_omp_dist, sustained_perf_base_med_omp_dist, point_id=0, line_id=2, color_id=1, label="omp (distribute)")
            plot_lp(target.fig, target.ax, N_sustained_base_omp_loop, sustained_perf_base_med_omp_loop, point_id=1, line_id=3, color_id=1, label="omp (loop)")
        end
        # median performance (w/ optimization flag: full scale | specific GPU, GPU comparison)
        for target in [(fig=perf_fast_med, ax=perf_fast_med.ax[begin]), (fig=compare_fast_med, ax=compare_fast_med.ax[gpu_id, end])]
            if compare
                plot_lp(target.fig, target.ax, N_best, perf_med_best, point_id=3, line_id=1, color_id=0, label=gpu[gpu_id].best_env_label, open_symbol=true)
            end
            if has_openacc
                plot_lp(target.fig, target.ax, N_fast_acc_par, perf_fast_med_acc_par, point_id=2, line_id=2, color_id=2, label="acc (parallel)")
                plot_lp(target.fig, target.ax, N_fast_acc_ker, perf_fast_med_acc_ker, point_id=4, line_id=3, color_id=2, label="acc (kernels)")
            end
            plot_lp(target.fig, target.ax, N_fast_omp_dist, perf_fast_med_omp_dist, point_id=0, line_id=2, color_id=1, label="omp (distribute)")
            plot_lp(target.fig, target.ax, N_fast_omp_loop, perf_fast_med_omp_loop, point_id=1, line_id=3, color_id=1, label="omp (loop)")
        end
        # median performance (w/ optimization flag: sustained performance | specific GPU, GPU comparison)
        for target in [(fig=sustained_perf_fast_med, ax=sustained_perf_fast_med.ax[begin]), (fig=compare_sustained_fast_med, ax=compare_sustained_fast_med.ax[gpu_id, end])]
            if compare
                plot_lp(target.fig, target.ax, N_sustained_best, sustained_perf_med_best, point_id=3, line_id=1, color_id=0, label=gpu[gpu_id].best_env_label, open_symbol=true)
            end
            if has_openacc
                plot_lp(target.fig, target.ax, N_sustained_fast_acc_par, sustained_perf_fast_med_acc_par, point_id=2, line_id=2, color_id=2, label="acc (parallel)")
                plot_lp(target.fig, target.ax, N_sustained_fast_acc_ker, sustained_perf_fast_med_acc_ker, point_id=4, line_id=3, color_id=2, label="acc (kernels)")
            end
            plot_lp(target.fig, target.ax, N_sustained_fast_omp_dist, sustained_perf_fast_med_omp_dist, point_id=0, line_id=2, color_id=1, label="omp (distribute)")
            plot_lp(target.fig, target.ax, N_sustained_fast_omp_loop, sustained_perf_fast_med_omp_loop, point_id=1, line_id=3, color_id=1, label="omp (loop)")
        end

        if compare
            # best performance (w/o optimization flag: full scale; GPU comparison)
            for target in [(fig=compare_base_max, ax=compare_base_max.ax[gpu_id, begin])]
                if has_openacc
                    plot_lp(target.fig, target.ax, N_base_acc_par, perf_base_max_acc_par ./ perf_max_best, point_id=2, line_id=2, color_id=2, label="acc (parallel)")
                    plot_lp(target.fig, target.ax, N_base_acc_ker, perf_base_max_acc_ker ./ perf_max_best, point_id=4, line_id=3, color_id=2, label="acc (kernels)")
                end
                plot_lp(target.fig, target.ax, N_base_omp_dist, perf_base_max_omp_dist ./ perf_max_best, point_id=0, line_id=2, color_id=1, label="omp (distribute)")
                plot_lp(target.fig, target.ax, N_base_omp_loop, perf_base_max_omp_loop ./ perf_max_best, point_id=1, line_id=3, color_id=1, label="omp (loop)")
            end
            # best performance (w/o optimization flag: sustained performance; GPU comparison)
            for target in [(fig=compare_sustained_base_max, ax=compare_sustained_base_max.ax[gpu_id, begin])]
                if has_openacc
                    plot_lp(target.fig, target.ax, N_sustained_base_acc_par, sustained_perf_base_max_acc_par ./ sustained_perf_max_best, point_id=2, line_id=2, color_id=2, label="acc (parallel)")
                    plot_lp(target.fig, target.ax, N_sustained_base_acc_ker, sustained_perf_base_max_acc_ker ./ sustained_perf_max_best, point_id=4, line_id=3, color_id=2, label="acc (kernels)")
                end
                plot_lp(target.fig, target.ax, N_sustained_base_omp_dist, sustained_perf_base_max_omp_dist ./ sustained_perf_max_best, point_id=0, line_id=2, color_id=1, label="omp (distribute)")
                plot_lp(target.fig, target.ax, N_sustained_base_omp_loop, sustained_perf_base_max_omp_loop ./ sustained_perf_max_best, point_id=1, line_id=3, color_id=1, label="omp (loop)")
            end
            # best performance (w/ optimization flag: sustained performance; GPU comparison)
            for target in [(fig=compare_fast_max, ax=compare_fast_max.ax[gpu_id, begin])]
                if has_openacc
                    plot_lp(target.fig, target.ax, N_fast_acc_par, perf_fast_max_acc_par ./ perf_max_best, point_id=2, line_id=2, color_id=2, label="acc (parallel)")
                    plot_lp(target.fig, target.ax, N_fast_acc_ker, perf_fast_max_acc_ker ./ perf_max_best, point_id=4, line_id=3, color_id=2, label="acc (kernels)")
                end
                plot_lp(target.fig, target.ax, N_fast_omp_dist, perf_fast_max_omp_dist ./ perf_max_best, point_id=0, line_id=2, color_id=1, label="omp (distribute)")
                plot_lp(target.fig, target.ax, N_fast_omp_loop, perf_fast_max_omp_loop ./ perf_max_best, point_id=1, line_id=3, color_id=1, label="omp (loop)")
            end
            # best performance (w/ optimization flag: sustained performance; GPU comparison)
            for target in [(fig=compare_sustained_fast_max, ax=compare_sustained_fast_max.ax[gpu_id, begin])]
                if has_openacc
                    plot_lp(target.fig, target.ax, N_sustained_fast_acc_par, sustained_perf_fast_max_acc_par ./ sustained_perf_max_best, point_id=2, line_id=2, color_id=2, label="acc (parallel)")
                    plot_lp(target.fig, target.ax, N_sustained_fast_acc_ker, sustained_perf_fast_max_acc_ker ./ sustained_perf_max_best, point_id=4, line_id=3, color_id=2, label="acc (kernels)")
                end
                plot_lp(target.fig, target.ax, N_sustained_fast_omp_dist, sustained_perf_fast_max_omp_dist ./ sustained_perf_max_best, point_id=0, line_id=2, color_id=1, label="omp (distribute)")
                plot_lp(target.fig, target.ax, N_sustained_fast_omp_loop, sustained_perf_fast_max_omp_loop ./ sustained_perf_max_best, point_id=1, line_id=3, color_id=1, label="omp (loop)")
            end

            # median performance (w/o optimization flag: full scale; GPU comparison)
            for target in [(fig=compare_base_med, ax=compare_base_med.ax[gpu_id, begin])]
                if has_openacc
                    plot_lp(target.fig, target.ax, N_base_acc_par, perf_base_med_acc_par ./ perf_max_best, point_id=2, line_id=2, color_id=2, label="acc (parallel)")
                    plot_lp(target.fig, target.ax, N_base_acc_ker, perf_base_med_acc_ker ./ perf_max_best, point_id=4, line_id=3, color_id=2, label="acc (kernels)")
                end
                plot_lp(target.fig, target.ax, N_base_omp_dist, perf_base_med_omp_dist ./ perf_max_best, point_id=0, line_id=2, color_id=1, label="omp (distribute)")
                plot_lp(target.fig, target.ax, N_base_omp_loop, perf_base_med_omp_loop ./ perf_max_best, point_id=1, line_id=3, color_id=1, label="omp (loop)")
            end
            # median performance (w/o optimization flag: sustained performance; GPU comparison)
            for target in [(fig=compare_sustained_base_med, ax=compare_sustained_base_med.ax[gpu_id, begin])]
                if has_openacc
                    plot_lp(target.fig, target.ax, N_sustained_base_acc_par, sustained_perf_base_med_acc_par ./ sustained_perf_max_best, point_id=2, line_id=2, color_id=2, label="acc (parallel)")
                    plot_lp(target.fig, target.ax, N_sustained_base_acc_ker, sustained_perf_base_med_acc_ker ./ sustained_perf_max_best, point_id=4, line_id=3, color_id=2, label="acc (kernels)")
                end
                plot_lp(target.fig, target.ax, N_sustained_base_omp_dist, sustained_perf_base_med_omp_dist ./ sustained_perf_max_best, point_id=0, line_id=2, color_id=1, label="omp (distribute)")
                plot_lp(target.fig, target.ax, N_sustained_base_omp_loop, sustained_perf_base_med_omp_loop ./ sustained_perf_max_best, point_id=1, line_id=3, color_id=1, label="omp (loop)")
            end
            # median performance (w/ optimization flag: full scale; GPU comparison)
            for target in [(fig=compare_fast_med, ax=compare_fast_med.ax[gpu_id, begin])]
                if has_openacc
                    plot_lp(target.fig, target.ax, N_fast_acc_par, perf_fast_med_acc_par ./ perf_max_best, point_id=2, line_id=2, color_id=2, label="acc (parallel)")
                    plot_lp(target.fig, target.ax, N_fast_acc_ker, perf_fast_med_acc_ker ./ perf_max_best, point_id=5, line_id=4, color_id=2, label="acc (kernels)")
                end
                plot_lp(target.fig, target.ax, N_fast_omp_dist, perf_fast_med_omp_dist ./ perf_max_best, point_id=0, line_id=2, color_id=1, label="omp (distribute)")
                plot_lp(target.fig, target.ax, N_fast_omp_loop, perf_fast_med_omp_loop ./ perf_max_best, point_id=1, line_id=3, color_id=1, label="omp (loop)")
            end
            # median performance (w/ optimization flag: sustained performance; GPU comparison)
            for target in [(fig=compare_sustained_fast_med, ax=compare_sustained_fast_med.ax[gpu_id, begin])]
                if has_openacc
                    plot_lp(target.fig, target.ax, N_sustained_fast_acc_par, sustained_perf_fast_med_acc_par ./ sustained_perf_max_best, point_id=2, line_id=2, color_id=2, label="acc (parallel)")
                    plot_lp(target.fig, target.ax, N_sustained_fast_acc_ker, sustained_perf_fast_med_acc_ker ./ sustained_perf_max_best, point_id=4, line_id=3, color_id=2, label="acc (kernels)")
                end
                plot_lp(target.fig, target.ax, N_sustained_fast_omp_dist, sustained_perf_fast_med_omp_dist ./ sustained_perf_max_best, point_id=0, line_id=2, color_id=1, label="omp (distribute)")
                plot_lp(target.fig, target.ax, N_sustained_fast_omp_loop, sustained_perf_fast_med_omp_loop ./ sustained_perf_max_best, point_id=1, line_id=3, color_id=1, label="omp (loop)")
            end
        end

        for fig in [perf_base_max, perf_fast_max, perf_base_med, perf_fast_med]
            at = fig.ax[begin]
            at.loglog()
        end
        for fig in [sustained_perf_base_max, sustained_perf_fast_max, sustained_perf_base_med, sustained_perf_fast_med]
            at = fig.ax[begin]
            # xl, xr = at.get_xlim()
            # at.set_xlim(util_pyplot.scale_axis(gpu[gpu_id].num_cores, xr))
            at.semilogx()
            at.yaxis.set_major_formatter(PyPlot.matplotlib.ticker.FuncFormatter(util_pyplot.scientific))
        end
        for fig in [perf_base_max, perf_fast_max, perf_base_med, perf_fast_med, sustained_perf_base_max, sustained_perf_fast_max, sustained_perf_base_med, sustained_perf_fast_med]
            at = fig.ax[begin]
            at.grid()
            set_xlabel(fig, at, L"$N$")
            set_ylabel(fig, at, "Number of interactions per second")
        end

        # set label
        for fig in [perf_base_max, perf_fast_max, perf_base_med, perf_fast_med, sustained_perf_base_max, sustained_perf_fast_max, sustained_perf_base_med, sustained_perf_fast_med]
            at = fig.ax[begin]
            handles, labels = at.get_legend_handles_labels()
            at.legend(handles[end:-1:begin], labels[end:-1:begin], numpoints=1, handlelength=2.0, loc="best", fontsize=0.8 * fig.fs)
            handles = nothing
            labels = nothing
        end

        # save figures
        if !ispath("fig")
            mkdir("fig")
        end
        perf_base_max.fig.savefig(string("fig/", "nbody", "_base", "_max", "_", name, ".png"), format="png", dpi=100, bbox_inches="tight")
        perf_base_med.fig.savefig(string("fig/", "nbody", "_base", "_med", "_", name, ".png"), format="png", dpi=100, bbox_inches="tight")
        perf_fast_max.fig.savefig(string("fig/", "nbody", "_fast", "_max", "_", name, ".png"), format="png", dpi=100, bbox_inches="tight")
        perf_fast_med.fig.savefig(string("fig/", "nbody", "_fast", "_med", "_", name, ".png"), format="png", dpi=100, bbox_inches="tight")
        sustained_perf_base_max.fig.savefig(string("fig/", "nbody", "_sustained", "_base", "_max", "_", name, ".png"), format="png", dpi=100, bbox_inches="tight")
        sustained_perf_base_med.fig.savefig(string("fig/", "nbody", "_sustained", "_base", "_med", "_", name, ".png"), format="png", dpi=100, bbox_inches="tight")
        sustained_perf_fast_max.fig.savefig(string("fig/", "nbody", "_sustained", "_fast", "_max", "_", name, ".png"), format="png", dpi=100, bbox_inches="tight")
        sustained_perf_fast_med.fig.savefig(string("fig/", "nbody", "_sustained", "_fast", "_med", "_", name, ".png"), format="png", dpi=100, bbox_inches="tight")
        if output_pdf
            perf_base_max.fig.savefig(string("fig/", "nbody", "_base", "_max", "_", name, ".pdf"), format="pdf", bbox_inches="tight")
            perf_base_med.fig.savefig(string("fig/", "nbody", "_base", "_med", "_", name, ".pdf"), format="pdf", bbox_inches="tight")
            perf_fast_max.fig.savefig(string("fig/", "nbody", "_fast", "_max", "_", name, ".pdf"), format="pdf", bbox_inches="tight")
            perf_fast_med.fig.savefig(string("fig/", "nbody", "_fast", "_med", "_", name, ".pdf"), format="pdf", bbox_inches="tight")
            sustained_perf_base_max.fig.savefig(string("fig/", "nbody", "_sustained", "_base", "_max", "_", name, ".pdf"), format="pdf", bbox_inches="tight")
            sustained_perf_base_med.fig.savefig(string("fig/", "nbody", "_sustained", "_base", "_med", "_", name, ".pdf"), format="pdf", bbox_inches="tight")
            sustained_perf_fast_max.fig.savefig(string("fig/", "nbody", "_sustained", "_fast", "_max", "_", name, ".pdf"), format="pdf", bbox_inches="tight")
            sustained_perf_fast_med.fig.savefig(string("fig/", "nbody", "_sustained", "_fast", "_med", "_", name, ".pdf"), format="pdf", bbox_inches="tight")
        end

        PyPlot.close(perf_base_max.fig)
        PyPlot.close(perf_base_med.fig)
        PyPlot.close(perf_fast_max.fig)
        PyPlot.close(perf_fast_med.fig)
        PyPlot.close(sustained_perf_base_max.fig)
        PyPlot.close(sustained_perf_base_med.fig)
        PyPlot.close(sustained_perf_fast_max.fig)
        PyPlot.close(sustained_perf_fast_med.fig)
        perf_base_max = nothing
        perf_base_med = nothing
        perf_fast_max = nothing
        perf_fast_med = nothing
        sustained_perf_base_max = nothing
        sustained_perf_base_med = nothing
        sustained_perf_fast_max = nothing
        sustained_perf_fast_med = nothing
    end

    # output summary CSV file
    summary |> CSV.write(string("nbody", "_stats", ".csv"), delim=',', writeheader=true)

    for at in compare_base_max.ax[:, end]
        at.set_ylim(util_pyplot.scale_axis(min_of_perf_base_max, max_of_perf_base_max, logPlt=true))
    end
    for at in compare_fast_max.ax[:, end]
        at.set_ylim(util_pyplot.scale_axis(min_of_perf_fast_max, max_of_perf_fast_max, logPlt=true))
    end
    for at in compare_base_med.ax[:, end]
        at.set_ylim(util_pyplot.scale_axis(min_of_perf_base_med, max_of_perf_base_med, logPlt=true))
    end
    for at in compare_fast_med.ax[:, end]
        at.set_ylim(util_pyplot.scale_axis(min_of_perf_fast_med, max_of_perf_fast_med, logPlt=true))
    end
    for fig in [compare_base_max, compare_fast_max, compare_base_med, compare_fast_med]
        for at in fig.ax[:, end]
            at.loglog()
        end
    end
    for at in compare_sustained_base_max.ax[:, end]
        at.set_ylim(util_pyplot.scale_axis(min_of_sustained_perf_base_max, max_of_sustained_perf_base_max, logPlt=false))
    end
    for at in compare_sustained_fast_max.ax[:, end]
        at.set_ylim(util_pyplot.scale_axis(min_of_sustained_perf_fast_max, max_of_sustained_perf_fast_max, logPlt=false))
    end
    for at in compare_sustained_base_med.ax[:, end]
        at.set_ylim(util_pyplot.scale_axis(min_of_sustained_perf_base_med, max_of_sustained_perf_base_med, logPlt=false))
    end
    for at in compare_sustained_fast_med.ax[:, end]
        at.set_ylim(util_pyplot.scale_axis(min_of_sustained_perf_fast_med, max_of_sustained_perf_fast_med, logPlt=false))
    end
    if compare
        for at in compare_base_max.ax[:, begin]
            at.set_ylim(util_pyplot.scale_axis(min_of_perf_base_max_speedup, max_of_perf_base_max_speedup, logPlt=false))
        end
        for at in compare_fast_max.ax[:, begin]
            at.set_ylim(util_pyplot.scale_axis(min_of_perf_fast_max_speedup, max_of_perf_fast_max_speedup, logPlt=false))
        end
        for at in compare_base_med.ax[:, begin]
            at.set_ylim(util_pyplot.scale_axis(min_of_perf_base_med_speedup, max_of_perf_base_med_speedup, logPlt=false))
        end
        for at in compare_fast_med.ax[:, begin]
            at.set_ylim(util_pyplot.scale_axis(min_of_perf_fast_med_speedup, max_of_perf_fast_med_speedup, logPlt=false))
        end
        for fig in [compare_base_max, compare_fast_max, compare_base_med, compare_fast_med]
            for at in fig.ax[:, begin]
                at.semilogx()
            end
        end
        for at in compare_sustained_base_max.ax[:, begin]
            at.set_ylim(util_pyplot.scale_axis(min_of_sustained_perf_base_max_speedup, max_of_sustained_perf_base_max_speedup, logPlt=false))
        end
        for at in compare_sustained_fast_max.ax[:, begin]
            at.set_ylim(util_pyplot.scale_axis(min_of_sustained_perf_fast_max_speedup, max_of_sustained_perf_fast_max_speedup, logPlt=false))
        end
        for at in compare_sustained_base_med.ax[:, begin]
            at.set_ylim(util_pyplot.scale_axis(min_of_sustained_perf_base_med_speedup, max_of_sustained_perf_base_med_speedup, logPlt=false))
        end
        for at in compare_sustained_fast_med.ax[:, begin]
            at.set_ylim(util_pyplot.scale_axis(min_of_sustained_perf_fast_med_speedup, max_of_sustained_perf_fast_med_speedup, logPlt=false))
        end
    end
    for fig in [compare_sustained_base_max, compare_sustained_fast_max, compare_sustained_base_med, compare_sustained_fast_med]
        for ii in 1:fig.nx
            for jj in 1:fig.ny
                at = fig.ax[ii, jj]
                # xl, xr = at.get_xlim()
                # at.set_xlim(util_pyplot.scale_axis(gpu[ii].num_cores, xr))
                at.semilogx()
                at.tick_params(axis="x", which="minor", labelbottom=false)
            end
            fig.ax[ii, begin].tick_params(axis="x", which="minor", labelbottom=true, labelsize=0.6 * fig.fs, pad=0.3 * fig.fs)
        end
        fig.ax[begin, ny].yaxis.set_major_formatter(PyPlot.matplotlib.ticker.FuncFormatter(util_pyplot.scientific))
    end
    for fig in [compare_base_max, compare_fast_max, compare_base_med, compare_fast_med, compare_sustained_base_max, compare_sustained_fast_max, compare_sustained_base_med, compare_sustained_fast_med]
        for at in fig.ax
            at.grid()
        end
        set_ylabel(fig, fig.ax[begin, ny], "Number of interactions per second")
        if compare
            set_ylabel(fig, fig.ax[begin, begin], "Ratio to the dedicated implementation")
        end

        for ii in 1:fig.nx
            set_xlabel(fig, fig.ax[ii, begin], L"$N$")
            for jj in 1:fig.ny
                maptag::String = ""
                if jj == fig.ny
                    maptag = gpu[ii].caption
                end
                id = (ii - 1) + fig.nx * (fig.ny - jj)
                caption = string("(", Char(97 + id), ")")
                at = fig.ax[ii, jj]
                at.text(0.02, 0.98, string(caption, "~", maptag), color="black", fontsize=fig.fs * 0.8, horizontalalignment="left", verticalalignment="top", transform=at.transAxes, bbox=Dict("facecolor" => "white", "edgecolor" => "None", "alpha" => 0.75))
            end

            at = fig.ax[ii, end]
            handles, labels = at.get_legend_handles_labels()
            at.legend(handles[end:-1:begin], labels[end:-1:begin], numpoints=1, handlelength=2.0, bbox_to_anchor=(0.99, 0.67), loc="upper right", fontsize=fig.fs * 0.75)
            # at.legend(handles[end:-1:begin], labels[end:-1:begin], numpoints=1, handlelength=2.0, loc="best", fontsize=fig.fs * 0.8)
            handles = nothing
            labels = nothing
        end
    end

    # save figures
    compare_base_max.fig.savefig(string("fig/", "nbody", "_base", "_max", ".png"), format="png", dpi=100, bbox_inches="tight")
    compare_base_med.fig.savefig(string("fig/", "nbody", "_base", "_med", ".png"), format="png", dpi=100, bbox_inches="tight")
    compare_fast_max.fig.savefig(string("fig/", "nbody", "_fast", "_max", ".png"), format="png", dpi=100, bbox_inches="tight")
    compare_fast_med.fig.savefig(string("fig/", "nbody", "_fast", "_med", ".png"), format="png", dpi=100, bbox_inches="tight")
    compare_sustained_base_max.fig.savefig(string("fig/", "nbody", "_sustained", "_base", "_max", ".png"), format="png", dpi=100, bbox_inches="tight")
    compare_sustained_base_med.fig.savefig(string("fig/", "nbody", "_sustained", "_base", "_med", ".png"), format="png", dpi=100, bbox_inches="tight")
    compare_sustained_fast_max.fig.savefig(string("fig/", "nbody", "_sustained", "_fast", "_max", ".png"), format="png", dpi=100, bbox_inches="tight")
    compare_sustained_fast_med.fig.savefig(string("fig/", "nbody", "_sustained", "_fast", "_med", ".png"), format="png", dpi=100, bbox_inches="tight")
    if output_pdf
        compare_base_max.fig.savefig(string("fig/", "nbody", "_base", "_max", ".pdf"), format="pdf", bbox_inches="tight")
        compare_base_med.fig.savefig(string("fig/", "nbody", "_base", "_med", ".pdf"), format="pdf", bbox_inches="tight")
        compare_fast_max.fig.savefig(string("fig/", "nbody", "_fast", "_max", ".pdf"), format="pdf", bbox_inches="tight")
        compare_fast_med.fig.savefig(string("fig/", "nbody", "_fast", "_med", ".pdf"), format="pdf", bbox_inches="tight")
        compare_sustained_base_max.fig.savefig(string("fig/", "nbody", "_sustained", "_base", "_max", ".pdf"), format="pdf", bbox_inches="tight")
        compare_sustained_base_med.fig.savefig(string("fig/", "nbody", "_sustained", "_base", "_med", ".pdf"), format="pdf", bbox_inches="tight")
        compare_sustained_fast_max.fig.savefig(string("fig/", "nbody", "_sustained", "_fast", "_max", ".pdf"), format="pdf", bbox_inches="tight")
        compare_sustained_fast_med.fig.savefig(string("fig/", "nbody", "_sustained", "_fast", "_med", ".pdf"), format="pdf", bbox_inches="tight")
    end

    PyPlot.close("all")
    return nothing
end


main()
