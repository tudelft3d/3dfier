/* ptinpoly.h - point in polygon inside/outside algorithms header file.
 *
 * by Eric Haines, 3D/Eye Inc, erich@eye.com
 */

#ifdef __cplusplus
extern "C"
{
#endif
/* =========================== System Related ============================= */
/* SRAN initializes random number generator, if needed */
#define SRAN()		srand(1)
/* RAN01 returns a double from [0..1) */
#define RAN01()		((double)rand() / 32768.0)

/* =========== Grid stuff ================================================= */

#define GR_FULL_VERT	0x01	/* line crosses vertically */
#define GR_FULL_HORZ	0x02	/* line crosses horizontally */

typedef struct {
    double	xa,ya ;
    double	minx, maxx, miny, maxy ;
    double	ax, ay ;
    double	slope, inv_slope ;
} GridRec, *pGridRec;

#define GC_BL_IN	0x0001	/* bottom left corner is in (else out) */
#define GC_BR_IN	0x0002	/* bottom right corner is in (else out) */
#define GC_TL_IN	0x0004	/* top left corner is in (else out) */
#define GC_TR_IN	0x0008	/* top right corner is in (else out) */
#define GC_L_EDGE_HIT	0x0010	/* left edge is crossed */
#define GC_R_EDGE_HIT	0x0020	/* right edge is crossed */
#define GC_B_EDGE_HIT	0x0040	/* bottom edge is crossed */
#define GC_T_EDGE_HIT	0x0080	/* top edge is crossed */
#define GC_B_EDGE_PARITY	0x0100	/* bottom edge parity */
#define GC_T_EDGE_PARITY	0x0200	/* top edge parity */
#define GC_AIM_L	(0<<10) /* aim towards left edge */
#define GC_AIM_B	(1<<10) /* aim towards bottom edge */
#define GC_AIM_R	(2<<10) /* aim towards right edge */
#define GC_AIM_T	(3<<10) /* aim towards top edge */
#define GC_AIM_C	(4<<10) /* aim towards a corner */
#define GC_AIM		0x1c00

#define GC_L_EDGE_CLEAR GC_L_EDGE_HIT
#define GC_R_EDGE_CLEAR GC_R_EDGE_HIT
#define GC_B_EDGE_CLEAR GC_B_EDGE_HIT
#define GC_T_EDGE_CLEAR GC_T_EDGE_HIT

#define GC_ALL_EDGE_CLEAR	(GC_L_EDGE_HIT | \
				 GC_R_EDGE_HIT | \
				 GC_B_EDGE_HIT | \
				 GC_T_EDGE_HIT )

typedef struct {
    short		tot_edges ;
    unsigned short	gc_flags ;
    GridRec		*gr ;
} GridCell, *pGridCell;

typedef struct {
    int		xres, yres ;	/* grid size */
    int		tot_cells ;	/* xres * yres */
    double	minx, maxx, miny, maxy ;	/* bounding box */
    double	xdelta, ydelta ;
    double	inv_xdelta, inv_ydelta ;
    double	*glx, *gly ;
    GridCell	*gc ;
} GridSet, *pGridSet ;

typedef struct {
  double x, y;
} Pipoint, *pPipoint;

/* add a little to the limits of the polygon bounding box to avoid precision
* problems.
*/
#define EPSILON		0.00001

/* The following structure is associated with a polygon */
typedef struct {
    int		id ;		/* vertex number of edge */
    int		full_cross ;	/* 1 if extends from top to bottom */
    double	minx, maxx ;	/* X bounds for bin */
} Edge, *pEdge ;

void GridSetup(pPipoint pgon[], int	numverts, int	resolution, pGridSet p_gs);
int AddGridRecAlloc(pGridCell p_gc, double xa, double ya, double xb, double yb, double eps);
void GridCleanup(pGridSet	p_gs);
int GridTest(pGridSet p_gs, pPipoint point);

#ifdef __cplusplus
}
#endif