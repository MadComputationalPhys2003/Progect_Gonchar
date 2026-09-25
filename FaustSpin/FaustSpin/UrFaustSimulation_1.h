#pragma once
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <variant>
#include "Urfaust.h"
enum class Dimension {
    one,
    two,
    three
};

enum class Initialization {
    parallel,
    random
};

struct UrFaustPseudoInterface {
    using System = std::variant<
        UrFaust1D::SpinSystem1D,
        UrFaust2D::SpinSystem2D,
        UrFaust3D::SpinSystem3D
    >;

    Dimension dim;
    Initialization initialization;

    uint64_t Lx, Ly, Lz;
    uint64_t x0, y0, z0;

    double S;
    double J;
    double eps;

    System system;

    UrFaustPseudoInterface(
        Dimension ddim,
        uint64_t lx, uint64_t ly, uint64_t lz,
        uint64_t x_0, uint64_t y_0, uint64_t z_0,
        double s, double j, double e,
        Initialization init
    )
        : dim(ddim),
        initialization(init),
        Lx(lx), Ly(ly), Lz(lz),
        x0(x_0), y0(y_0), z0(z_0),
        S(s), J(j), eps(e),
        system(create_system(
            ddim,
            lx, ly, lz,
            x_0, y_0, z_0,
            s, j, e,
            init
        ))
    {
    }

    double energy() const {
        return std::visit(
            [](const auto& selected_system) {
                return selected_system.energy();
            },
            system
        );
    }

private:
    static System create_system(
        Dimension dim,
        uint64_t lx, uint64_t ly, uint64_t lz,
        uint64_t x0, uint64_t y0, uint64_t z0,
        double S, double J, double eps,
        Initialization initialization
    ) {
        if (dim == Dimension::one) {
            const auto mode =
                initialization == Initialization::parallel
                ? UrFaust1D::Ions::SpinInit::parallel
                : UrFaust1D::Ions::SpinInit::random;

            return UrFaust1D::SpinSystem1D(
                lx,
                x0,
                S, J, eps,
                mode
            );
        }

        if (dim == Dimension::two) {
            const auto mode =
                initialization == Initialization::parallel
                ? UrFaust2D::Ions::SpinInit::parallel
                : UrFaust2D::Ions::SpinInit::random;

            return UrFaust2D::SpinSystem2D(
                lx, ly,
                x0, y0,
                S, J, eps,
                mode
            );
        }

        if (dim == Dimension::three) {
            const auto mode =
                initialization == Initialization::parallel
                ? UrFaust3D::Ions::SpinInit::parallel
                : UrFaust3D::Ions::SpinInit::random;

            return UrFaust3D::SpinSystem3D(
                lx, ly, lz,
                x0, y0, z0,
                S, J, eps,
                mode
            );
        }

        throw std::invalid_argument("Unknown lattice dimension");
    }
};
