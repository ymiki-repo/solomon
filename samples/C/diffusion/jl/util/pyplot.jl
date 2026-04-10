module util_pyplot

using PyPlot
function config(; pkg="\\usepackage{physics,siunitx}")
    rcParams = PyPlot.PyDict(PyPlot.matplotlib."rcParams")
    # embed fonts
    rcParams["ps.useafm"] = true
    rcParams["pdf.use14corefonts"] = true
    rcParams["text.usetex"] = true
    # use packages
    rcParams["text.latex.preamble"] = pkg
    return nothing
end


using Parameters
@with_kw struct MyPlotType
    num::Int = 0
    type::Array{Any,1}
end

function call(base::MyPlotType; id::Integer=0)
    return base.type[begin+(id%base.num)]
end


using PyCall
@with_kw mutable struct Panel
    fig::Figure

    nx::Integer = 1
    ny::Integer = 1
    ax::Array{PyCall.PyObject,2} # axes

    fs::Int32 # font size
    ms::Float32 # marker size
    lw::Float32 # line width

    point::MyPlotType # point type
    line::MyPlotType # line style
    color::MyPlotType
    mono::MyPlotType # color (monochrome)
end

# chars: number of characters (for setting fontsize)
# dpi: dots per inch (for setting resolution)
# inch: size of panels in units of inch (A4 is 8.27 inch * 14.32 inch)
function set_Panel(; nx::Integer=1, ny::Integer=1, share_xaxis::Bool=true, share_yaxis::Bool=true, chars::Float32=24.0f0, dpi::Float32=300.0f0, inch::Float32=15.0f0, xscale::Float32=1.0f0, yscale::Float32=1.0f0)
    # set sizes (font size, marker size, line width, and tick length)
    scale_factor = 1.0f0 * cbrt(Float32(max(nx, ny)))
    fontsize = Int32(round(scale_factor * inch * 72 / chars)) # 72 pt = 1 inch
    markersize = scale_factor * inch
    linewidth = scale_factor * inch * 0.25f0
    ticklength = 6.0f0 * linewidth

    # configure axes
    xmin, xmax = 0.0f0, 1.0f0
    ymin, ymax = 0.0f0, 1.0f0
    xbin = (xmax - xmin) / Float32(nx)
    ybin = (ymax - ymin) / Float32(ny)
    xmargin, ymargin = 0.0f0, 0.0f0
    margin = 0.15f0
    if !share_yaxis
        xmin = 0.0f0
        xbin = 1.0f0 / Float32(nx)
        xmargin = xbin * margin
    end
    if !share_xaxis
        ymin = 0.0f0
        ybin = 1.0f0 / Float32(ny)
        ymargin = ybin * margin
    end

    # set default symbols
    pt = MyPlotType(num=11, type=["o", "s", "^", "D", "v", "+", "x", "*", "h", "p", "."])
    ls = MyPlotType(num=5, type=["solid", (0, (1, 1)), (0, (5, 5)), (0, (5, 1, 1, 1)), (0, (5, 1, 1, 1, 1, 1, 1, 1))])
    cl = MyPlotType(num=10,
        # taken from Model Color Palette for Color Universal Design ver.4 (pages 7 and 2)
        # conversion using https://hogehoge.tk/tool/number.html
        type=[
            "#000000",# black
            "#ff4b00",# red
            "#005aff",# blue
            "#f6aa00",# orange
            "#03af7a",# green
            "#4dc4ff",# sky blue
            "#804000",# brown
            "#990099",# purple
            "#fff100",# yellow
            "#ff8082",# pink
        ]
    )
    mn = MyPlotType(num=4,
        # taken from Model Color Palette for Color Universal Design ver.4 (page 2)
        # conversion using https://hogehoge.tk/tool/number.html
        type=
        [
            "#000000",# black
            "#84919e",# dark gray
            "#c8c8cb",# light gray
            "#ffffff"# white
        ]
    )


    # construct structure
    panel = Panel(
        fig=PyPlot.figure(figsize=(inch * xscale * nx, inch * yscale * ny), dpi=dpi),
        nx=nx, ny=ny, ax=Array{PyCall.PyObject,2}(undef, (nx, ny)),
        fs=fontsize, ms=markersize, lw=linewidth,
        point=pt, line=ls, color=cl, mono=mn
    )

    # commit axes
    for jj in 1:ny
        yl = ymin + Float32(jj) * ybin + ymargin
        for ii in 1:nx
            xl = xmin + Float32(ii) * xbin + xmargin
            panel.ax[ii, jj] = panel.fig.add_axes((xl, yl, xbin - 2.0f0 * xmargin, ybin - 2.0f0 * ymargin))
        end
    end

    # configure axes
    for at in panel.ax
        for axis in ["top", "bottom", "left", "right"]
            at.spines[axis].set_linewidth(linewidth)
        end
        at.tick_params(axis="both", direction="in", bottom=true, top=true, left=true, right=true, labelsize=fontsize, length=ticklength, width=linewidth)
        at.tick_params(axis="x", pad=0.3f0 * fontsize)
        at.tick_params(axis="both", which="minor", direction="in", bottom=true, top=true, left=true, right=true, length=0.5f0 * ticklength, width=0.5f0 * linewidth)
        if share_xaxis
            at.tick_params(labelbottom=false)
        end
        if share_yaxis
            at.tick_params(labelleft=false)
        end
    end
    if share_xaxis
        for ii in 1:nx
            panel.ax[ii, begin].tick_params(labelbottom=true)
        end
    end
    if share_yaxis
        for jj in 1:ny
            panel.ax[begin, jj].tick_params(labelleft=true)
        end
    end

    return panel
