// AssetLinker: compiled assets -> pieces of zone stream -> ZoneEditor -> fast file.

#include "codol_assets/link.hpp"

#include <algorithm>
#include <cstring>
#include <set>

namespace codol::assets
{
	namespace
	{
		namespace fs = std::filesystem;
		using zone::ZoneEditor;
		using zone::ZoneNewPiece;
		using zone::ZoneNewPointer;

		struct Pieces
		{
			std::vector<ZoneNewPiece> list;

			std::uint32_t add(const void* data, const std::size_t size)
			{
				ZoneNewPiece piece;
				piece.bytes.assign(static_cast<const std::uint8_t*>(data), static_cast<const std::uint8_t*>(data) + size);
				list.push_back(std::move(piece));
				return static_cast<std::uint32_t>(list.size() - 1);
			}
			template <typename T>
			std::uint32_t add(const T& value) { return add(&value, sizeof(T)); }
			template <typename T>
			std::uint32_t add_array(const std::vector<T>& values) { return add(values.data(), values.size() * sizeof(T)); }
			std::uint32_t add_string(const std::string_view text)
			{
				ZoneNewPiece piece;
				piece.string = true;
				piece.bytes.assign(text.begin(), text.end());
				piece.bytes.push_back(0);
				list.push_back(std::move(piece));
				return static_cast<std::uint32_t>(list.size() - 1);
			}
			void to_piece(const std::uint32_t piece, const std::uint32_t offset, const std::uint32_t target, const std::uint32_t targetOffset = 0)
			{
				ZoneNewPointer p;
				p.offset = offset;
				p.piece = target;
				p.pieceOffset = targetOffset;
				list[piece].pointers.push_back(p);
			}
			void to_asset(const std::uint32_t piece, const std::uint32_t offset, const std::size_t asset)
			{
				ZoneNewPointer p;
				p.offset = offset;
				p.asset = asset;
				list[piece].pointers.push_back(p);
			}
			void put32(const std::uint32_t piece, const std::uint32_t offset, const std::uint32_t value)
			{
				std::memcpy(list[piece].bytes.data() + offset, &value, 4);
			}
		};

		// an asset of another zone: a header that holds only the name, with a comma in front
		template <typename Header>
		void add_reference(Pieces& pieces, const std::string& name)
		{
			Header header{};
			header.name = c1::ZPTR_INLINE;
			pieces.add(header);
			pieces.add_string("," + name);
		}
	}

	struct AssetLinker::Impl
	{
		LinkOptions options;
		zone::ClientKey key;
		MaterialLibrary library;
		ZoneEditor editor;
		std::map<std::string, std::unique_ptr<ZoneEditor>> donors;
		std::vector<std::string> log;

		Status donor(const std::string& relative, ZoneEditor*& out)
		{
			auto& slot = donors[relative];
			if (!slot)
			{
				slot = std::make_unique<ZoneEditor>();
				if (const Status s = slot->open(options.install / relative, key); !s)
				{
					slot.reset();
					donors.erase(relative);
					return Status::error("donor zone " + relative + ": " + s.message);
				}
			}
			out = slot.get();
			return {};
		}

		Status exists(const std::int32_t type, const std::string& name) const
		{
			return editor.find_asset(type, name) == ZoneEditor::npos ? Status{}
				: Status::error("the zone already has " + zone::asset_type_name(type) + " '" + name + "'");
		}

