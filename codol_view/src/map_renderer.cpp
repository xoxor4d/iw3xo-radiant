// The public face of codol_view: device ownership rules, targets, device loss, the editor queries.

#include "renderer_impl.hpp"

namespace codol::view
{
	bool map_renderer::impl::init(IDirect3DDevice9* dev, const options& o, std::string* error)
	{
		shutdown();
		const auto fail = [&](std::string text)
		{
			last_error = text;
			render::set_error(error, std::move(text));
			shutdown();
			return false;
		};
		if (!dev)
		{
			return fail("no Direct3D device");
		}
		opts = o;
		dev->AddRef();
		*device.put() = dev;
		if (FAILED(device->GetDeviceCaps(&caps)))
		{
			return fail("IDirect3DDevice9::GetDeviceCaps failed");
		}
		if (caps.VertexShaderVersion < D3DVS_VERSION(3, 0) || caps.PixelShaderVersion < D3DPS_VERSION(3, 0))
		{
			return fail("the device does not run shader model 3 ( the game's shaders are vs_3_0 / ps_3_0 )");
		}
		{
			IDirect3DDevice9Ex* ex = nullptr;
			is_ex = SUCCEEDED(device->QueryInterface(__uuidof(IDirect3DDevice9Ex), reinterpret_cast<void**>(&ex))) && ex;
			if (ex)
			{
				ex->Release();
			}
		}
		// IDirect3DDevice9Ex has no managed pool
		managed = !is_ex && opts.pool != resource_pool::default_pool;

		if (opts.client_exe.empty())
		{
			opts.client_exe = opts.game_root / "codoMP_client_shipRetail.exe";
		}
		if (opts.zone_directories.empty())
		{
			opts.zone_directories = { opts.game_root / "client_shipRetail" / "chinese", opts.game_root / "client_shipRetail" / "patch" };
		}
		std::string text;
		if (!library.init(opts.client_exe, opts.zone_directories, &text))
		{
			return fail(text);
		}
		if (!render::derive_iwd_key(opts.client_exe, iwd_key, &text))
		{
			return fail("IWD key: " + text);
		}
		std::vector<std::string> archive_warnings;
		if (!render::open_image_archives(opts.game_root, iwd_key, images, &text, &archive_warnings))
		{
			return fail(text);
		}
		{
			// GameInstallation.cs:343: the "models/" entries of main ( and main_shared )
			std::vector<std::filesystem::path> directories = { opts.game_root / "main" };
			std::error_code ec;
			if (std::filesystem::is_directory(opts.game_root / "main_shared", ec))
			{
				directories.push_back(opts.game_root / "main_shared");
			}
			model_archives = render::iwd_archive_set::open(directories, &iwd_key, "models/", {}, &archive_warnings);
		}
		wavelet = render::wavelet_tables::load_from_executable(opts.client_exe, &text);
		if (!wavelet)
		{
			return fail("wavelet tables: " + text);
		}
		return true;
	}

	void map_renderer::impl::release_default_pool()
	{
		target_surface.reset();
		// the host was given this texture by render(): see host_retired
		retire_host_texture(target_texture);
		depth_surface.reset();
		target_width = target_height = depth_width = depth_height = 0;
		saved_state.reset();
		float_z_surface.reset();
		float_z_texture.reset();
		float_z.handle.reset();
		float_z_width = float_z_height = 0;
		float_z_unavailable = false;
		release_shadow_targets();
		shadow_unavailable = false;
		current_sun_shadow.reset();
	}

