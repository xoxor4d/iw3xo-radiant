#include "std_include.hpp"

namespace components
{
	void on_effect_stop()
	{
		fx_system::ed_is_playing = false;
		fx_system::ed_is_repeating = false;

		const auto system = fx_system::FX_GetSystem(0);

		if(fx_system::ed_active_effect)
		{
			const auto stat = fx_system::FX_GetEffectStatus(fx_system::ed_active_effect);
			fx_system::FX_DelRefToEffect(system, fx_system::ed_active_effect);

			if(!stat)
			{
				fx_system::FX_KillEffect(system, fx_system::ed_active_effect);
			}

			fx_system::ed_active_effect = nullptr;
		}
	}

	void effects::radiant_init_fx()
	{
		on_effect_stop();
	}
	
	effects::effects()
	{ }

	effects::~effects()
	{ }
}
