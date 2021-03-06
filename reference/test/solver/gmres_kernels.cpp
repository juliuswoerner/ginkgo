/*******************************<GINKGO LICENSE>******************************
Copyright 2017-2019

Karlsruhe Institute of Technology
Universitat Jaume I
University of Tennessee

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************<GINKGO LICENSE>*******************************/

#include <ginkgo/core/solver/gmres.hpp>


#include <gtest/gtest.h>


#include <core/test/utils/assertions.hpp>
#include <ginkgo/core/base/exception.hpp>
#include <ginkgo/core/base/executor.hpp>
#include <ginkgo/core/matrix/dense.hpp>
#include <ginkgo/core/preconditioner/jacobi.hpp>
#include <ginkgo/core/stop/combined.hpp>
#include <ginkgo/core/stop/iteration.hpp>
#include <ginkgo/core/stop/residual_norm_reduction.hpp>
#include <ginkgo/core/stop/time.hpp>


namespace {


class Gmres : public ::testing::Test {
protected:
    using Mtx = gko::matrix::Dense<>;
    Gmres()
        : exec(gko::ReferenceExecutor::create()),
          mtx(gko::initialize<Mtx>(
              {{1.0, 2.0, 3.0}, {3.0, 2.0, -1.0}, {0.0, -1.0, 2}}, exec)),
          gmres_factory(
              gko::solver::Gmres<>::build()
                  .with_criteria(
                      gko::stop::Iteration::build().with_max_iters(4u).on(exec),
                      gko::stop::Time::build()
                          .with_time_limit(std::chrono::seconds(6))
                          .on(exec),
                      gko::stop::ResidualNormReduction<>::build()
                          .with_reduction_factor(1e-15)
                          .on(exec))
                  .on(exec)),
          mtx_big(gko::initialize<Mtx>(
              {{2295.7, -764.8, 1166.5, 428.9, 291.7, -774.5},
               {2752.6, -1127.7, 1212.8, -299.1, 987.7, 786.8},
               {138.3, 78.2, 485.5, -899.9, 392.9, 1408.9},
               {-1907.1, 2106.6, 1026.0, 634.7, 194.6, -534.1},
               {-365.0, -715.8, 870.7, 67.5, 279.8, 1927.8},
               {-848.1, -280.5, -381.8, -187.1, 51.2, -176.2}},
              exec)),
          gmres_factory_big(
              gko::solver::Gmres<>::build()
                  .with_criteria(
                      gko::stop::Iteration::build().with_max_iters(100u).on(
                          exec),
                      gko::stop::ResidualNormReduction<>::build()
                          .with_reduction_factor(1e-15)
                          .on(exec))
                  .on(exec)),
          mtx_medium(
              gko::initialize<Mtx>({{-86.40, 153.30, -108.90, 8.60, -61.60},
                                    {7.70, -77.00, 3.30, -149.20, 74.80},
                                    {-121.40, 37.10, 55.30, -74.20, -19.20},
                                    {-111.40, -22.60, 110.10, -106.20, 88.90},
                                    {-0.70, 111.70, 154.40, 235.00, -76.50}},
                                   exec))
    {}

