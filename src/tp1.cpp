#include "orbiter.h"
#include "color.h"       // couleur
#include "image.h"       // image
#include "image_io.h"    // entrees / sorties sur les images
#include "mat.h"
#include "vec.h"
#include <iostream>
#include <vector>

Image image(512, 512);
float ZBuffer[512][512][2];


class Triangle{
public:
    Triangle(){};
    Triangle(Point a, Point b, Point c){p1 = a; p2 = b; p3 = c;}
    Point p1;
    Point p2;
    Point p3;
};

//bool fin(std::vector<Triangle>& vec){
//    if(distance(vec.back().p1, vec.back().p2) <= 1)
//        return true;
//    return false;
//}

//void subDivise(std::vector<Triangle>& vec, int i){
//    Point midp1p2 = center(vec[i].p1, vec[i].p2);
//    Point midp2p3 = center(vec[i].p2, vec[i].p3);
//    Point midp1p3 = center(vec[i].p1, vec[i].p3);
//
//    vec.push_back(Triangle(vec[i].p1, midp1p2, midp1p3));
//    vec.push_back(Triangle(midp1p2, vec[i].p2, midp2p3));
//    vec.push_back(Triangle(vec[i].p3, midp1p3, midp2p3));
//    vec.push_back(Triangle(midp1p2, midp2p3, midp1p3));
//}
//
//void divise(std::vector<Triangle> vec){
//    int i = 0;
//
//    while(!fin(vec)){
//        subDivise(vec, i);
//        i++;
//    }
//    std::cout << vec.back().p1 << " " << vec.back().p2 << " " << vec.back().p3 << std::endl;
//}

void diviseRec(int id, Triangle& tri, const Transform& V, const Transform& P, Color color){
    Transform M = Translation((tri.p1.x + tri.p2.x + tri.p3.x) / 3, (tri.p1.y + tri.p2.y + tri.p3.y) / 3, (tri.p1.z + tri.p2.z + tri.p3.z) / 3);
    Transform viewPort = P * V * M;
    Point p = viewPort(Point(0,0));
    if(p.x > 1 || p.x < -1 || p.y < -1 || p.y > 1){
//        std::cout << M(Point(0,0)) << std::endl;
//        std::cout << p << std::endl;
        return;
    }
    if(distance(tri.p1, tri.p2) <= 1){
        if(ZBuffer[(int)tri.p1.x][(int)tri.p2.y][0] != -1){
            //cas plus proche
            if(ZBuffer[(int)tri.p1.x][(int)tri.p2.y][1] > p.z){
                ZBuffer[(int)tri.p1.x][(int)tri.p2.y][0] = id;
                ZBuffer[(int)tri.p1.x][(int)tri.p2.y][1] = p.z;
                int minX = std::min(tri.p1.x, std::min(tri.p2.x, tri.p3.x));
                int maxX = std::max(tri.p1.x, std::max(tri.p2.x, tri.p3.x));

                int minY = std::min(tri.p1.y, std::min(tri.p2.y, tri.p3.y));
                int maxY = std::max(tri.p1.y, std::max(tri.p2.y, tri.p3.y));

                for(int i = minX; i <= maxX; i++){
                    for(int j = minY; j <= maxY; j++) {
                        if(i >= 0 && i < 512 && j >= 0 && j < 512)
                            image(i, j) = color;
                    }
                }

            }
        }
        else{
            ZBuffer[(int)tri.p1.x][(int)tri.p2.y][0] = id;
            ZBuffer[(int)tri.p1.x][(int)tri.p2.y][1] = p.z;

            int minX = std::min(tri.p1.x, std::min(tri.p2.x, tri.p3.x));
            int maxX = std::max(tri.p1.x, std::max(tri.p2.x, tri.p3.x));

            int minY = std::min(tri.p1.y, std::min(tri.p2.y, tri.p3.y));
            int maxY = std::max(tri.p1.y, std::max(tri.p2.y, tri.p3.y));

            for(int i = minX; i <= maxX; i++){
                for(int j = minY; j <= maxY; j++) {
                    if(i >= 0 && i < 512 && j >= 0 && j < 512)
                        image(i, j) = color;
                }
            }
        }

        return;
    }
    Point midp1p2 = center(tri.p1, tri.p2);
    Point midp2p3 = center(tri.p2, tri.p3);
    Point midp1p3 = center(tri.p1, tri.p3);

    Triangle t1 = Triangle(tri.p1, midp1p2, midp1p3);
    Triangle t2 = Triangle(midp1p2, tri.p2, midp2p3);
    Triangle t3 = Triangle(tri.p3, midp1p3, midp2p3);
    Triangle t4 = Triangle(midp1p2, midp2p3, midp1p3);

    diviseRec(id, t1, V, P, color);
    diviseRec(id, t2, V, P, color);
    diviseRec(id, t3, V, P, color);
    diviseRec(id, t4, V, P, color);
}

int tp1( )
{
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////INIT////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
    Orbiter cam;
    cam.lookat(Point(0,0), Point(512,512));

    Triangle triangle(Point(0,512,0), Point(512,512,0), Point(256,0,0));
    Triangle triangle2(Point(0,0,1), Point(512,0,1), Point(256,512,1));
    Triangle triangle3(Point(0,512,2), Point(0,0,2), Point(512,256,2));
    //Triangle triangle4(Point(512,512,3), Point(0,256,3), Point(512,0,3));

    Transform M = Identity();
    Transform V = cam.view();
    Transform P = cam.projection(512, 512, 45);

//    triangle3.p1 = M(triangle.p1);
//    triangle3.p2 = M(triangle.p2);
//    triangle3.p3 = M(triangle.p3);

    for(int i = 0; i < 512; i++){
        for(int j = 0; j < 512; j++){
            ZBuffer[i][j][0] = -1;
        }
    }
//////////////////////////////////////////////////////////////////////////////////////////////
    diviseRec(1, triangle2, V, P, Color(1.f,0.f,0.f,1.f));
    diviseRec(0, triangle, V, P, Color(0.f,1.f,0.f,1.f));
    diviseRec(2, triangle3, V, P, Color(0.f,0.f,1.f,1.f));
    //diviseRec(3, triangle4, V, P, Color(0.f,1.f,1.f,1.f));

    write_image(image, "out.bmp");
    return 0;
}
