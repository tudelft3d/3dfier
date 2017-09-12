### OVERLAPTILES & PIPELINE
const epsg = 32748  # UTM 48S

### OVERLAPTILES
# set the correct paths
const globstring = "*.laz"
const lasdir = "/Users/epta/workdir/merged/"
const lastilesdir = "/Users/epta/workdir/vanRonald/tiled/"
const ext = "laz"  # output format, "las" or "laz"
const cellsize = 1000.0  # size of a tile [m]
const overlap = 100.0  # width of overlap [m]
const nmax = 2^24  # number of points to process per iteration, reduces max RAM usage [-]
const buffersize = 500.0  # carefully tuned, don't change [m]
const edges = 32  # number of edges used in buffer [-]

### PIPELINE
const casename = ""  # subdirectory to place results in, root if ""

# general settings
const high_res = 1.0  # cellsize for water grids & and high res dtm [m]
const low_res = 25.0  # cellsize for coarse DTM [m]
const min_points = 0.5  # min no of high res ground cells in low res grid [-]
const nodata = -9999.0
const mask_convex_hull = true  # set this to false for runnning without GeoJSON files

# ground pmf filter parameters
const radius = 16.0
const slope = 0.01
const dh_max = 2.5
const dh_min = 0.1

# water filter parameters
const wf_radius = 16.0
const wf_slope = 0.1
const wf_dh_max = 1.0
const wf_dh_min = 0.2
const int_thresh = 20
const roughness_thresh = 0.05
const wf_tolerance = 0.10

# mc pmf parameters
# max radius for size of filtered features (32m for buildings)
mc_radius = [8.0, 16.0, 32.0]
# Slope is for flat terrain (small = mounds removed)
mc_slope = [0.01, 0.02, 0.04, 0.08, 0.16, 0.32, 0.64, 1.28]
# Maximum unfiltered height, 2.5 for single floor buildings (lowest buildings)
mc_dh_max = [1.0, 2.5, 5.0]
# Small features, if height is greater than dh_min => non ground
mc_dh_min = [0.01, 0.02, 0.04, 0.06, 0.08, 0.1, 0.2, 0.4, 0.6, 1.0]
