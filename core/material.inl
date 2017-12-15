template<typename... scatter_types>
HOST material<scatter_list<scatter_types...>>::material(material_legacy_thomas const & mat_legacy)
	: scatter_list<scatter_types...>(scatter_types::create(mat_legacy)...),
	barrier(static_cast<real>(mat_legacy.barrier() / constant::ec))
{
}

template<typename... scatter_types>
PHYSICS bool material<scatter_list<scatter_types...>>::can_reach_vacuum(real kineticEnergy) const
{
	return kineticEnergy >= barrier;
}