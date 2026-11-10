# Draws the performance measured by sh/<system>/iterate.sh at each optimization level; run in the sample directory:
#   julia --project=jl jl/plot/diffusion_iterate.jl [options] [DIR ...]
# DIR: a directory made by iterate.sh (<host>_<target>_<arch>, holding iterate/model<N>/diffusion_benchmark_*.csv);
#      without DIR, every directory in the current directory that holds iterate/ is drawn
# A row of panels for each DIR (the first at the top), a column for each way of offloading (models 0 to 3, explicit data transfer);
# writes fig/diffusion_iterate_opt_{max,med}.png and fig/diffusion_iterate_stats.csv.
using ArgParse
using CSV
using PyPlot
include("../util/pyplot.jl")
include("../util/results.jl")

function main()
    cfg = ArgParseSettings(description="Draw the performance measured by sh/<system>/iterate.sh at each optimization level (run in the sample directory)")
    util_results.add_common_args!(cfg, "iterate")
    argv = parse_args(cfg)
    util_pyplot.config(usetex=argv["usetex"])

    # OPT_LEVEL 1 to 4 of the Makefile (the legend shows the names in the CSV files)
    lines = [
        (offset=0, level=1, mode="explicit", label=nothing, point_id=1, line_id=1, color_id=2),
        (offset=0, level=2, mode="explicit", label=nothing, point_id=2, line_id=2, color_id=3),
        (offset=0, level=3, mode="explicit", label=nothing, point_id=3, line_id=3, color_id=0),
        (offset=0, level=4, mode="explicit", label=nothing, point_id=0, line_id=0, color_id=1),
    ]
    summary = util_results.draw_grid(argv, "iterate", lines, (:max, :med), "diffusion_iterate_opt")

    # output summary CSV file
    summary |> CSV.write(string("fig/", "diffusion", "_iterate", "_stats", ".csv"), delim=',', writeheader=true)
    return nothing
end


if abspath(PROGRAM_FILE) == @__FILE__
    main()
end
