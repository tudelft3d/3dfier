/* ptinpoly.c - point in polygon inside/outside code.

   by Eric Haines, 3D/Eye Inc, erich@eye.com

   This code contains the following algorithms:
	grid testing - grid imposed on polygon
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ptinpoly.h"

#define X	0
#define Y	1

#ifndef TRUE
#define TRUE	1
#define FALSE	0
#endif

#ifndef HUGE
#define HUGE	1.797693134862315e+308
#endif

#ifndef M_PI
#define M_PI	3.14159265358979323846
#endif

/* test if a & b are within epsilon.  Favors cases where a < b */
#define Near(a,b,eps)	( ((b)-(eps)<(a)) && ((a)-(eps)<(b)) )

#define MALLOC_CHECK( a )	if ( !(a) ) {				   \
				    fprintf( stderr, "out of memory\n" ) ; \
				    exit(1) ;				   \
				}

/* ======= Grid algorithm ================================================= */

/* Impose a grid upon the polygon and test only the local edges against the
 * point.
 *
 * Call setup with 2D polygon _pgon_ with _numverts_ number of vertices,
 * grid resolution _resolution_ and a pointer to a grid structure _p_gs_.
 * Call testing procedure with a pointer to this array and test point _point_,
 * returns 1 if inside, 0 if outside.
 * Call cleanup with pointer to grid structure to free space.
 */

/* Strategy for setup:
 *   Get bounds of polygon, allocate grid.
 *   "Walk" each edge of the polygon and note which edges have been crossed
 *     and what cells are entered (points on a grid edge are always considered
 *     to be above that edge).	Keep a record of the edges overlapping a cell.
 *     For cells with edges, determine if any cell border has no edges passing
 *     through it and so can be used for shooting a test ray.
 *     Keep track of the parity of the x (horizontal) grid cell borders for
 *     use in determining whether the grid corners are inside or outside.
 */