end



function scale_axis(minimum::Real, maximum::Real; logPlt::Bool=true)
    blank_val = 0.2
    if logPlt
        width = log10(maximum / minimum)
        blank = width * blank_val * 0.5
        scale = 10.0^blank
        return (minimum / scale, maximum * scale)
    else
        width = maximum - minimum
        blank = width * blank_val * 0.5
        return (minimum - blank, maximum + blank)
    end
end

using Printf
function scientific(val::Real, pos)
    str = @sprintf "%.1e" val
    a, b = split(str, 'e')
    return string("\$", a, " \\times 10^{", parse(Int, b), "}\$")
end
function scientific2(val::Real, pos)
    str = @sprintf "%.2e" val
    a, b = split(str, 'e')
    return string("\$", a, " \\times 10^{", parse(Int, b), "}\$")
end

end


# import sys
# def add_colorbar(fig, ax, img, width, label, fs, lw, vertical = True, multiple = True):
#     if multiple:
#         x0, x1 = sys.float_info.max, -sys.float_info.max
#         y0, y1 = sys.float_info.max, -sys.float_info.max
#         for at in ax:
#             xl, xr = at.get_position().x0, at.get_position().x1
#             yb, yt = at.get_position().y0, at.get_position().y1
#             x0 = min(x0, xl)
#             x1 = max(x1, xr)
#             y0 = min(y0, yb)
#             y1 = max(y1, yt)
#     else:
#         x0, x1 = ax.get_position().x0, ax.get_position().x1
#         y0, y1 = ax.get_position().y0, ax.get_position().y1
#     if vertical:
#         cax = fig.add_axes([x1, y0, width, y1 - y0])
#         bar = fig.colorbar(img, cax = cax)
#         bar.solids.set_edgecolor('face')
#         bar.set_label(label, fontsize = fs)
#         cax.tick_params(axis = 'y', labelsize = fs, left = False, right = True, length = 12.0 * lw, width = lw, labelleft = False, labelright = True)
#         cax.tick_params(axis = 'y', which = 'minor', left = False, right = True, length = 6.0 * lw, width = 0.5 * lw)
#     else:
#         cax = fig.add_axes([x0, y1, x1 - x0, width])
#         bar = fig.colorbar(img, cax = cax, orientation = 'horizontal')
#         bar.solids.set_edgecolor('face')
#         bar.set_label(label, fontsize = fs, labelpad = 0.5 * fs)
#         cax.tick_params(axis = 'x', labelsize = fs, bottom = False, top = True, length = 12.0 * lw, width = lw, labelbottom = False, labeltop = True)
#         cax.tick_params(axis = 'x', which = 'minor', bottom = False, top = True, length = 6.0 * lw, width = 0.5 * lw)
#         cax.xaxis.set_label_position('top')

# def set_shared_xlabel(ax, xlabel):
#     fig = ax[-1].figure
#     fig.canvas.draw()
#     # get the corner for all plots
#     x0, x1 = 1, 0
#     y0, y1 = 1, 0
#     for at in ax:
#         at.set_xlabel('') # remove existing xlabels
#         xl, xr = at.get_position().x0, at.get_position().x1
#         bboxes, _ = at.xaxis.get_ticklabel_extents(fig.canvas.renderer)
#         bboxes = bboxes.inverse_transformed(fig.transFigure)
#         yb, yt = bboxes.y0, bboxes.y1
#         if x0 > xl:
#             x0 = xl
#         if x1 < xr:
#             x1 = xr
#         if y0 > yb:
#             y0 = yb
#         if y1 < yt:
#             y1 = yt
#     # set position of label
#     ax[-1].set_xlabel(xlabel)
#     # ax[-1].xaxis.set_label_coords((x0 + x1) / 2, (y0 + y1) / 2, transform = fig.transFigure)
#     ax[-1].xaxis.set_label_coords((x0 + x1) / 2, y0, transform = fig.transFigure)

# def set_shared_ylabel(ax, ylabel):
#     fig = ax[-1].figure
#     fig.canvas.draw()
#     # get the corner for all plots
#     x0, x1 = 1, 0
#     y0, y1 = 1, 0
#     for at in ax:
#         at.set_ylabel('') # remove existing ylabels
#         yb, yt = at.get_position().y0, at.get_position().y1
#         bboxes, _ = at.yaxis.get_ticklabel_extents(fig.canvas.renderer)
#         bboxes = bboxes.inverse_transformed(fig.transFigure)
#         xl, xr = bboxes.x0, bboxes.x1
#         if x0 > xl:
#             x0 = xl
#         if x1 < xr:
#             x1 = xr
#         if y0 > yb:
#             y0 = yb
#         if y1 < yt:
#             y1 = yt
#     # set position of label
#     ax[-1].set_ylabel(ylabel)
#     ax[-1].yaxis.set_label_coords((x0 + x1) / 2, (y0 + y1) / 2, transform = fig.transFigure)

# def set_global_title(fig, title, fontsize, offset):
#     y1 = 0
#     for at in fig.get_axes():
#         yt = at.get_position().y1
#         if y1 < yt:
#             y1 = yt
#     fig.suptitle(title, fontsize = fontsize, y = offset + y1, verticalalignment = 'bottom')