		// 1 x 1 images every map zone carries its own copy of
		Status default_image(const std::string& name, std::size_t& index)
		{
			std::uint8_t pixel[4];		// B G R A
			std::uint8_t semantic = 0;
			if (name == "$identitynormalmap")
			{
				const std::uint8_t p[4] = { 0xFF, 0x80, 0x80, 0x80 };
				std::memcpy(pixel, p, 4);
				semantic = 5;
			}
			else if (name == "$white")
			{
				std::memset(pixel, 0xFF, 4);
			}
			else if (name == "$black")
			{
				const std::uint8_t p[4] = { 0, 0, 0, 0xFF };
				std::memcpy(pixel, p, 4);
			}
			else if (name == "$gray")
			{
				std::memset(pixel, 0x80, 4);
			}
			else
			{
				return Status::error("not a default image");
			}
			CompiledImage image;
			image.name = name;
			image.semantic = semantic;
			image.width = image.height = 1;
			image.iwiFormat = 1;
			image.d3dFormat = 21;
			image.flags = 3;			// no picmip, no mips: as shipped
			image.levelCount = 1;
			image.surfaces.push_back({ pixel, pixel + 4 });
			const std::vector<ZoneNewPiece> pieces = image_pieces(image);
			if (const Status s = editor.add_new_asset(c1::ASSET_TYPE_IMAGE, name, pieces); !s)
			{
				return s;
			}
			index = editor.find_asset(c1::ASSET_TYPE_IMAGE, name);
			log.push_back("image " + name + ": 1 x 1 copy added, as map zones have it");
			return {};
		}

		Status techset(const std::string& name, std::size_t& index, bool& byName)
		{
			byName = false;
			index = editor.find_asset(c1::ASSET_TYPE_TECHNIQUE_SET, name);
			if (index != ZoneEditor::npos)
			{
				return {};
			}
			const LibraryTechset* t = library.find(name);
			if (!t || (t->commonZone.empty() && t->donorZone.empty()))
			{
				return Status::error("technique set '" + name + "' is not defined by any zone of the install");
			}
			if (!t->commonZone.empty() && !options.copyAllTechsets)
			{
				byName = true;
				return {};
			}
			ZoneEditor* from = nullptr;
			if (const Status s = donor(t->donorZone, from); !s)
			{
				return s;
			}
			const std::size_t there = from->find_asset(c1::ASSET_TYPE_TECHNIQUE_SET, name);
			if (there == ZoneEditor::npos)
			{
				return Status::error("technique set '" + name + "' is not in " + t->donorZone + " ( stale material library? )");
			}
			const std::size_t before = editor.asset_count();
			if (const Status s = editor.add_asset_closure(*from, there); !s)
			{
				return Status::error("technique set '" + name + "' from " + t->donorZone + ": " + s.message);
			}
			index = editor.find_asset(c1::ASSET_TYPE_TECHNIQUE_SET, name);
			log.push_back("techset " + name + ": copied from " + t->donorZone + " ( " + std::to_string(editor.asset_count() - before) + " assets with what it shares )");
			// "<..>_sm_<..>" has a sibling "<..>_hsm_<..>" wherever it is defined: the set the game draws with when it
			// shadows with hardware shadow maps, found by name. Without it the material gets no dynamic sun shadow.
			if (const std::size_t at = name.find("_sm_"); at != std::string::npos && index != ZoneEditor::npos)
			{
				std::string sibling = name;
				sibling.replace(at, 4, "_hsm_");
				if (editor.find_asset(c1::ASSET_TYPE_TECHNIQUE_SET, sibling) == ZoneEditor::npos)
				{
					const std::size_t theirs = from->find_asset(c1::ASSET_TYPE_TECHNIQUE_SET, sibling);
					if (theirs != ZoneEditor::npos && editor.add_asset_closure(*from, theirs))
					{
						log.push_back("techset " + sibling + ": copied with it ( hardware shadow maps )");
					}
					else
					{
						log.push_back("techset " + sibling + ": not a top level asset of " + t->donorZone + "; materials with " + name + " get no dynamic sun shadow from this zone alone");
					}
					index = editor.find_asset(c1::ASSET_TYPE_TECHNIQUE_SET, name);
				}
			}
			return index == ZoneEditor::npos ? Status::error("technique set '" + name + "' did not arrive") : Status{};
		}
	};

	AssetLinker::AssetLinker() : impl_(std::make_unique<Impl>()) {}
	AssetLinker::~AssetLinker() = default;

	const MaterialLibrary& AssetLinker::library() const { return impl_->library; }
	ZoneEditor& AssetLinker::editor() { return impl_->editor; }
	const zone::ClientKey& AssetLinker::key() const { return impl_->key; }
	const std::vector<std::string>& AssetLinker::log() const { return impl_->log; }

