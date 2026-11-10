# Installs the packages of the plotting scripts (jl/Project.toml) into the environment of jl/ (writes jl/Manifest.toml);
# the default Julia environment is not changed. Run in the sample directory: julia jl/package.jl
using Pkg
Pkg.activate(@__DIR__)
Pkg.instantiate()