void GridSetup(pPipoint pgon[], int	numverts, int	resolution, pGridSet p_gs)
{
double	*vtx0, *vtx1, *vtxa, *vtxb, *p_gl ;
int	i, j, gc_clear_flags ;
double	vx0, vx1, vy0, vy1, gxdiff, gydiff, eps ;
pGridCell	p_gc, p_ngc ;
double	xdiff, ydiff, tmax, inv_x, inv_y, xdir, ydir, t_near, tx, ty ;
double	tgcx, tgcy ;
int	gcx, gcy, sign_x ;
int	y_flag, io_state ;

    p_gs->xres = p_gs->yres = resolution ;
    p_gs->tot_cells = p_gs->xres * p_gs->yres ;
    p_gs->glx = (double *)malloc( (p_gs->xres+1) * sizeof(double));
    MALLOC_CHECK( p_gs->glx ) ;
    p_gs->gly = (double *)malloc( (p_gs->yres+1) * sizeof(double));
    MALLOC_CHECK( p_gs->gly ) ;
    p_gs->gc = (pGridCell)malloc( p_gs->tot_cells * sizeof(GridCell));
    MALLOC_CHECK( p_gs->gc ) ;

    p_gs->minx =
    p_gs->maxx = pgon[0]->x ;
    p_gs->miny =
    p_gs->maxy = pgon[0]->y ;

    /* find bounds of polygon */
    for ( i = 1 ; i < numverts ; i++ ) {
	vx0 = pgon[i]->x ;
	if ( p_gs->minx > vx0 ) {
	    p_gs->minx = vx0 ;
	} else if ( p_gs->maxx < vx0 ) {
	    p_gs->maxx = vx0 ;
	}

	vy0 = pgon[i]->y ;
	if ( p_gs->miny > vy0 ) {
	    p_gs->miny = vy0 ;
	} else if ( p_gs->maxy < vy0 ) {
	    p_gs->maxy = vy0 ;
	}
    }

    /* add a little to the bounds to ensure everything falls inside area */
    gxdiff = p_gs->maxx - p_gs->minx ;
    gydiff = p_gs->maxy - p_gs->miny ;
    p_gs->minx -= EPSILON * gxdiff ;
    p_gs->maxx += EPSILON * gxdiff ;
    p_gs->miny -= EPSILON * gydiff ;
    p_gs->maxy += EPSILON * gydiff ;

    /* avoid roundoff problems near corners by not getting too close to them */
    eps = 1e-9 * ( gxdiff + gydiff ) ;

    /* use the new bounds to compute cell widths */
    TryAgain:
    p_gs->xdelta =
	    (p_gs->maxx-p_gs->minx) / (double)p_gs->xres ;
    p_gs->inv_xdelta = 1.0 / p_gs->xdelta ;

    p_gs->ydelta =
	    (p_gs->maxy-p_gs->miny) / (double)p_gs->yres ;
    p_gs->inv_ydelta = 1.0 / p_gs->ydelta ;

    for ( i = 0, p_gl = p_gs->glx ; i < p_gs->xres ; i++ ) {
	*p_gl++ = p_gs->minx + i * p_gs->xdelta ;
    }
    /* make last grid corner precisely correct */
    *p_gl = p_gs->maxx ;

    for ( i = 0, p_gl = p_gs->gly ; i < p_gs->yres ; i++ ) {
	*p_gl++ = p_gs->miny + i * p_gs->ydelta ;
    }
    *p_gl = p_gs->maxy ;

    for ( i = 0, p_gc = p_gs->gc ; i < p_gs->tot_cells ; i++, p_gc++ ) {
	p_gc->tot_edges = 0 ;
	p_gc->gc_flags = 0x0 ;
	p_gc->gr = NULL ;
    }

    /* loop through edges and insert into grid structure */
    double pt[2] = { pgon[numverts - 1]->x,pgon[numverts - 1]->y } ;
    vtx0 = &pt[0] ;
    for ( i = 0 ; i < numverts ; i++ ) {
      double pt1[2] = { pgon[i]->x, pgon[i]->y } ;
      vtx1 = &pt1[0] ;

	if ( vtx0[Y] < vtx1[Y] ) {
	    vtxa = vtx0 ;
	    vtxb = vtx1 ;
	} else {
	    vtxa = vtx1 ;
	    vtxb = vtx0 ;
	}

	/* Set x variable for the direction of the ray */
	xdiff = vtxb[X] - vtxa[X] ;
	ydiff = vtxb[Y] - vtxa[Y] ;
	tmax = sqrt( xdiff * xdiff + ydiff * ydiff ) ;

	/* if edge is of 0 length, ignore it (useless edge) */
	if ( tmax == 0.0 ) goto NextEdge ;

	xdir = xdiff / tmax ;
	ydir = ydiff / tmax ;

	gcx = (int)(( vtxa[X] - p_gs->minx ) * p_gs->inv_xdelta) ;
	gcy = (int)(( vtxa[Y] - p_gs->miny ) * p_gs->inv_ydelta) ;

	/* get information about slopes of edge, etc */
	if ( vtxa[X] == vtxb[X] ) {
	    sign_x = 0 ;
	    tx = HUGE ;
	} else {
	    inv_x = tmax / xdiff ;
	    tx = p_gs->xdelta * (double)gcx + p_gs->minx - vtxa[X] ;
	    if ( vtxa[X] < vtxb[X] ) {
		sign_x = 1 ;
		tx += p_gs->xdelta ;
		tgcx = p_gs->xdelta * inv_x ;
	    } else {
		sign_x = -1 ;
		tgcx = -p_gs->xdelta * inv_x ;
	    }
	    tx *= inv_x ;
	}

	if ( vtxa[Y] == vtxb[Y] ) {
	    ty = HUGE ;
	} else {
	    inv_y = tmax / ydiff ;
	    ty = (p_gs->ydelta * (double)(gcy+1) + p_gs->miny - vtxa[Y])
		* inv_y ;
	    tgcy = p_gs->ydelta * inv_y ;
	}

	p_gc = &p_gs->gc[gcy*p_gs->xres+gcx] ;

	vx0 = vtxa[X] ;
	vy0 = vtxa[Y] ;

	t_near = 0.0 ;

	do {
	    /* choose the next boundary, but don't move yet */
	    if ( tx <= ty ) {
		gcx += sign_x ;

		ty -= tx ;
		t_near += tx ;
		tx = tgcx ;

		/* note which edge is hit when leaving this cell */
		if ( t_near < tmax ) {
		    if ( sign_x > 0 ) {
			p_gc->gc_flags |= GC_R_EDGE_HIT ;
			vx1 = p_gs->glx[gcx] ;
		    } else {
			p_gc->gc_flags |= GC_L_EDGE_HIT ;
			vx1 = p_gs->glx[gcx+1] ;
		    }

		    /* get new location */
		    vy1 = t_near * ydir + vtxa[Y] ;
		} else {
		    /* end of edge, so get exact value */
		    vx1 = vtxb[X] ;
		    vy1 = vtxb[Y] ;
		}

		y_flag = FALSE ;

	    } else {

		gcy++ ;

		tx -= ty ;
		t_near += ty ;
		ty = tgcy ;

		/* note top edge is hit when leaving this cell */
		if ( t_near < tmax ) {
		    p_gc->gc_flags |= GC_T_EDGE_HIT ;
		    /* this toggles the parity bit */
		    p_gc->gc_flags ^= GC_T_EDGE_PARITY ;

		    /* get new location */
		    vx1 = t_near * xdir + vtxa[X] ;
		    vy1 = p_gs->gly[gcy] ;
		} else {
		    /* end of edge, so get exact value */
		    vx1 = vtxb[X] ;
		    vy1 = vtxb[Y] ;
		}

		y_flag = TRUE ;
	    }

	    /* check for corner crossing, then mark the cell we're in */
	    if ( !AddGridRecAlloc( p_gc, vx0, vy0, vx1, vy1, eps ) ) {
		/* warning, danger - we have just crossed a corner.
		 * There are all kinds of topological messiness we could
		 * do to get around this case, but they're a headache.
		 * The simplest recovery is just to change the extents a bit
		 * and redo the meshing, so that hopefully no edges will
		 * perfectly cross a corner.  Since it's a preprocess, we
		 * don't care too much about the time to do it.
		 */

		/* clean out all grid records */
		for ( i = 0, p_gc = p_gs->gc
		    ; i < p_gs->tot_cells
		    ; i++, p_gc++ ) {

		    if ( p_gc->gr ) {
			free( p_gc->gr ) ;
		    }
		}

		/* make the bounding box ever so slightly larger, hopefully
		 * changing the alignment of the corners.
		 */
		p_gs->minx -= EPSILON * gxdiff * 0.24 ;
		p_gs->miny -= EPSILON * gydiff * 0.10 ;

		/* yes, it's the dreaded goto - run in fear for your lives! */
		goto TryAgain ;
	    }

	    if ( t_near < tmax ) {
		/* note how we're entering the next cell */
		/* TBD: could be done faster by incrementing index in the
		 * incrementing code, above */
		p_gc = &p_gs->gc[gcy*p_gs->xres+gcx] ;

		if ( y_flag ) {
		    p_gc->gc_flags |= GC_B_EDGE_HIT ;
		    /* this toggles the parity bit */
		    p_gc->gc_flags ^= GC_B_EDGE_PARITY ;
		} else {
		    p_gc->gc_flags |=
			( sign_x > 0 ) ? GC_L_EDGE_HIT : GC_R_EDGE_HIT ;
		}
	    }

	    vx0 = vx1 ;
	    vy0 = vy1 ;
	}
	/* have we gone further than the end of the edge? */
	while ( t_near < tmax ) ;

	NextEdge:
	vtx0 = vtx1 ;
    }

    /* the grid is all set up, now set up the inside/outside value of each
     * corner.
     */
    p_gc = p_gs->gc ;
    p_ngc = &p_gs->gc[p_gs->xres] ;

    /* we know the bottom and top rows are all outside, so no flag is set */
    for ( i = 1; i < p_gs->yres ; i++ ) {
	/* start outside */
	io_state = 0x0 ;

	for ( j = 0; j < p_gs->xres ; j++ ) {

	    if ( io_state ) {
		/* change cell left corners to inside */
		p_gc->gc_flags |= GC_TL_IN ;
		p_ngc->gc_flags |= GC_BL_IN ;
	    }

	    if ( p_gc->gc_flags & GC_T_EDGE_PARITY ) {
		io_state = !io_state ;
	    }

	    if ( io_state ) {
		/* change cell right corners to inside */
		p_gc->gc_flags |= GC_TR_IN ;
		p_ngc->gc_flags |= GC_BR_IN ;
	    }

	    p_gc++ ;
	    p_ngc++ ;
	}
    }

    p_gc = p_gs->gc ;
    for ( i = 0; i < p_gs->tot_cells ; i++ ) {

	/* reverse parity of edge clear (1==edge clear) */
	gc_clear_flags = p_gc->gc_flags ^ GC_ALL_EDGE_CLEAR ;
	if ( gc_clear_flags & GC_L_EDGE_CLEAR ) {
	    p_gc->gc_flags |= GC_AIM_L ;
	} else
	if ( gc_clear_flags & GC_B_EDGE_CLEAR ) {
	    p_gc->gc_flags |= GC_AIM_B ;
	} else
	if ( gc_clear_flags & GC_R_EDGE_CLEAR ) {
	    p_gc->gc_flags |= GC_AIM_R ;
	} else
	if ( gc_clear_flags & GC_T_EDGE_CLEAR ) {
	    p_gc->gc_flags |= GC_AIM_T ;
	} else {
	    /* all edges have something on them, do full test */
	    p_gc->gc_flags |= GC_AIM_C ;
	}
	p_gc++ ;
    }
}