	Status AssetLinker::begin(const LinkOptions& options)
	{
		impl_->options = options;
		const fs::path exe = options.exe.empty() ? options.install / "codoMP_client_shipRetail.exe" : options.exe;
		if (const Status s = impl_->key.load_from_exe(exe); !s)
		{
			return s;
		}
		if (const Status s = impl_->library.open(options.install, impl_->key, options.libraryCache); !s)
		{
			return s;
		}
		return options.base.empty() ? impl_->editor.create(options.zoneName) : impl_->editor.open(options.base, impl_->key);
	}

	Status AssetLinker::add_image(const CompiledImage& image)
	{
		if (const Status s = impl_->exists(c1::ASSET_TYPE_IMAGE, image.name); !s)
		{
			return s;
		}
		const std::vector<ZoneNewPiece> pieces = image_pieces(image);
		return impl_->editor.add_new_asset(c1::ASSET_TYPE_IMAGE, image.name, pieces);
	}

	Status AssetLinker::carry(const fs::path& fastfile, const std::span<const std::int32_t> types, Carried& out)
	{
		ZoneEditor donor;
		if (const Status s = donor.open(fastfile, impl_->key); !s)
		{
			return Status::error(fastfile.string() + ": " + s.message);
		}
		ZoneEditor& editor = impl_->editor;
		// what the zone had before: an asset that came along with an earlier one is not "kept"
		std::set<std::pair<std::int32_t, std::string>> before;
		for (std::size_t k = 0; k < editor.asset_count(); k++)
		{
			const zone::EditorAsset a = editor.asset(k);
			before.emplace(a.type, a.name);
		}
		for (std::size_t k = 0; k < donor.asset_count(); k++)
		{
			const zone::EditorAsset a = donor.asset(k);
			if (std::find(types.begin(), types.end(), a.type) == types.end())
			{
				continue;
			}
			const std::string label = zone::asset_type_name(a.type) + " " + a.name;
			if (editor.find_asset(a.type, a.name) != ZoneEditor::npos)
			{
				if (before.count({ a.type, a.name }))
				{
					out.kept.push_back(label);
				}
				else
				{
					// came with an asset that points at it: it is one of the carried, not a dependency
					out.copied[a.type]++;
					out.dependencies -= out.dependencies ? 1 : 0;
				}
				continue;
			}
			const std::size_t count = editor.asset_count();
			if (const Status s = editor.add_asset_closure(donor, k); !s)
			{
				out.failed.push_back(label + ": " + s.message);
				continue;
			}
			out.copied[a.type]++;
			out.dependencies += static_cast<std::uint32_t>(editor.asset_count() - count - 1);
		}
		return {};
	}

	Status AssetLinker::add_rawfile(const std::string_view name, const std::span<const std::uint8_t> contents)
	{
		return impl_->editor.set_rawfile(name, contents);
	}

