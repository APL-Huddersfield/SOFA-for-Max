//
//  PointTree.h
//  SofaMax2
//
//  Created by Dale Johnson on 25/01/2019.
//  Copyright © 2019 Dale Johnson. All rights reserved.
//

#ifndef PointTree_h
#define PointTree_h
#include <stdint.h>
#include "kdtree.h"

typedef struct _point {
    uint64_t ID;
    double pos[3];
} t_point;

t_point make2DCartesianPoint(uint64_t ID, double x, double y);
t_point make3DCartesianPoint(uint64_t ID, double x, double y, double z);
t_point makeSphericalPoint(uint64_t ID, double azi, double elev);

t_point* getNearestPoint(struct kdtree* tree, t_point* reqPoint);

#endif /* SofaMax_h */
