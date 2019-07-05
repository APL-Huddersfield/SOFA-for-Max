//
//  PointTree.c
//  SofaMax2
//
//  Created by Dale Johnson on 25/01/2019.
//  Copyright © 2019 Dale Johnson. All rights reserved.
//

#include "PointTree.h"

t_point make2DCartesianPoint(uint64_t ID, double x, double y) {
    t_point newPoint;
    newPoint.ID = ID;
    newPoint.pos[0] = x;
    newPoint.pos[1] = y;
    newPoint.pos[2] = 0.0;
    return newPoint;
}

t_point make3DCartesianPoint(uint64_t ID, double x, double y, double z) {
    t_point newPoint;
    newPoint.ID = ID;
    newPoint.pos[0] = x;
    newPoint.pos[1] = y;
    newPoint.pos[2] = z;
    return newPoint;
}

t_point makeSphericalPoint(uint64_t ID, double azi, double elev) {
    t_point newPoint;
    newPoint.ID = ID;
    newPoint.pos[0] = azi;
    newPoint.pos[1] = elev;
    newPoint.pos[2] = 1.0;
    return newPoint;
}

t_point* getNearestPoint(struct kdtree* tree, t_point* reqPoint) {
    struct kdres* k = kd_nearest(tree, reqPoint->pos);
    return (t_point*)kd_res_item_data(k);
}