int AddGridRecAlloc(pGridCell p_gc, double xa, double ya, double xb, double yb, double eps )
{
pGridRec	p_gr ;
double		slope, inv_slope ;

    if ( Near(ya, yb, eps) ) {
	if ( Near(xa, xb, eps) ) {
	    /* edge is 0 length, so get rid of it */
	    return( FALSE ) ;
	} else {
	    /* horizontal line */
	    slope = HUGE ;
	    inv_slope = 0.0 ;
	}
    } else {
	if ( Near(xa, xb, eps) ) {
	    /* vertical line */
	    slope = 0.0 ;
	    inv_slope = HUGE ;
	} else {
	    slope = (xb-xa)/(yb-ya) ;
	    inv_slope = (yb-ya)/(xb-xa) ;
	}
    }

    p_gc->tot_edges++ ;
    if ( p_gc->tot_edges <= 1 ) {
	p_gc->gr = (pGridRec)malloc( sizeof(GridRec) ) ;
    } else {
	p_gc->gr = (pGridRec)realloc( p_gc->gr,
		p_gc->tot_edges * sizeof(GridRec) ) ;
    }
    MALLOC_CHECK( p_gc->gr ) ;
    p_gr = &p_gc->gr[p_gc->tot_edges-1] ;

    p_gr->slope = slope ;
    p_gr->inv_slope = inv_slope ;

    p_gr->xa = xa ;
    p_gr->ya = ya ;
    if ( xa <= xb ) {
	p_gr->minx = xa ;
	p_gr->maxx = xb ;
    } else {
	p_gr->minx = xb ;
	p_gr->maxx = xa ;
    }
    if ( ya <= yb ) {
	p_gr->miny = ya ;
	p_gr->maxy = yb ;
    } else {
	p_gr->miny = yb ;
	p_gr->maxy = ya ;
    }

    /* P2 - P1 */
    p_gr->ax = xb - xa ;
    p_gr->ay = yb - ya ;

    return( TRUE ) ;
}