	Status AssetLinker::add_material(const CompiledMaterial& material)
	{
		if (const Status s = impl_->exists(c1::ASSET_TYPE_MATERIAL, material.name); !s)
		{
			return s;
		}
		// what it points at first: the editor's asset indices are taken afterwards
		std::size_t techsetIndex = ZoneEditor::npos;
		bool techsetByName = false;
		if (const Status s = impl_->techset(material.techset, techsetIndex, techsetByName); !s)
		{
			return Status::error("material '" + material.name + "': " + s.message);
		}
		std::vector<std::size_t> imageIndex(material.images.size(), ZoneEditor::npos);
		for (std::size_t k = 0; k < material.images.size(); k++)
		{
			imageIndex[k] = impl_->editor.find_asset(c1::ASSET_TYPE_IMAGE, material.images[k]);
			if (imageIndex[k] == ZoneEditor::npos && !material.images[k].empty() && material.images[k].front() == '$')
			{
				impl_->default_image(material.images[k], imageIndex[k]);
			}
		}
		techsetIndex = techsetByName ? ZoneEditor::npos : impl_->editor.find_asset(c1::ASSET_TYPE_TECHNIQUE_SET, material.techset);
		for (std::size_t k = 0; k < material.images.size(); k++)
		{
			if (imageIndex[k] != ZoneEditor::npos)
			{
				imageIndex[k] = impl_->editor.find_asset(c1::ASSET_TYPE_IMAGE, material.images[k]);
			}
		}

		Pieces pieces;
		c1::Material header = material.header;
		header.name = c1::ZPTR_INLINE;
		header.techniqueSet = c1::ZPTR_INLINE;
		header.textureTable = material.textures.empty() ? 0 : c1::ZPTR_INLINE;
		header.constantTable = material.constants.empty() ? 0 : c1::ZPTR_INLINE;
		header.stateBitsTable = material.stateBits.empty() ? 0 : c1::ZPTR_INLINE;
		const std::uint32_t head = pieces.add(header);
		pieces.add_string(material.name);
		if (techsetByName)
		{
			add_reference<c1::MaterialTechniqueSet>(pieces, material.techset);
			impl_->log.push_back("material " + material.name + ": technique set " + material.techset + " by name ( defined in "
				+ impl_->library.find(material.techset)->commonZone + " )");
		}
		else
		{
			pieces.to_asset(head, offsetof(c1::Material, techniqueSet), techsetIndex);
		}
		if (!material.textures.empty())
		{
			std::vector<c1::MaterialTextureDef> table = material.textures;
			for (c1::MaterialTextureDef& t : table)
			{
				t.image = c1::ZPTR_INLINE;
			}
			const std::uint32_t tablePiece = pieces.add_array(table);
			for (std::size_t k = 0; k < table.size(); k++)
			{
				if (imageIndex[k] != ZoneEditor::npos)
				{
					pieces.to_asset(tablePiece, static_cast<std::uint32_t>(k * sizeof(c1::MaterialTextureDef) + offsetof(c1::MaterialTextureDef, image)), imageIndex[k]);
				}
				else
				{
					add_reference<c1::GfxImage>(pieces, material.images[k]);
					impl_->log.push_back("material " + material.name + ": image " + material.images[k] + " is not in the zone, referenced by name");
				}
			}
		}
		if (!material.constants.empty())
		{
			pieces.add_array(material.constants);
		}
		if (!material.stateBits.empty())
		{
			pieces.add_array(material.stateBits);
		}
		for (const std::string& note : material.notes)
		{
			impl_->log.push_back("material " + material.name + ": " + note);
		}
		return impl_->editor.add_new_asset(c1::ASSET_TYPE_MATERIAL, material.name, pieces.list);
	}

