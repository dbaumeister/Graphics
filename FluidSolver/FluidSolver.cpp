//
// Created by dbaumeister on 02.06.15.
//

#include <iostream>
#include "FluidSolver.h"

void FluidSolver::applyForces() {
    Vector3D gravity(0, 0.5f, 0);

    for(int i = 1; i < scene.getDimX() - 1; ++i){
        for(int j = 1; j < scene.getDimY() - 1; ++j){
                scene.vel(i,j) += (scene.getDt() * gravity);
        }
    }
}

void FluidSolver::advectVelocities(){
    VectorGrid advVel(scene.getDimX(), scene.getDimY());

    for (int j = 1; j < scene.getDimY() - 1; ++j) {
        for (int i = 1; i < scene.getDimX() - 1; ++i) {

            Real ib = i - scene.getDt() * (scene.vel(i + 1, j).x + scene.vel(i, j).x) / 2.f;
            Real jb = j - scene.getDt() * (scene.vel(i, j + 1).y + scene.vel(i, j).y) / 2.f;

            if(ib > 0 && ib < scene.getDimX() - 1 && jb > 0 && jb < scene.getDimY() - 1){

                Real ri = ib - floorf(ib);
                Real rj = jb - floorf(jb);

                Vector3D d00 = (1.f - rj) * (1.f - ri) * scene.vel((int)ib, (int)jb);
                Vector3D d10 = (1.f - rj) * ri * scene.vel((int)ib + 1, (int)jb);
                Vector3D d01 = rj * (1.f - ri) * scene.vel((int)ib, (int)jb + 1);
                Vector3D d11 = rj * ri * scene.vel((int)ib + 1, (int)jb + 1);

                advVel(i, j) = d00 + d10 + d11 + d01;

            } else advVel(i, j) = 0;

        }
    }
    scene.vel.set(advVel);
}

void FluidSolver::computeDivergence(RealGrid& div) {
    for (int j = 1; j < scene.getDimY() - 1; ++j) {
        for (int i = 1; i < scene.getDimX() - 1; ++i) {
            div.at(i, j) = scene.vel(i + 1, j).x - scene.vel(i, j).x
                                                   + scene.vel(i, j + 1).y - scene.vel(i, j).y;
        }
    }

}

void FluidSolver::computeOffDiag(RealGrid& offdiag){

    for (int j = 1; j < scene.getDimY() - 1; ++j) {
        for (int i = 1; i < scene.getDimX() - 1; ++i) {
            offdiag.at(i, j) = -1.f;
        }
    }
}

void FluidSolver::computeDiag(RealGrid& diag, RealGrid& offdiag){
    for (int j = 1; j < scene.getDimY() - 1; ++j) {
        for (int i = 1; i < scene.getDimX() - 1; ++i) {
            diag.at(i, j) = -1.f * (offdiag(i - 1, j) + offdiag(i + 1, j) + offdiag(i, j - 1) + offdiag(i, j + 1));
        }
    }
}

Real gridNorm(RealGrid& grid){
    return 0;
}

void applyIterativeStep(RealGrid& residual, RealGrid& divergence, RealGrid& diag, RealGrid& offdiag){

}

void FluidSolver::solvePressure(RealGrid& divergence, RealGrid& diag, RealGrid& offdiag){
    //use scene.pressure
    RealGrid residual(scene.getDimX(), scene.getDimY());

    for(int n = 0; n < iterations; ++n){
        applyIterativeStep(residual, divergence, diag, offdiag);
        if(accuracy > gridNorm(residual)) break;
    }
}

void FluidSolver::correctVelocity(){
    for (int j = 1; j < scene.getDimY() - 1; ++j) {
        for (int i = 1; i < scene.getDimX() - 1; ++i) {
            scene.vel(i, j).x -= scene.pressure(i + 1, j) - scene.pressure(i, j);
            scene.vel(i, j).y -= scene.pressure(i, j + 1) - scene.pressure(i, j);
        }
    }
}

void FluidSolver::advectDensity(){
    RealGrid advDen(scene.getDimX(), scene.getDimY());

    for (int j = 1; j < scene.getDimY() - 1; ++j) {
        for (int i = 1; i < scene.getDimX() - 1; ++i) {

            Real ib = i - scene.getDt() * (scene.vel(i + 1, j).x + scene.vel(i, j).x) / 2.f;
            Real jb = j - scene.getDt() * (scene.vel(i, j + 1).y + scene.vel(i, j).y) / 2.f;

            if(ib > 0 && ib < scene.getDimX() - 1 && jb > 0 && jb < scene.getDimY() - 1){

                Real ri = ib - floorf(ib);
                Real rj = jb - floorf(jb);

                Real d00 = (1.f - rj) * (1.f - ri) * scene.density((int)ib, (int)jb);
                Real d10 = (1.f - rj) * ri * scene.density((int)ib + 1, (int)jb);
                Real d01 = rj * (1.f - ri) * scene.density((int)ib, (int)jb + 1);
                Real d11 = rj * ri * scene.density((int)ib + 1, (int)jb + 1);

                advDen.at(i, j) = d00 + d10 + d11 + d01;

            } else advDen.at(i, j) = 0;

        }
    }

    scene.density.set(advDen);

}

void FluidSolver::next() {
    advectVelocities();
    applyForces();

    RealGrid divergence(scene.getDimX(), scene.getDimY());
    computeDivergence(divergence);

    RealGrid offdiag(scene.getDimX(), scene.getDimY());
    computeOffDiag(offdiag);

    RealGrid diag(scene.getDimX(), scene.getDimY());
    computeDiag(diag, offdiag);

    solvePressure(divergence, diag, offdiag);

    correctVelocity();
    advectDensity();
}