    std::shared_ptr<const gko::Executor> exec;
    std::shared_ptr<Mtx> mtx;
    std::shared_ptr<Mtx> mtx_medium;
    std::shared_ptr<Mtx> mtx_big;
    std::unique_ptr<gko::solver::Gmres<>::Factory> gmres_factory;
    std::unique_ptr<gko::solver::Gmres<>::Factory> gmres_factory_big;
};


TEST_F(Gmres, SolvesStencilSystem)
{
    auto solver = gmres_factory->generate(mtx);
    auto b = gko::initialize<Mtx>({13.0, 7.0, 1.0}, exec);
    auto x = gko::initialize<Mtx>({0.0, 0.0, 0.0}, exec);

    solver->apply(b.get(), x.get());

    GKO_ASSERT_MTX_NEAR(x, l({1.0, 3.0, 2.0}), 1e-14);
}


TEST_F(Gmres, SolvesMultipleStencilSystems)
{
    auto solver = gmres_factory->generate(mtx);
    auto b = gko::initialize<Mtx>({{13.0, 6.0}, {7.0, 4.0}, {1.0, 1.0}}, exec);
    auto x = gko::initialize<Mtx>({{0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}}, exec);

    solver->apply(b.get(), x.get());

    GKO_ASSERT_MTX_NEAR(x, l({{1.0, 1.0}, {3.0, 1.0}, {2.0, 1.0}}), 1e-14);
}


TEST_F(Gmres, SolvesStencilSystemUsingAdvancedApply)
{
    auto solver = gmres_factory->generate(mtx);
    auto alpha = gko::initialize<Mtx>({2.0}, exec);
    auto beta = gko::initialize<Mtx>({-1.0}, exec);
    auto b = gko::initialize<Mtx>({13.0, 7.0, 1.0}, exec);
    auto x = gko::initialize<Mtx>({0.5, 1.0, 2.0}, exec);

    solver->apply(alpha.get(), b.get(), beta.get(), x.get());

    GKO_ASSERT_MTX_NEAR(x, l({1.5, 5.0, 2.0}), 1e-14);
}


TEST_F(Gmres, SolvesMultipleStencilSystemsUsingAdvancedApply)
{
    auto solver = gmres_factory->generate(mtx);
    auto alpha = gko::initialize<Mtx>({2.0}, exec);
    auto beta = gko::initialize<Mtx>({-1.0}, exec);
    auto b = gko::initialize<Mtx>({{13.0, 6.0}, {7.0, 4.0}, {1.0, 1.0}}, exec);
    auto x = gko::initialize<Mtx>({{0.5, 1.0}, {1.0, 2.0}, {2.0, 3.0}}, exec);

    solver->apply(alpha.get(), b.get(), beta.get(), x.get());

    GKO_ASSERT_MTX_NEAR(x, l({{1.5, 1.0}, {5.0, 0.0}, {2.0, -1.0}}), 1e-14);
}


TEST_F(Gmres, SolvesBigDenseSystem1)
{
    auto solver = gmres_factory_big->generate(mtx_big);
    auto b = gko::initialize<Mtx>(
        {72748.36, 297469.88, 347229.24, 36290.66, 82958.82, -80192.15}, exec);
    auto x = gko::initialize<Mtx>({0.0, 0.0, 0.0, 0.0, 0.0, 0.0}, exec);

    solver->apply(b.get(), x.get());

    GKO_ASSERT_MTX_NEAR(x, l({52.7, 85.4, 134.2, -250.0, -16.8, 35.3}), 1e-10);
}


TEST_F(Gmres, SolvesBigDenseSystem2)
{
    auto solver = gmres_factory_big->generate(mtx_big);
    auto b = gko::initialize<Mtx>(
        {175352.10, 313410.50, 131114.10, -134116.30, 179529.30, -43564.90},
        exec);
    auto x = gko::initialize<Mtx>({0.0, 0.0, 0.0, 0.0, 0.0, 0.0}, exec);

    solver->apply(b.get(), x.get());

    GKO_ASSERT_MTX_NEAR(x, l({33.0, -56.0, 81.0, -30.0, 21.0, 40.0}), 1e-10);
}


double infNorm(gko::matrix::Dense<> *mat, size_t col = 0)
{
    using std::abs;
    double norm = 0.0;
    for (size_t i = 0; i < mat->get_size()[0]; ++i) {
        double absEntry = abs(mat->at(i, col));
        if (norm < absEntry) norm = absEntry;
    }
    return norm;
}


TEST_F(Gmres, SolvesMultipleDenseSystemForDivergenceCheck)
{
    auto solver = gmres_factory_big->generate(mtx_big);
    auto b1 = gko::initialize<Mtx>(
        {1300083.0, 1018120.5, 906410.0, -42679.5, 846779.5, 1176858.5}, exec);
    auto b2 = gko::initialize<Mtx>(
        {886630.5, -172578.0, 684522.0, -65310.5, 455487.5, 607436.0}, exec);

    auto x1 = gko::initialize<Mtx>({0.0, 0.0, 0.0, 0.0, 0.0, 0.0}, exec);
    auto x2 = gko::initialize<Mtx>({0.0, 0.0, 0.0, 0.0, 0.0, 0.0}, exec);

    auto bc = Mtx::create(exec, gko::dim<2>{mtx_big->get_size()[0], 2});
    auto xc = Mtx::create(exec, gko::dim<2>{mtx_big->get_size()[1], 2});
    for (size_t i = 0; i < bc->get_size()[0]; ++i) {
        bc->at(i, 0) = b1->at(i);
        bc->at(i, 1) = b2->at(i);

        xc->at(i, 0) = x1->at(i);
        xc->at(i, 1) = x2->at(i);
    }

    solver->apply(b1.get(), x1.get());
    solver->apply(b2.get(), x2.get());
    solver->apply(bc.get(), xc.get());
    auto mergedRes = Mtx::create(exec, gko::dim<2>{b1->get_size()[0], 2});
    for (size_t i = 0; i < mergedRes->get_size()[0]; ++i) {
        mergedRes->at(i, 0) = x1->at(i);
        mergedRes->at(i, 1) = x2->at(i);
    }

    auto alpha = gko::initialize<Mtx>({1.0}, exec);
    auto beta = gko::initialize<Mtx>({-1.0}, exec);

    auto residual1 = Mtx::create(exec, b1->get_size());
    residual1->copy_from(b1.get());
    auto residual2 = Mtx::create(exec, b2->get_size());
    residual2->copy_from(b2.get());
    auto residualC = Mtx::create(exec, bc->get_size());
    residualC->copy_from(bc.get());

    mtx_big->apply(alpha.get(), x1.get(), beta.get(), residual1.get());
    mtx_big->apply(alpha.get(), x2.get(), beta.get(), residual2.get());
    mtx_big->apply(alpha.get(), xc.get(), beta.get(), residualC.get());

    double normS1 = infNorm(residual1.get());
    double normS2 = infNorm(residual2.get());
    double normC1 = infNorm(residualC.get(), 0);
    double normC2 = infNorm(residualC.get(), 1);
    double normB1 = infNorm(b1.get());
    double normB2 = infNorm(b2.get());

    // make sure that all combined solutions are as good or better than the
    // single solutions
    ASSERT_LE(normC1 / normB1, normS1 / normB1 + 1e-14);
    ASSERT_LE(normC2 / normB2, normS2 / normB2 + 1e-14);

    // Not sure if this is necessary, the assertions above should cover what is
    // needed.
    GKO_ASSERT_MTX_NEAR(xc, mergedRes, 1e-14);
}


TEST_F(Gmres, SolvesBigDenseSystem1WithRestart)
{
    auto gmres_factory_restart =
        gko::solver::Gmres<>::build()
            .with_krylov_dim(4u)
            .with_criteria(
                gko::stop::Iteration::build().with_max_iters(200u).on(exec),
                gko::stop::ResidualNormReduction<>::build()
                    .with_reduction_factor(1e-15)
                    .on(exec))
            .on(exec);
    auto solver = gmres_factory_restart->generate(mtx_medium);
    auto b = gko::initialize<Mtx>(
        {-13945.16, 11205.66, 16132.96, 24342.18, -10910.98}, exec);
    auto x = gko::initialize<Mtx>({0.0, 0.0, 0.0, 0.0, 0.0}, exec);

    solver->apply(b.get(), x.get());

    GKO_ASSERT_MTX_NEAR(x, l({-140.20, -142.20, 48.80, -17.70, -19.60}), 1e-5);
}


TEST_F(Gmres, SolvesWithPreconditioner)
{
    auto gmres_factory_preconditioner =
        gko::solver::Gmres<>::build()
            .with_criteria(
                gko::stop::Iteration::build().with_max_iters(100u).on(exec),
                gko::stop::ResidualNormReduction<>::build()
                    .with_reduction_factor(1e-15)
                    .on(exec))
            .with_preconditioner(gko::preconditioner::Jacobi<>::build()
                                     .with_max_block_size(3u)
                                     .on(exec))
            .on(exec);
    auto solver = gmres_factory_preconditioner->generate(mtx_big);
    auto b = gko::initialize<Mtx>(
        {175352.10, 313410.50, 131114.10, -134116.30, 179529.30, -43564.90},
        exec);
    auto x = gko::initialize<Mtx>({0.0, 0.0, 0.0, 0.0, 0.0, 0.0}, exec);

    solver->apply(b.get(), x.get());

    GKO_ASSERT_MTX_NEAR(x, l({33.0, -56.0, 81.0, -30.0, 21.0, 40.0}), 1e-10);
}


}  // namespace