/* Test point against grid and edges in the cell (if any).  Algorithm:
 *    Check bounding box; if outside then return.
 *    Check cell point is inside; if simple inside or outside then return.
 *    Find which edge or corner is considered to be the best for testing and
 *	  send a test ray towards it, counting the crossings.  Add in the
 *	  state of the edge or corner the ray went to and so determine the
 *	  state of the point (inside or outside).
 */
int GridTest(pGridSet p_gs, pPipoint point)
{
int	j, count, init_flag ;
pGridCell	p_gc ;
pGridRec	p_gr ;
double	tx, ty, xcell, ycell, bx,by,cx,cy, cornerx, cornery ;
double	alpha, beta, denom ;
unsigned short	gc_flags ;
int	inside_flag ;

    /* first, is point inside bounding rectangle? */
    if ( ( ty = point->y ) < p_gs->miny ||
	 ty >= p_gs->maxy ||
	 ( tx = point->x ) < p_gs->minx ||
	 tx >= p_gs->maxx ) {

	/* outside of box */
	inside_flag = FALSE ;
    } else {

	/* what cell are we in? */
	ycell = ( ty - p_gs->miny ) * p_gs->inv_ydelta ;
	xcell = ( tx - p_gs->minx ) * p_gs->inv_xdelta ;
	p_gc = &p_gs->gc[((int)ycell)*p_gs->xres + (int)xcell] ;

	/* is cell simple? */
	count = p_gc->tot_edges ;
	if ( count ) {
	    /* no, so find an edge which is free. */
	    gc_flags = p_gc->gc_flags ;
	    p_gr = p_gc->gr ;
	    switch( gc_flags & GC_AIM ) {
	    case GC_AIM_L:
		/* left edge is clear, shoot X- ray */
		/* note - this next statement requires that GC_BL_IN is 1 */
		inside_flag = gc_flags & GC_BL_IN ;
		for ( j = count+1 ; --j ; p_gr++ ) {
		    /* test if y is between edges */
		    if ( ty >= p_gr->miny && ty < p_gr->maxy ) {
			if ( tx > p_gr->maxx ) {
			    inside_flag = !inside_flag ;
			} else if ( tx > p_gr->minx ) {
			    /* full computation */
			    if ( ( p_gr->xa -
				( p_gr->ya - ty ) * p_gr->slope ) < tx ) {
				inside_flag = !inside_flag ;
			    }
			}
		    }
		}
		break ;

	    case GC_AIM_B:
		/* bottom edge is clear, shoot Y+ ray */
		/* note - this next statement requires that GC_BL_IN is 1 */
		inside_flag = gc_flags & GC_BL_IN ;
		for ( j = count+1 ; --j ; p_gr++ ) {
		    /* test if x is between edges */
		    if ( tx >= p_gr->minx && tx < p_gr->maxx ) {
			if ( ty > p_gr->maxy ) {
			    inside_flag = !inside_flag ;
			} else if ( ty > p_gr->miny ) {
			    /* full computation */
			    if ( ( p_gr->ya - ( p_gr->xa - tx ) *
				    p_gr->inv_slope ) < ty ) {
				inside_flag = !inside_flag ;
			    }
			}
		    }
		}
		break ;

	    case GC_AIM_R:
		/* right edge is clear, shoot X+ ray */
		inside_flag = (gc_flags & GC_TR_IN) ? 1 : 0 ;

		/* TBD: Note, we could have sorted the edges to be tested
		 * by miny or somesuch, and so be able to cut testing
		 * short when the list's miny > point.y .
		 */
		for ( j = count+1 ; --j ; p_gr++ ) {
		    /* test if y is between edges */
		    if ( ty >= p_gr->miny && ty < p_gr->maxy ) {
			if ( tx <= p_gr->minx ) {
			    inside_flag = !inside_flag ;
			} else if ( tx <= p_gr->maxx ) {
			    /* full computation */
			    if ( ( p_gr->xa -
				( p_gr->ya - ty ) * p_gr->slope ) >= tx ) {
				inside_flag = !inside_flag ;
			    }
			}
		    }
		}
		break ;

	    case GC_AIM_T:
		/* top edge is clear, shoot Y+ ray */
		inside_flag = (gc_flags & GC_TR_IN) ? 1 : 0 ;
		for ( j = count+1 ; --j ; p_gr++ ) {
		    /* test if x is between edges */
		    if ( tx >= p_gr->minx && tx < p_gr->maxx ) {
			if ( ty <= p_gr->miny ) {
			    inside_flag = !inside_flag ;
			} else if ( ty <= p_gr->maxy ) {
			    /* full computation */
			    if ( ( p_gr->ya - ( p_gr->xa - tx ) *
				    p_gr->inv_slope ) >= ty ) {
				inside_flag = !inside_flag ;
			    }
			}
		    }
		}
		break ;

	    case GC_AIM_C:
		/* no edge is clear, bite the bullet and test
		 * against the bottom left corner.
		 * We use Franklin Antonio's algorithm (Graphics Gems III).
		 */
		/* TBD: Faster yet might be to test against the closest
		 * corner to the cell location, but our hope is that we
		 * rarely need to do this testing at all.
		 */
		inside_flag = ((gc_flags & GC_BL_IN) == GC_BL_IN) ;
		init_flag = TRUE ;

		/* get lower left corner coordinate */
		cornerx = p_gs->glx[(int)xcell] ;
		cornery = p_gs->gly[(int)ycell] ;
		for ( j = count+1 ; --j ; p_gr++ ) {

		    /* quick out test: if test point is
		     * less than minx & miny, edge cannot overlap.
		     */
		    if ( tx >= p_gr->minx && ty >= p_gr->miny ) {

			/* quick test failed, now check if test point and
			 * corner are on different sides of edge.
			 */
			if ( init_flag ) {
			    /* Compute these at most once for test */
			    /* P3 - P4 */
			    bx = tx - cornerx ;
			    by = ty - cornery ;
			    init_flag = FALSE ;
			}
			denom = p_gr->ay * bx - p_gr->ax * by ;
			if ( denom != 0.0 ) {
			    /* lines are not collinear, so continue */
			    /* P1 - P3 */
			    cx = p_gr->xa - tx ;
			    cy = p_gr->ya - ty ;
			    alpha = by * cx - bx * cy ;
			    if ( denom > 0.0 ) {
				if ( alpha < 0.0 || alpha >= denom ) {
				    /* test edge not hit */
				    goto NextEdge ;
				}
				beta = p_gr->ax * cy - p_gr->ay * cx ;
				if ( beta < 0.0 || beta >= denom ) {
				    /* polygon edge not hit */
				    goto NextEdge ;
				}
			    } else {
				if ( alpha > 0.0 || alpha <= denom ) {
				    /* test edge not hit */
				    goto NextEdge ;
				}
				beta = p_gr->ax * cy - p_gr->ay * cx ;
				if ( beta > 0.0 || beta <= denom ) {
				    /* polygon edge not hit */
				    goto NextEdge ;
				}
			    }
			    inside_flag = !inside_flag ;
			}

		    }
		    NextEdge: ;
		}
		break ;
	    }
	} else {
	    /* simple cell, so if lower left corner is in,
	     * then cell is inside.
	     */
	    inside_flag = p_gc->gc_flags & GC_BL_IN ;
	}
    }

    return( inside_flag ) ;
}

void GridCleanup(pGridSet	p_gs)
{
int	i ;
pGridCell	p_gc ;

    for ( i = 0, p_gc = p_gs->gc
	; i < p_gs->tot_cells
	; i++, p_gc++ ) {

	if ( p_gc->gr ) {
	    free( p_gc->gr ) ;
	}
    }
    free( p_gs->glx ) ;
    free( p_gs->gly ) ;
    free( p_gs->gc ) ;
}