	void map_renderer::impl::unload()
	{
		map_loaded = false;
		frame = nullptr;
		release_default_pool();
		opaque_models.clear();
		transparent_models.clear();
		opaque_brushes.clear();
		transparent_brushes.clear();
		opaque_meshes.clear();
		transparent_meshes.clear();
		fixed_meshes.clear();
		meshes.clear();
		placed_models.clear();
		model_backups.clear();
		brush_backups.clear();
		entity_models.clear();
		entity_brushes.clear();
		static_count = 0;
		next_handle = 0;
		hidden_count = 0;
		held_map_zone.reset();
		held_zone.reset();
		material_by_asset.clear();
		lighting_by_material.clear();
		resolved_models.clear();
		grid.reset();
		trace.reset();
		light_grid_bytes = 0;
		lighting_handles = 0;
		free_lighting_handles.clear();
		lighting_fill.reset();
		lighting_texture_height = 0;
		brush_pick.clear();
		world_pick.clear();
		world_positions.clear();
		world_positions.shrink_to_fit();
		pick_bytes = 0;
		pick_triangles = 0;
		pick_build_ms = 0;
		model_name_list.clear();
		material_name_list.clear();
		model_names_built = material_names_built = false;
		image_sizes.clear();
		for (auto& entry : thumbnails)
		{
			retire_host_texture(entry.second);
		}
		thumbnails.clear();
		surface_visible.clear();
		visibility.reset();
		lighting_volume.reset();
		model_lighting = {};
		static_models.clear();
		brush_models.clear();
		models.clear();
		retired_models.clear();
		surfaces.clear();
		ranges.clear();
		vertex_buffer.reset();
		layer_buffer.reset();
		index_buffer.reset();
		world_indices.reset();
		lightmaps.clear();
		probes.clear();
		sun = nullptr;
		lights.clear();
		light_defs.clear();
		outdoor = nullptr;
		pass_plans.clear();
		materials.clear();
		declarations.clear();
		programs.clear();
		vertex_shaders.clear();
		pixel_shaders.clear();
		textures.clear();
		retired_textures.clear();
		texture_failed.clear();
		texture_failures.clear();
		case_textures.clear();
		case_sizes_frame.clear();
		case_sizes_last.clear();
		white = {};
		black = {};
		flat_normal = {};
		state_bits.clear();
		sampler_states.clear();
		applied_state_bits = nullptr;
		unbound_constants.clear();
		constants.reset();
		scope.zones.clear();
		retired_zones.clear();
		entities.clear();
		entity_string.clear();
		warnings.clear();
		map_name.clear();
		next_material_id = 0;
		fog_source = "nothing loaded";
		has_fog = false;
		stat = {};
	}

	void map_renderer::impl::shutdown()
	{
		unload();
		host_retired.clear();
		library.clear();
		images = {};
		model_archives.reset();
		wavelet.reset();
		device.reset();
		device_lost = false;
	}

	bool map_renderer::impl::ensure_targets(const int width, const int height, const bool need_color, const bool need_depth, const bool need_float_z)
	{
		if (need_float_z && !float_z_unavailable && (!float_z_surface || float_z_width != width || float_z_height != height))
		{
			// GameMapRenderer.cs:698-712: without the target soft particles / decals are not feathered
			float_z_surface.reset();
			float_z_texture.reset();
			float_z.handle.reset();
			if (FAILED(device->CreateTexture(static_cast<UINT>(width), static_cast<UINT>(height), 1, D3DUSAGE_RENDERTARGET, D3DFMT_R32F, D3DPOOL_DEFAULT,
				float_z_texture.put(), nullptr)) || FAILED(float_z_texture->GetSurfaceLevel(0, float_z_surface.put())))
			{
				float_z_texture.reset();
				float_z_unavailable = true;
				warn("no float z target; z feathered materials are not feathered");
			}
			else
			{
				*float_z.handle.put() = float_z_texture.get();
				float_z_texture.get()->AddRef();
				float_z.name = "$floatz";
				float_z_width = width;
				float_z_height = height;
			}
		}
		// GameMapRenderer.cs:684-715 ( EnsureTargets )
		if (need_color && (!target_surface || target_width != width || target_height != height))
		{
			target_surface.reset();
			target_texture.reset();
			if (FAILED(device->CreateTexture(static_cast<UINT>(width), static_cast<UINT>(height), 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
				target_texture.put(), nullptr)) || FAILED(target_texture->GetSurfaceLevel(0, target_surface.put())))
			{
				target_texture.reset();
				last_error = "cannot create the " + std::to_string(width) + "x" + std::to_string(height) + " render target";
				return false;
			}
			target_width = width;
			target_height = height;
		}
		if (need_depth && (!depth_surface || depth_width != width || depth_height != height))
		{
			depth_surface.reset();
			if (FAILED(device->CreateDepthStencilSurface(static_cast<UINT>(width), static_cast<UINT>(height), D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0, TRUE,
				depth_surface.put(), nullptr)))
			{
				last_error = "cannot create the " + std::to_string(width) + "x" + std::to_string(height) + " depth buffer";
				return false;
			}
			depth_width = width;
			depth_height = height;
		}
		return true;
	}

	// ---------------------------------------------------------------------------------------------------

	map_renderer::map_renderer() : impl_(std::make_unique<impl>()) {}
	map_renderer::~map_renderer() { impl_->shutdown(); }

	bool map_renderer::init(IDirect3DDevice9* device, const options& options, std::string* error) { return impl_->init(device, options, error); }
	void map_renderer::shutdown() { impl_->shutdown(); }
	bool map_renderer::initialised() const { return static_cast<bool>(impl_->device); }

	bool map_renderer::load_map(const std::string& map_name, std::string* error)
	{
		try
		{
			impl_->pending_edits = {};
			return impl_->load_map(map_name, error);
		}
		catch (const std::exception& e)
		{
			impl_->unload();
			impl_->last_error = std::string("load_map: ") + e.what();
			render::set_error(error, impl_->last_error);
			return false;
		}
	}

	void map_renderer::unload()
	{
		impl_->pending_edits = {};
		impl_->unload();
	}