	Status AssetLinker::add_model(const CompiledModel& model)
	{
		if (const Status s = impl_->exists(c1::ASSET_TYPE_XMODEL, model.name); !s)
		{
			return s;
		}
		const std::vector<std::string> materials = model.materials();
		const std::size_t boneCount = model.boneNames.size();
		const std::size_t children = boneCount - model.rootBones;
		if (materials.size() > 255 || boneCount == 0 || model.lods.empty())
		{
			return Status::error("model '" + model.name + "': more than 255 surfaces, or nothing to write");
		}

		Pieces pieces;
		c1::XModel header{};
		header.name = c1::ZPTR_INLINE;
		header.numBones = static_cast<std::uint8_t>(boneCount);
		header.numRootBones = model.rootBones;
		header.numsurfs = static_cast<std::uint8_t>(materials.size());
		header.scale = 1.0f;
		header.boneNames = c1::ZPTR_INLINE;
		header.parentList = header.quats = header.trans = children ? c1::ZPTR_INLINE : 0;
		header.partClassification = c1::ZPTR_INLINE;
		header.baseMat = c1::ZPTR_INLINE;
		header.materialHandles = c1::ZPTR_INLINE;
		std::uint16_t surfIndex = 0;
		for (std::size_t l = 0; l < 4; l++)
		{
			c1::XModelLodInfo& info = header.lodInfo[l];
			info.dist = 1000000.0f;
			if (l < model.lods.size())
			{
				info.dist = model.lods[l].distance;
				info.numsurfs = static_cast<std::uint16_t>(model.lods[l].surfaces.size());
				info.surfIndex = surfIndex;
				info.modelSurfs = c1::ZPTR_INLINE_INSERT;
				info._unk52[0] = static_cast<std::uint8_t>(l);		// the LOD's own index; the three bytes after it are static model cache state
				surfIndex = static_cast<std::uint16_t>(surfIndex + info.numsurfs);
			}
		}
		header.numLods = static_cast<std::uint8_t>(model.lods.size());
		header._unk298 = static_cast<std::uint8_t>(model.collisionLod < 0 ? 255 : model.collisionLod);
		header.collSurfs = model.collision.empty() ? 0 : c1::ZPTR_INLINE;
		header.numCollSurfs = static_cast<std::int32_t>(model.collision.size());
		header.contents = model.contents;
		header.boneInfo = c1::ZPTR_INLINE;
		std::memcpy(header._unk316, &model.radius, 4);
		std::memcpy(header._unk316 + 4, &model.bounds, sizeof(model.bounds));
		const std::uint32_t head = pieces.add(header);
		pieces.add_string(model.name);

		// bone names: script strings
		{
			const std::vector<std::uint16_t> names(boneCount, 0);
			const std::uint32_t piece = pieces.add_array(names);
			for (std::size_t k = 0; k < boneCount; k++)
			{
				pieces.list[piece].scriptStrings.emplace_back(static_cast<std::uint32_t>(k * 2), model.boneNames[k]);
			}
		}
		if (children)
		{
			pieces.add_array(model.parentList);
			pieces.add_array(model.quats);
			pieces.add_array(model.trans);
		}
		pieces.add_array(model.partClassification);
		pieces.add_array(model.baseMat);
		{
			const std::vector<std::uint32_t> handles(materials.size(), c1::ZPTR_INLINE);
			const std::uint32_t piece = pieces.add_array(handles);
			for (std::size_t k = 0; k < materials.size(); k++)
			{
				const std::size_t index = impl_->editor.find_asset(c1::ASSET_TYPE_MATERIAL, materials[k]);
				if (index != ZoneEditor::npos)
				{
					pieces.to_asset(piece, static_cast<std::uint32_t>(k * 4), index);
				}
				else
				{
					add_reference<c1::Material>(pieces, materials[k]);
					impl_->log.push_back("model " + model.name + ": material " + materials[k] + " is not in the zone, referenced by name");
				}
			}
		}
		std::uint32_t memory = 0;
		for (std::size_t l = 0; l < model.lods.size(); l++)
		{
			const CompiledLod& lod = model.lods[l];
			c1::XModelSurfs surfs{};
			surfs.name = c1::ZPTR_INLINE;
			surfs.surfs = c1::ZPTR_INLINE;
			surfs.numsurfs = static_cast<std::uint16_t>(lod.surfaces.size());
			std::memcpy(surfs.partBits, lod.partBits, sizeof(surfs.partBits));
			pieces.add(surfs);
			const std::uint32_t namePiece = pieces.add_string(lod.surfsName);
			pieces.to_piece(head, static_cast<std::uint32_t>(offsetof(c1::XModel, lodInfo) + l * sizeof(c1::XModelLodInfo) + offsetof(c1::XModelLodInfo, surfsName)), namePiece);
			std::vector<c1::XSurface> headers;
			for (const CompiledSurface& s : lod.surfaces)
			{
				c1::XSurface h = s.header;
				h.triIndices = c1::ZPTR_INLINE;
				h.vertsBlend = s.blend.empty() ? 0 : c1::ZPTR_INLINE;
				h.verts0 = c1::ZPTR_INLINE;
				h.vertList = s.rigid.empty() ? 0 : c1::ZPTR_INLINE;
				headers.push_back(h);
			}
			pieces.add_array(headers);
			for (const CompiledSurface& s : lod.surfaces)
			{
				if (!s.blend.empty())
				{
					pieces.add_array(s.blend);
				}
				pieces.add_array(s.vertices);
				if (!s.rigid.empty())
				{
					std::vector<c1::XRigidVertList> lists = s.rigid;
					for (std::size_t k = 0; k < lists.size(); k++)
					{
						lists[k].collisionTree = k < s.trees.size() && !s.trees[k].nodes.empty() ? c1::ZPTR_INLINE : 0;
					}
					pieces.add_array(lists);
					// per list with a tree: header, nodes, leafs ( the order the game's loader reads them in )
					for (std::size_t k = 0; k < lists.size(); k++)
					{
						if (!lists[k].collisionTree)
						{
							continue;
						}
						const CompiledCollisionTree& tree = s.trees[k];
						struct TreeHeader
						{
							float trans[3], scale[3];
							std::uint32_t nodeCount, nodes, leafCount, leafs;
						};
						static_assert(sizeof(TreeHeader) == 40);
						TreeHeader th{};
						std::memcpy(th.trans, tree.trans, sizeof(th.trans));
						std::memcpy(th.scale, tree.scale, sizeof(th.scale));
						th.nodeCount = static_cast<std::uint32_t>(tree.nodes.size());
						th.nodes = c1::ZPTR_INLINE;
						th.leafCount = static_cast<std::uint32_t>(tree.leafs.size());
						th.leafs = tree.leafs.empty() ? 0 : c1::ZPTR_INLINE;
						pieces.add(th);
						pieces.add_array(tree.nodes);
						if (!tree.leafs.empty())
						{
							pieces.add_array(tree.leafs);
						}
						memory += static_cast<std::uint32_t>(sizeof(th) + tree.nodes.size() * 16 + tree.leafs.size() * 2);
					}
				}
				pieces.add_array(s.indices);
				memory += static_cast<std::uint32_t>(s.vertices.size() * 32 + s.indices.size() * 2 + s.blend.size() * 2 + s.rigid.size() * 12 + sizeof(c1::XSurface));
			}
		}
		if (!model.collision.empty())
		{
			struct CollSurf
			{
				std::uint32_t tris;
				std::int32_t count;
				c1::Bounds bounds;
				std::int32_t bone, contents, surfaceFlags;
			};
			static_assert(sizeof(CollSurf) == 44);
			std::vector<CollSurf> table;
			for (const CompiledCollisionSurface& c : model.collision)
			{
				table.push_back({ c1::ZPTR_INLINE, static_cast<std::int32_t>(c.triangles.size()), c.bounds, c.bone, c.contents, c.surfaceFlags });
			}
			pieces.add_array(table);
			for (const CompiledCollisionSurface& c : model.collision)
			{
				pieces.add_array(c.triangles);
				memory += static_cast<std::uint32_t>(c.triangles.size() * 48 + 44);
			}
		}
		pieces.add_array(model.boneInfo);
		// memory use of the model ( never zero in a shipped model ): what it brings along
		memory += static_cast<std::uint32_t>(sizeof(c1::XModel) + boneCount * (2 + 1 + 32 + 28) + children * (1 + 8 + 12));
		pieces.put32(head, offsetof(c1::XModel, _unk316) + 28, memory);
		for (const std::string& note : model.notes)
		{
			impl_->log.push_back("model " + model.name + ": " + note);
		}
		return impl_->editor.add_new_asset(c1::ASSET_TYPE_XMODEL, model.name, pieces.list);
	}

	Status AssetLinker::add_loaded_sound(const CompiledLoadedSound& sound)
	{
		return link_loaded_sound(impl_->editor, sound);
	}

	Status AssetLinker::add_snd_curve(const SndCurveDef& curve)
	{
		return link_snd_curve(impl_->editor, curve);
	}

	Status AssetLinker::add_sound_aliases(const CompiledAliasList& list)
	{
		return link_sound_alias_list(impl_->editor, list, &impl_->log);
	}

	Status AssetLinker::write_zone(std::vector<std::uint8_t>& zoneBytes)
	{
		return impl_->editor.write(zoneBytes);
	}

	Status AssetLinker::write(const fs::path& fastfile)
	{
		std::vector<std::uint8_t> zoneBytes;
		if (const Status s = write_zone(zoneBytes); !s)
		{
			return s;
		}
		return zone::write_fastfile(fastfile, zoneBytes, impl_->key);
	}
}
