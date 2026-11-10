# Draws the performance measured by sh/<system>/iterate.sh for each way of managing the memory; run in the sample directory:
#   julia --project=jl jl/plot/diffusion_mode.jl [options] [DIR ...]
# DIR: a directory made by iterate.sh (<host>_<target>_<arch>, holding iterate/model<N>/diffusion_benchmark_*.csv);
#      without DIR, every directory in the current directory that holds iterate/ is drawn
# A row of panels for each DIR (the first at the top), a column for each way of offloading; the lines are explicit data transfer
# (models 0 to 3), managed memory (4 to 7), unified memory (8 to 11), and unified memory with first touch (12 to 15), where measured;
# writes fig/diffusion_mode_{max,med}.png and fig/diffusion_mode_stats.csv.
using ArgParse
using CSV
using PyPlot
include("../util/pyplot.jl")
include("../util/results.jl")

function main()
    cfg = ArgParseSettings(description="Draw the performance measured by sh/<system>/iterate.sh for each way of managing the memory (run in the sample directory)")
    util_results.add_common_args!(cfg, "iterate")
    util_results.add_opt_arg!(cfg)
    argv = parse_args(cfg)
    util_pyplot.config(usetex=argv["usetex"])
    dir_levels = util_results.opt_levels(argv, length(util_results.result_dirs(argv, "iterate")))

    lines = [
        (offset=0, level=nothing, mode="explicit", label="explicit", point_id=0, line_id=0, color_id=0),
        (offset=4, level=nothing, mode="managed", label="managed", point_id=1, line_id=1, color_id=2),
        (offset=8, level=nothing, mode="unified", label="unified", point_id=2, line_id=2, color_id=3),
        (offset=12, level=nothing, mode="unified, first touch", label="unified, first touch", point_id=3, line_id=3, color_id=1),
    ]
    summary = util_results.draw_grid(argv, "iterate", lines, (:max, :med), "diffusion_mode", dir_levels=dir_levels, legend_each_row=true)

    # output summary CSV file
    summary |> CSV.write(string("fig/", "diffusion", "_mode", "_stats", ".csv"), delim=',', writeheader=true)
    return nothing
end


if abspath(PROGRAM_FILE) == @__FILE__
    main()
end
