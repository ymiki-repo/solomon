# Draws the performance of the runs of sh/<system>/verify.sh at each optimization level; run in the sample directory:
#   julia --project=jl jl/plot/diffusion_verify.jl [options] [DIR ...]
# DIR: a directory made by verify.sh (<host>_<target>_<arch>, holding verify/model<N>/diffusion_benchmark_*.csv);
#      without DIR, every directory in the current directory that holds verify/ is drawn
# A row of panels for each DIR (the first at the top), a column for each way of offloading (models 0 to 3, explicit data transfer);
# writes fig/diffusion_verify_opt.png (the errors are judged by verify.sh, not drawn here).
using ArgParse
using PyPlot
include("../util/pyplot.jl")
include("../util/results.jl")

function main()
    cfg = ArgParseSettings(description="Draw the performance of the runs of sh/<system>/verify.sh at each optimization level (run in the sample directory)")
    util_results.add_common_args!(cfg, "verify")
    argv = parse_args(cfg)
    util_pyplot.config(usetex=argv["usetex"])

    # OPT_LEVEL 0 to 4 of the Makefile (the legend shows the names in the CSV files)
    lines = [(offset=0, level=level, mode="explicit", label=nothing, point_id=level, line_id=level, color_id=level) for level in 0:4]
    util_results.draw_grid(argv, "verify", lines, (:max,), "diffusion_verify_opt")
    return nothing
end


if abspath(PROGRAM_FILE) == @__FILE__
    main()
end
