#ifndef __ELECTRON_IONISATION_H_
#define __ELECTRON_IONISATION_H_

#include "../common/util/table_2D.h"
#include "../common/util/random.h"
#include "../legacy_thomas/material.hh"

namespace nbl { namespace scatter {

template<bool gpu_flag>
class electron_ionisation
{
public:
	PHYSICS real sample(real K, util::random_generator<gpu_flag>& rng) const
	{
		const real x = logr(K);
		const real y = rng.unit();
		return _ionisation_table.get_rounddown(x, y);
	}

	static HOST electron_ionisation create(material_legacy_thomas const & mat)
	{
		/*
		 * TODO: move this somewhere else (see inelastic, elastic, binding)
		 * Translate index in a table to the relevant physical value.
		 * K_at refers to kinetic energy (log space) and P_at refers to
		 * the differential cross section (linear space).
		 */
		auto __logspace_K_at = [&](int x)
		{
			return K_min*std::exp(1.0*x / (K_cnt - 1)*std::log(K_max / K_min));
		};
		auto __linspace_P_at = [&](int y)
		{
			return 1.0*y / (P_cnt - 1);
		};
		
		std::vector<real> ionisation_vector(K_cnt*P_cnt, 0);
		for (int y = 0; y < P_cnt; ++y)
		{
			const double P = __linspace_P_at(y);
			for (int x = 0; x < K_cnt; ++x)
			{
				const double omega0 = __logspace_K_at(x); // in eV
				const double margin = 10; // Magic KB number, in eV
				
				double binding = -1;
				if (omega0 > 100)
				{
					binding = mat.ionization_energy((omega0 + margin)*constant::ec, P) / constant::ec;
					if (binding < 50)
						binding = -1;
				}
				if (binding < 0)
				{
					binding = mat.outer_shell_ionization_energy(omega0*constant::ec) / constant::ec;
					if (binding < 0)
						binding = -1;
				}

				ionisation_vector[y*K_cnt + x] = (real)binding;
			}
		}

		electron_ionisation ei;
		ei._ionisation_table = util::table_2D<real, gpu_flag>::create(ionisation_vector.data(), logr(K_min), logr(K_max), K_cnt, 0, 1, P_cnt);
		return ei;
	}
	static HOST void destroy(electron_ionisation & ei)
	{
		util::table_2D<real, gpu_flag>::destroy(ei._ionisation_table);
	}

private:
	util::table_2D<real, gpu_flag> _ionisation_table;
};

}} // namespace nbl::scatter

#endif // __ELECTRON_IONISATION_H_