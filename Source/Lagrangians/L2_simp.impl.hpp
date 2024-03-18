#if !defined(L2_SIMP_H_INCLUDED)
#error "This file should only be included through BaseProcaFieldLevel.hpp"
#endif

#ifndef L2_SIMP_IMPL_H_INCLUDED
#define L2_SIMP_IMPL_H_INCLUDED



template <class G2>
template <class data_t, template <typename> class vars_t, template <typename> class diff2_vars_t, template <typename> class rhs_vars_t>
void L2<G2>::compute_emtensor_modification(
    emtensor_t<data_t> &base_emtensor,
    const vars_t<data_t> &matter_vars,
    const MetricVars<data_t> &metric_vars,
    const vars_t<Tensor<1,data_t>> &d1,
    const diff2_vars_t<Tensor<1,data_t>> &d2,
    const vars_t<data_t> &advec
) const
{
    //modify EM tensor
    Tensor<2,data_t> gamma_UU { TensorAlgebra::compute_inverse_sym(metric_vars.gamma) };

    data_t g_func { 0 };
    data_t g_prime { 0 };
    data_t g_prime2 { 0 };
    m_g2_function.compute_function(g_func, g_prime, g_prime2, matter_vars, metric_vars, d1, d2);

    base_emtensor.rho -= m_params.alpha2 * (  g_func + 2 * matter_vars.phi * matter_vars.phi * g_prime );

    FOR1(i)
    {
        base_emtensor.Si[i] +=  m_params.alpha2 * (  2 * matter_vars.Avec[i] * matter_vars.phi * g_prime );

        FOR1(j)
        {
            base_emtensor.S +=  m_params.alpha2 * (  3 * g_func - 2 * gamma_UU[i][j] * matter_vars.Avec[i] * matter_vars.Avec[j] * g_prime );
            base_emtensor.Sij[i][j] +=  m_params.alpha2 * (  metric_vars.gamma[i][j] * g_func - 2 * matter_vars.Avec[i] * matter_vars.Avec[j] * g_prime );
        }
    }
};



template <class G2>
template <class data_t, template <typename> class vars_t, template <typename> class diff2_vars_t, template <typename> class rhs_vars_t>
void L2<G2>::matter_rhs_modification(
    rhs_vars_t<data_t> &total_rhs,
    const vars_t<data_t> &vars,
    const MetricVars<data_t> &metric_vars,
    const vars_t<Tensor<1,data_t>> &d1,
    const diff2_vars_t<Tensor<1,data_t>> &d2,
    const vars_t<data_t> &advec
) const
{
    //modify RHS
    Tensor<2,data_t> gamma_UU { TensorAlgebra::compute_inverse_sym(metric_vars.gamma) };
    auto chris_phys { TensorAlgebra::compute_christoffel(metric_vars.d1_gamma, gamma_UU) };

    data_t g_prime { 0 };
    data_t g_prime2 { 0 };
    m_g2_function(g_prime, g_prime2, vars, metric_vars, d1, d2);
    data_t gnn { g_prime - 2 * vars.phi * vars.phi * g_prime2 };

    // covariant derivative of spatial part of Proca field
    Tensor<2, data_t> DA;
    FOR2(i, j)
    {
        DA[i][j] = d1.Avec[j][i];
        FOR1(k) { DA[i][j] -= chris_phys[k][i][j] * vars.Avec[k]; }
    }



    //Modify electric part
    FOR2(i,j)
    {
        total_rhs.Evec[i] +=  m_params.alpha2 * (  2 * metric_vars.lapse * m_params.alpha2 * g_prime * gamma_UU[i][j] * vars.Avec[j] );
    }

    //Spatial part remains unchanged

    //Modify scalar part
    total_rhs.phi +=  m_params.alpha2 * (  vars.lapse * (g_prime - 1) * vars.phi * vars.K / (gnn) );
    FOR1(i)
    {
        total_rhs.phi +=  m_params.alpha2 * (  2 * vars.lapse * g_prime2 * vars.phi * vars.Avec[i] * vars.Evec[i] / gnn );

        FOR1(j)
        {
            total_rhs.phi +=
                 m_params.alpha2 * (  vars.lapse * gamma_UU[i][j] * ( - (g_prime - 1) / gnn * DA[i][j] +
                                2 *  g_prime2 / gnn * 2 * vars.phi *
                                    vars.Avec[i] * d1.phi[j]) );

            FOR2(k, l)
            {
                total_rhs.phi -=
                     m_params.alpha2 * (  2 * vars.lapse * g_prime2 / gnn * gamma_UU[i][k] * gamma_UU[j][l] *
                    ( 
                        vars.phi * vars.Avec[i] * vars.Avec[j] * metric_vars.K_tensor[k][l]
                    + vars.Avec[i] * vars.Avec[j] * DA[k][l]
                    ) );
            }
        }
    }
    
};

#endif //L2_SIMP_IMPL_H_INCLUDED