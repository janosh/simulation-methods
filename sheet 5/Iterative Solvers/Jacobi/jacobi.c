#include <stdio.h>
#include <stdlib.h>
#include <math.h>


// modulo function to implement periodic boundaries on the grid
int mod(int x, int N) {
    x = x % N;
    return (x < 0) ? (x + N) : x;
}


// This function improves the solution Ax = b with one Jacobi step, with the result overwriting phi. The matrix A is hardcoded and represents the Poisson equation.
void jacobi_step(double *phi, double *rho, int N, double h, double G)
{
    double *phinew = calloc(N * N, sizeof(double));

    for (int i = 0; i < N; i++)
    for (int j = 0; j < N; j++) {
        phinew[i * N + j] = (phi[mod(i - 1, N) * N + j] + phi[mod(i + 1, N) * N + j] + phi[i * N + mod(j - 1, N)] + phi[i * N + mod(j + 1, N)] - 4 * M_PI * G * h * h * rho[i * N + j])/4;
    }

    for(int i = 0; i < N; i++)
    for(int j = 0; j < N; j++)
    phi[i * N + j] = phinew[i * N + j];

    free(phinew);
}


// This function calculates the resdiuum vector res = b - Aphi, for input vectors of length N. The output is stored in res.
void calc_residual(double *phi, double *rho, int N, double *res, double h, double G)
{
    for(int i = 0; i < N; i++)
    for(int j = 0; j < N; j++)
    {
        res[i * N + j] = 4 * M_PI * G * h * h * rho[i * N + j] - (phi[mod(i + 1, N) * N + j] + phi[mod(i - 1, N) * N + j] + phi[i * N + mod(j + 1, N)] + phi[i * N + mod(j - 1, N)] - 4 * phi[i * N + j]);
    }
}

// This function calculates the norm of the vector of length N, defined as the usual quadratic vector norm.
double norm_of_residual(double *res, int N)
{
    double res_sum = 0;

    for(int i = 0; i < N; i++)
    for(int j = 0; j < N; j++)
    res_sum += res[i * N + j] * res[i * N + j];

    return sqrt(res_sum);
}


int main(int argc, char **argv)
{
    const int N = 256, Nsteps = 2000;
    const double L = 1.0, h = L / N, G = 1.0, eta = L/10, rho0 = 10.0;

    // allocate some storage for the potential, density, PE's r.h.s., and the residual
    double *phi = calloc(N * N, sizeof(double));
    double *rho = calloc(N * N, sizeof(double));
    double *res = calloc(N * N, sizeof(double));

    // now set-up the density field
    double rho_sum = 0;
    for(int i = 0; i < N; i++)
    for(int j = 0; j < N; j++)
    {
        double dx = (i - N / 2 + 0.5) * h, dy = (j - N / 2 + 0.5) * h;
        double r2 = dx * dx + dy * dy;

        rho[i * N + j] = rho0 * exp(-r2 / (2 * eta * eta));
    }

    // open a file for outputting the residual values, and then do 2000 Jacobi steps
    FILE *datafile = fopen("res_jacobi.txt", "w");

    printf("\nJacobi\n");
    for(int i = 0; i <= Nsteps; i++)
    {
        jacobi_step(phi, rho, N, h, G);
        calc_residual(phi, rho, N, res, h, G);
        double S = norm_of_residual(res, N);

        if (10 * i % Nsteps == 0) {
            printf("step: %4d\t\tresidual = %g\n", i, S);
        }
        fprintf(datafile, "\n%g", S);
    }

    FILE *phi_data = fopen("phi_jac.dat", "w");
    double phi_sum = 0;
    for(int i = 0; i < N; i++)
    for(int j = 0; j < N; j++) {
        phi_sum += phi[i * N + j];
        fwrite(&phi[i * N + j], sizeof(double), 1, phi_data);
    }
    printf("phi_sum = %g\n", phi_sum);
}
