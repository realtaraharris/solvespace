#include "Solver.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "solvespace.h"

static Slvs_System sys;

static void *CheckMalloc(size_t n)
{
    void *r = malloc(n);
    if(!r) {
        printf("out of memory!\n");
        exit(-1);
    }
    return r;
}

void Example3d()
{
    /* This will contain a single group, which will arbitrarily number 1. */
    Slvs_hGroup g = 1;

    /* A point, initially at (x y z) = (10 10 10) */
    sys.param[sys.params++] = Slvs_MakeParam(1, g, 10.0);
    sys.param[sys.params++] = Slvs_MakeParam(2, g, 10.0);
    sys.param[sys.params++] = Slvs_MakeParam(3, g, 10.0);
    sys.entity[sys.entities++] = Slvs_MakePoint3d(101, g, 1, 2, 3);
    /* and a second point at (20 20 20) */
    sys.param[sys.params++] = Slvs_MakeParam(4, g, 20.0);
    sys.param[sys.params++] = Slvs_MakeParam(5, g, 20.0);
    sys.param[sys.params++] = Slvs_MakeParam(6, g, 20.0);
    sys.entity[sys.entities++] = Slvs_MakePoint3d(102, g, 4, 5, 6);
    /* and a line segment connecting them. */
    sys.entity[sys.entities++] = Slvs_MakeLineSegment(200, g,
                                        SLVS_FREE_IN_3D, 101, 102);

    /* The distance between the points should be 30.0 units. */
    sys.constraint[sys.constraints++] = Slvs_MakeConstraint(
                                            1, g,
                                            SLVS_C_PT_PT_DISTANCE,
                                            SLVS_FREE_IN_3D,
                                            30.0,
                                            101, 102, 0, 0);

    /* Let's tell the solver to keep the second point as close to constant
     * as possible, instead moving the first point. */
    sys.dragged[0] = 4;
    sys.dragged[1] = 5;
    sys.dragged[2] = 6;

    /* Now that we have written our system, we solve. */
    Slvs_Solve(&sys, g);

    if(sys.result == SLVS_RESULT_OKAY) {
        printf("okay; now at (%.3f %.3f %.3f)\n"
               "             (%.3f %.3f %.3f)\n",
                sys.param[0].val, sys.param[1].val, sys.param[2].val,
                sys.param[3].val, sys.param[4].val, sys.param[5].val);
        printf("%d DOF\n", sys.dof);
    } else {
        printf("solve failed");
    }
}

int doIt()
{
    sys.param      = CheckMalloc(50*sizeof(sys.param[0]));
    sys.entity     = CheckMalloc(50*sizeof(sys.entity[0]));
    sys.constraint = CheckMalloc(50*sizeof(sys.constraint[0]));

    sys.failed  = CheckMalloc(50*sizeof(sys.failed[0]));
    sys.faileds = 50;

    /*Example3d();*/
    for(;;) {
        Example3d();
        sys.params = sys.constraints = sys.entities = 0;
        break;
    }
    return 0;
}
