//Whippet; A container for entity component systems.
//Copyright (C) 2017-2018 Peter LaValle / gmail
//
//This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//See the GNU Affero General Public License for more details.
//
//You should have received a copy of the GNU Affero General Public License (agpl-3.0.txt) along with this program.
//If not, see <https://www.gnu.org/licenses/>.


#include "whippet.hpp"

whippet::universe::universe(void) :
	_systems(nullptr)
{
}

whippet::entity whippet::universe::create(void)
{
	return whippet::entity(this, guid_activate());
}

whippet::guid_t whippet::universe::guid_activate(void)
{
	uint32_t next = 1 + (uint32_t)_guid_active.size();

	while (_guid_active.end() != _guid_active.find(next))
		--next;

	assert(0 != next);

	_guid_active.emplace(next);

	return next;
}

void whippet::universe::guid_release(whippet::guid_t guid)
{
	// we can only release "live" guid values (obviously)
	assert(_guid_active.end() != _guid_active.find(guid));

	_guid_active.erase(_guid_active.find(guid));

	assert(_guid_active.end() == _guid_active.find(guid));
}

void whippet::universe::visit_(const whippet::guid_t entity_guid, const std::type_index provider_type, void* userdata, bool(*callback)(void*, void*))
{
	assert((std::type_index(typeid(whippet::_component)) == provider_type) || _providers.contains(provider_type));

	if (std::type_index(typeid(whippet::_component)) != provider_type)
		_providers[provider_type]->visit(
			entity_guid, false,
			userdata, callback
		);
	else
		for (auto& provider : _providers)
			// allow the inners to break out of the full-visit
			if (!provider.second->visit(
				entity_guid, true,
				userdata, callback
			))
				return;
}

bool whippet::universe::installed_(std::type_index kind)const
{
	return _providers.contains(kind);
}

void whippet::universe::weed(void)
{
	for (auto& kv : _providers)
		kv.second->weed();
}

whippet::universe::~universe(void)
{
	// clear out components
	for (auto& kv : _providers)
		kv.second->purge();

	// cleanup entities & components
	_providers.clear();

	// cleanup _system objects
	if (nullptr != _systems)
		_systems->_cleanup(_systems);
}