	void map_renderer::release_retired()
	{
		impl_->host_retired.clear();
	}
	std::uint64_t map_renderer::host_texture_generation() const { return impl_->host_generation; }
	bool map_renderer::loaded() const { return impl_->map_loaded; }
	const std::string& map_renderer::map_name() const { return impl_->map_name; }

	IDirect3DTexture9* map_renderer::render(const camera& cam, const int width, const int height, const frame_options& frame)
	{
		try
		{
			return impl_->render(cam, width, height, frame);
		}
		catch (const std::exception& e)
		{
			impl_->last_error = std::string("render: ") + e.what();
			impl_->frame = nullptr;
			return nullptr;
		}
	}

	void map_renderer::on_device_lost()
	{
		impl& i = *impl_;
		if (!i.device)
		{
			return;
		}
		i.device_lost = true;
		i.release_default_pool();
		// Reset fails while anything of the default pool is alive, and the host draws nothing with a lost device
		i.host_retired.clear();
		if (!i.managed && i.map_loaded)
		{
			// everything lives in the default pool: it has to go before Reset and is loaded again after it; so are
			// the edits ( hidden and moved objects, placed models, meshes )
			const std::string name = i.map_name;
			i.pending_edits = i.capture_edits();
			i.unload();
			i.host_retired.clear();
			i.map_name = name;
		}
	}

	bool map_renderer::on_device_reset()
	{
		impl& i = *impl_;
		if (!i.device)
		{
			return false;
		}
		i.device_lost = false;
		if (!i.managed && !i.map_loaded && !i.map_name.empty())
		{
			const std::string name = i.map_name;
			const edit_snapshot edits = std::move(i.pending_edits);
			i.pending_edits = {};
			if (!load_map(name, nullptr))
			{
				return false;
			}
			i.apply_edits(edits);
			return true;
		}
		return true;		// targets and the state block are made again by the next render()
	}

	IDirect3DTexture9* map_renderer::depth_texture() const { return impl_->float_z_texture.get(); }
	statistics map_renderer::stats() const { return impl_->stat; }
	std::vector<std::string> map_renderer::warnings() const { return { impl_->warnings.begin(), impl_->warnings.end() }; }
	std::vector<std::string> map_renderer::texture_failures() const { return impl_->texture_failures; }
	const std::string& map_renderer::last_error() const { return impl_->last_error; }
	const std::string& map_renderer::entity_string() const { return impl_->entity_string; }
	const std::string& map_renderer::fog_source() const { return impl_->fog_source; }

	bool map_renderer::world_bounds(float3& center, float3& half_size) const
	{
		center = impl_->world_mid;
		half_size = impl_->world_half;
		return impl_->map_loaded;
	}

	std::vector<scene_object> map_renderer::objects() const
	{
		const impl& i = *impl_;
		std::vector<scene_object> result;
		if (!i.map_loaded)
		{
			return result;
		}
		result.reserve(i.surfaces.size() + i.static_models.size() + i.brush_models.size() + i.placed_models.size() + i.meshes.size());
		for (const world_surface& s : i.surfaces)
		{
			if (s.brush_model)
			{
				continue;
			}
			i.fill_object(s, result.emplace_back());
		}
		for (const static_model_instance& m : i.static_models)
		{
			i.fill_object(m, result.emplace_back());
		}
		for (const brush_model_instance& b : i.brush_models)
		{
			i.fill_object(b, result.emplace_back());
		}
		for (const auto& placed : i.placed_models)
		{
			i.fill_object(*placed.second, result.emplace_back());
		}
		for (const auto& mesh : i.meshes)
		{
			i.fill_object(*mesh.second, result.emplace_back());
		}
		return result;
	}

	bool map_renderer::spawn_camera(const int n, camera& out) const
	{
		// RenderCommands.cs:222-232 ( DefaultCamera ): the intermission camera first; then the spawn points at eye height
		const impl& i = *impl_;
		int index = 0;
		for (int pass = 0; pass < 2; pass++)
		{
			for (const map_entity& entity : i.entities)
			{
				const std::string name = to_lower(entity.class_name());
				const bool intermission = name == "mp_global_intermission";
				const bool spawn = !intermission && name.find("spawn") != std::string::npos;
				if ((pass == 0 ? !intermission : !spawn))
				{
					continue;
				}
				const auto origin = parse_vector(entity.get("origin"));
				if (!origin)
				{
					continue;
				}
				if (index++ == n)
				{
					const float3 angles = parse_vector(entity.get("angles")).value_or(float3{});
					out.origin = pass == 0 ? *origin : *origin + float3{ 0, 0, 60 };
					out.pitch = angles.x;
					out.yaw = angles.y;
					out.roll = angles.z;
					return true;
				}
			}
		}
		return false;
	}
}
