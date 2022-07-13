#include "std_include.hpp"

#include "contact.h"

#define ZERO_EPSILON 0.000001f

// og dContactGeom struct
// + 2  depth
// + 3  g1
// + 4  g2
// ! + 5  surfFlags (in dContactGeomExt)
// + 6  next pos0
// + 10  next normal0
// + 12  next normal2

// dContactGeom now
// + 2  depth
// + 3  g1
// + 4  g2
// + 5  side1
// + 6  side2
// !+ 7  surfFlags (in dContactGeomExt)
// + 8  next pos0
// + 12  next normal0
// + 14  next normal2

// so to loop not yet optimized code -> sizeof(dContactGeomExt) / 4

namespace physics
{
	void Phys_AssignInitialGroups(const ContactList* contacts, int* group)
	{
		int* g0;

		if (!contacts || contacts->contactCount <= 0)
		{
			Assert();
		}

		float inorm[3];
		inorm[0] = contacts->contacts[104729 % contacts->contactCount].contact.normal[2];
		inorm[1] = contacts->contacts[104729 % contacts->contactCount].contact.normal[1];
		inorm[2] = contacts->contacts[104729 % contacts->contactCount].contact.normal[0];

		auto c0 = 0;
		float mx0 = FLT_MAX;
		int bestDotContact = -1;

		auto norm0 = &contacts->contacts[0].contact.normal[0]; // started at [2]

		if (!contacts->contactCount)
		{
			g0 = group;
		LABEL_13:
			Assert();
			goto LABEL_14;
		}

		do
		{
			g0 = group;
			group[c0] = -1;

			const auto cr_dot0 = norm0[1] * inorm[1] + norm0[0] * inorm[2] + inorm[0] * norm0[2];
			if (mx0 > cr_dot0)
			{
				mx0 = norm0[1] * inorm[1] + norm0[0] * inorm[2] + inorm[0] * norm0[2];
				bestDotContact = c0;
			}
			++c0;
			norm0 += (sizeof(dContactGeomExt) / 4); // optimize !! (was + 12)
		} while (c0 != contacts->contactCount);

		if (bestDotContact == -1)
		{
			goto LABEL_13;
		}

	LABEL_14:
		g0[104729 % contacts->contactCount] = 0;
		g0[bestDotContact] = 1;

		auto c1 = 0;
		auto mx1 = FLT_MAX;
		auto bestDotContact1 = -1;

		if (!contacts->contactCount)
		{
			Assert();
			return;
		}

		auto norm1 = &contacts->contacts[0].contact.normal[0]; // started at [2]
		do
		{
			const auto cr_dot1 = norm1[1] * inorm[1] + norm1[0] * inorm[2] + norm1[2] * inorm[0];
			const auto dot1 = norm1[0] * contacts->contacts[bestDotContact].contact.normal[0] + norm1[1] * contacts->contacts[bestDotContact].contact.normal[1] + norm1[2] * contacts->contacts[bestDotContact].contact.normal[2];

			auto v18 = cr_dot1;
			auto groupb = cr_dot1 - dot1;

			if (groupb < 0.0f)
			{
				v18 = dot1;
			}

			if (mx1 > v18)
			{
				mx1 = v18;
				bestDotContact1 = c1;
			}

			++c1;
			norm1 += (sizeof(dContactGeomExt) / 4); // optimize !! (was + 12)

		} while (c1 != contacts->contactCount);

		if (bestDotContact1 == -1)
		{
			Assert();
			return;
		}

		g0[bestDotContact1] = 2;
	}

	void Phys_KMeans(const ContactList* contacts, float(*centroid)[3], int* group)
	{
		float(*v3)[3]; // ebx
		float v4; // st7
		float* v5; // eax
		int v6; // ecx
		//int v7; // edx
		//float* v8; // ecx
		float* v9; // eax
		float* v10; // esi
		//int v11; // edi
		float v12; // st6
		//float* v13; // ebx
		int bestGroup; // esi
		//int v15; // ecx
		float* v16; // edx
		float v17; // st4
		float v18; // st5
		float v19; // [esp+Ch] [ebp-Ch]
		float v20; // [esp+Ch] [ebp-Ch]
		float v21; // [esp+10h] [ebp-8h]
		int v22; // [esp+14h] [ebp-4h]

		v3 = centroid;
		v22 = 0;
	LABEL_2:
		v4 = 0.0;

		while (true)
		{
			v5 = &(*v3)[2];
			v6 = 3;
			do
			{
				*(v5 - 2) = v4;
				v5 += 3;
				--v6;
				*(v5 - 4) = v4;
				*(v5 - 3) = v4;
			} while (v6);

			

			if (contacts->contactCount)
			{
				auto group0_norm = &contacts->contacts[0].contact.normal[0]; // started at [2]
				auto idx = 0;

				do
				{
					if (group[idx] != -1)
					{
						v9 = v3[group[idx]];
						v9[0] = group0_norm[0] + v9[0];
						v9[1] = group0_norm[1] + v9[1];
						v9[2] = group0_norm[2] + v9[2];
					}

					++idx;
					group0_norm += (sizeof(dContactGeomExt) / 4); // optimize !! (was + 12)

				} while (idx != contacts->contactCount);
			}

			v10 = (float*)v3;
			auto vec_size = 3;
			do
			{
				utils::vector::normalize(v10);
				v10 += 3;
				--vec_size;
			} while (vec_size);


			if (v22 == 5)
			{
				break;
			}
			++v22;

			if (!contacts->contactCount)
			{
				goto LABEL_2;
			}

			v4 = 0.0f;
			v12 = 0.0f;

			//auto v13 = &contacts->contacts[0].contact.normal[2];
			auto v13 = &contacts->contacts[0].contact.normal[0]; // started at [2]

			do
			{
				bestGroup = -1;
				v21 = -FLT_MAX;
				auto idx = 0;
				v16 = &(*centroid)[0]; // started at [2]
				do
				{
					v19 = v16[1] * v13[1] + v16[0] * v13[0] + v16[2] * v13[2];
					v17 = v21 - v19;
					v18 = v19;
					if (v17 < v12)
					{
						bestGroup = idx;
					}

					v20 = v17;
					if (v20 < v4)
					{
						v21 = v18;
					}

					++idx;
					v16 += 3;
				} while (idx != 3);

				if (bestGroup == -1)
				{
					Assert();
					v4 = 0.0f;
					v12 = 0.0f;
				}

				group[vec_size++] = bestGroup;
				v13 += (sizeof(dContactGeomExt) / 4); // optimize !! (was + 12)

			} while (vec_size != contacts->contactCount);

			v3 = centroid;
		}
	}

	void Phys_MergeGroups(const ContactList* contacts, float(*centroid)[3], int* group)
	{
		float* v4;
		float* v6;
		float* v9;

		int idx2;
		auto idx = 0;

		v4 = &(*centroid)[1];
		v9 = v4;

		do
		{
			idx2 = idx + 1;
			if (idx != 2)
			{
				v6 = v4 + 4;
				do
				{
					auto v8 = *(v6 - 2) * *(v4 - 1) + *(v6 - 1) * *v4 + v4[1] * v6[0];
					if (v8 > 0.999f)
					{
						if (physGlob.dumpContacts)
						{
							printf("Group %i merged into %i.\n", idx2, idx);
							v4 = v9;
						}

						auto group_idx = 0;
						if (contacts->contactCount)
						{
							do
							{
								if (group[group_idx] == idx2)
								{
									group[group_idx] = idx;
								}
								++group_idx;

							} while (group_idx != contacts->contactCount);

							v4 = v9;
						}
					}
					++idx2;
					v6 += 3;

				} while (idx2 != 3);

				idx2 = idx + 1;
			}

			idx = idx2;
			v4 += 3;
			v9 = v4;

		} while (idx2 != 2);
	}

	void Phys_GenerateGroupContacts(const ContactList* contacts, float(*centroid)[3], int* group, ContactList* out)
	{
		float* v4; // edx
		int v12; // eax
		int v13; // ebx
		int v15; // ecx
		int v17; // edx
		float* v18; // edx
		int i; // ecx
		float v20; // st6
		float v21; // st6
		int v22; // ebx
		int v23; // edx
		int v24; // ecx
		int v25; // edi
		int v26; // edx
		float v27[3][3]; // [esp+Ch] [ebp-68h] BYREF
		float v28; // [esp+30h] [ebp-44h]
		float v29; // [esp+34h] [ebp-40h]
		float v30; // [esp+38h] [ebp-3Ch]
		int v31[2]; // [esp+3Ch] [ebp-38h]
		int v32[2]; // [esp+44h] [ebp-30h]
		int v33; // [esp+4Ch] [ebp-28h]
		int v34; // [esp+50h] [ebp-24h]
		int v35; // [esp+54h] [ebp-20h]
		int v36; // [esp+58h] [ebp-1Ch]
		int v37; // [esp+5Ch] [ebp-18h]
		int v38; // [esp+60h] [ebp-14h]
		float v39; // [esp+64h] [ebp-10h]
		float* v40; // [esp+68h] [ebp-Ch]
		int v41; // [esp+6Ch] [ebp-8h]
		int v42; // [esp+70h] [ebp-4h]

		v4 = (float*)centroid;
		out->contactCount = 0;

		{
			auto v6 = &(*centroid)[0]; // started at 2
			auto idx = 3;
			do
			{
				v6[0] = 0.0f;
				v6[1] = 0.0f;
				v6[2] = 0.0f;

				v6 += 3;
				--idx;

			} while (idx);
		}

		if (contacts->contactCount)
		{
			const auto geom = contacts->contacts;
			for (auto it = 0; it < contacts->contactCount; it++)
			{
				if (group[it] == -1)
				{
					Assert();
				}

				float* cent = centroid[3 * group[it]];
				cent[0] = geom[it].contact.normal[0] + cent[0];
				cent[1] = geom[it].contact.normal[1] + cent[1];
				cent[2] = geom[it].contact.normal[2] + cent[2];
			}
		}

		v42 = 0;
		v40 = v4;

		do
		{
			utils::vector::normalize2(v40, v27[2]);
			Phys_CreateBasisFromNormal(v27[1], v27[2], (float*)v27);
			v39 = -FLT_MAX;
			v35 = -1;
			v36 = -1;
			v31[0] = -8388609;
			v31[1] = -8388609;
			v33 = -1;
			v34 = -1;
			v32[0] = 2139095039;
			v32[1] = 2139095039;
			v12 = contacts->contactCount;
			v13 = 0;
			v38 = -1;
			v41 = 0;

			if (v12)
			{
				auto v14 = &contacts->contacts[0].contact.pos[1];
				do
				{
					if (group[v13] == v42)
					{
						v15 = v13 + 1;
						if (v13 + 1 != v12)
						{
							auto v16 = v14 + (sizeof(dContactGeomExt) / 4) + 1;//+ 13;
							do
							{
								v17 = group[v15];
								if (v17 == v42)
								{
									v28 = *(v16 - 2) - *(v14 - 1);
									v29 = *(v16 - 1) - *v14;
									v30 = *v16 - v14[1];
									*(float*)&v37 = v29 * v29 + v28 * v28 + v30 * v30;

									if (*(float*)&v37 < 0.01f)
									{
										v17 = -1;
									}
									group[v15] = v17;
								}
								++v15;
								v16 += (sizeof(dContactGeomExt) / 4); //12;

							} while (v15 != contacts->contactCount);
						}

						++v41;
						v18 = &v27[0][2];

						for (i = 0; i != 2; ++i)
						{
							*(float*)&v37 = *(v18 - 1) * *v14 + *(v18 - 2) * *(v14 - 1) + v14[1] * *v18;
							v20 = *(float*)&v37;
							if (*(float*)&v32[i] > (double)*(float*)&v37)
							{
								v32[i] = v37;
								*(int*)((char*)&v33 + i * 4) = v13;
							}
							if (*(float*)&v31[i] < v20)
							{
								*(float*)&v31[i] = v20;
								*(int*)((char*)&v35 + i * 4) = v13;
							}
							v18 += 3;
						}

						if (v39 < v14[7]) // ???? prob wrong
						{
							v21 = v14[7];
							v38 = v13;
							v39 = v21;
						}
					}

					v12 = contacts->contactCount;
					++v13;
					v14 += (sizeof(dContactGeomExt) / 4); //12;

				} while (v13 != v12);

				if (v41)
				{
					v22 = v33;
					v23 = v34;
					memcpy(&out->contacts[out->contactCount++], &contacts->contacts[v33], sizeof(out->contacts[out->contactCount++]));
					v24 = out->contactCount;
					if (v23 != v22)
					{
						memcpy(&out->contacts[v24], &contacts->contacts[v23], sizeof(out->contacts[v24]));
						v24 = ++out->contactCount;
					}

					v25 = v35;
					if (v35 != v23 && v35 != v22)
					{
						memcpy(&out->contacts[v24], &contacts->contacts[v35], sizeof(out->contacts[v24]));
						v24 = ++out->contactCount;
						v25 = v35;
					}

					v26 = v36;
					if (v36 != v25 && v36 != v34 && v36 != v22)
					{
						memcpy(&out->contacts[v24], &contacts->contacts[v36], sizeof(out->contacts[v24]));
						v24 = ++out->contactCount;
						v25 = v35;
					}
					if (v38 != v26 && v38 != v25 && v38 != v34 && v38 != v22)
					{
						memcpy(&out->contacts[v24], &contacts->contacts[v38], sizeof(out->contacts[v24]));
						++out->contactCount;
					}
				}
			}

			v40 += 3;
			++v42;

		} while (v42 != 3);
	}

	void Phys_CheckOpposingNormals(dxBody* body0, dxBody* body1, ContactList* contacts)
	{
		float* i;
		float* v8;

		const auto count = contacts->contactCount;
		auto idx = 0;
		int idx2;

		if (count - 1 > 0)
		{
			for (i = &contacts->contacts[0].contact.normal[2]; ; i += (sizeof(dContactGeomExt) / 4)) //12)
			{
				idx2 = idx + 1;
				if (idx + 1 < count)
				{
					break;
				}

			LABEL_8:
				idx = idx2;
				if (idx2 >= count - 1)
				{
					return;
				}
			}

			v8 = i + (sizeof(dContactGeomExt) / 4); //12;
			while (true)
			{
				const auto dot = *(v8 - 1) * *(i - 1) + *(v8 - 2) * *(i - 2) + *v8 * *i;
				if (dot < -0.99f)
				{
					break;
				}

				++idx2;
				v8 += 12;

				if (idx2 >= count)
				{
					idx2 = idx + 1;
					goto LABEL_8;
				}
			}

			if (body0)
			{
				const auto data = static_cast<PhysObjUserData*>(dBodyGetData(body0));
				if (data->debugContacts)
				{
					printf("Body0 flagged stuck due to normals %d and %d\n", idx, idx2);
				}
				data->state = PHYS_OBJ_STATE_STUCK;
			}

			if (body1)
			{
				const auto data = static_cast<PhysObjUserData*>(dBodyGetData(body1));
				if (data->debugContacts)
				{
					printf("Body1 flagged stuck due to normals %d and %d\n", idx, idx2);
				}
				data->state = PHYS_OBJ_STATE_STUCK;
			}
		}
	}

	void Phys_DumpGroups(const float(*centroid)[3])
	{
		auto count = 0;
		const float* v2 = (float*)&(*centroid)[1];

		do
		{
			printf("Group %i: N:%g %g %g\n", count++, *(v2 - 1), *v2, v2[1]);
			v2 += 3;

		} while (count != 3);
	}

	void Phys_DumpContact(int contact, dContactGeom* geom)
	{
		printf("Contact %i: P:(%g %g %g), N:(%g %g %g), D:%g\n", 
			contact, geom->pos[0], geom->pos[1], geom->pos[2], 
			geom->normal[0], geom->normal[1], geom->normal[2], 
			geom->depth);
	}

	void Phys_DumpContacts(const ContactList* contacts, const int* group)
	{
		auto count = 0;
		if (contacts->contactCount)
		{
			auto v3 = &contacts->contacts[0].contact.normal[2];
			do
			{
				printf("Contact %i: G:%i, P:(%g %g %g), N:(%g %g %g), D:%g\n", count, group[count], *(v3 - 6), *(v3 - 5), *(v3 - 4), *(v3 - 2), *(v3 - 1), *v3, v3[2]);
				++count;
				v3 += (sizeof(dContactGeomExt) / 4); //12;

			} while (count != contacts->contactCount);
		}
	}

	void Phys_ReduceContacts(const ContactList* in, ContactList* out)
	{
		int v4[128];
		float centroid[3][3]; // xbox etc = [4][3]

		Phys_AssignInitialGroups(in, v4);
		Phys_KMeans(in, centroid, v4);

		if (physGlob.dumpContacts)
		{
			Phys_DumpGroups(centroid);
			Phys_DumpContacts(in, v4);
		}

		Phys_MergeGroups(in, centroid, v4);
		Phys_GenerateGroupContacts(in, centroid, v4, out);
	}

	void Phys_ApplyContactJitter(dxBody* body1, dxBody* body0, PhysWorld worldIndex, dContactGeom* contact)
	{
		if (!body0 || !body1)
		{
			//v4 = contact;
			if (contact->normal[2] >= 0.69999999f)
			{
				const auto w = &physGlob.worldData[worldIndex];

				auto b = body0;
				if (!body0)
				{
					b = body1;
				}

				auto max_mass = phys_jitterMaxMass->current.value;
				auto mm2 = max_mass - b->mass.mass;
				
				if (mm2 >= 0.0f)
				{
					max_mass = b->mass.mass;
				}
				mm2 = max_mass;


				auto idx = 0;
				mm2 = mm2 / phys_jitterMaxMass->current.value;
				mm2 = max_mass * (1.0f - mm2 * (mm2 * mm2));

				if (!(w->numJitterRegions <= 0))
				{
					auto ugly = &w->jitterRegions[0].innerRadius;

					do
					{
						float vec[3] = {};
						vec[0] = contact->pos[0] - *(ugly - 5);
						vec[1] = contact->pos[1] - *(ugly - 4);
						vec[2] = 0.0f;

						auto squared = vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2];

						if (*(ugly - 1) > squared)
						{
							squared = sqrt(squared);
							auto v11 = squared;

							float max = 1.0f;
							if (*ugly < squared)
							{
								squared = 1.0f / (ugly[1] - *ugly);
								max = (ugly[1] - v11) * squared;
							}

							squared = max;
							auto v19 = static_cast<float>(rand()) * (ugly[3] - ugly[2]) + ugly[2];
							v19 = v19 * squared * mm2 * 1000.0f / static_cast<float>(g_phys_msecStep[worldIndex]);

							const float v14[3] = 
							{
								0.0f, 0.0f, v19
							};

							Phys_ObjAddForce(worldIndex, b, contact->pos, v14);
						}
						++idx;
						ugly += 9;

					} while (idx < w->numJitterRegions);
				}
			}
		}
	}

	void Phys_RemoveOpposingNormalContacts(ContactList* contacts, const float* com)
	{
		int result; // eax
		int v3; // esi
		float* v4; // edx
		float normal2; // st6
		float* v6; // ecx
		float v7; // st5
		float v8; // st4
		float v9; // st3
		bool v10; // cc
		float v11; // [esp+18h] [ebp-10h]
		float v12; // [esp+18h] [ebp-10h]
		float v13; // [esp+18h] [ebp-10h]
		int v14; // [esp+1Ch] [ebp-Ch]
		int v15; // [esp+20h] [ebp-8h]
		float* v16; // [esp+24h] [ebp-4h]

		// dContactGeomExt size differs so offsets pot wrong?

		result = contacts->contactCount - 1;
		if (result > 0)
		{
			v3 = 1;
			v14 = 1;
			v4 = &contacts->contacts[0].contact.normal[2];

			

			do
			{
				v15 = v3;
				if (v3 < contacts->contactCount)
				{
					normal2 = contacts->contacts[0].contact.normal[2];
					v6 = v4 + (sizeof(dContactGeomExt) / 4); //12; // lands at next depth
					v7 = *(v4 - 1);
					v16 = v4 + (sizeof(dContactGeomExt) / 4); //12; // lands at next depth
					v8 = *(v4 - 2);

					do
					{
						// normal2 - normal3
						v11 = *(v6 - 2) * v8 + *(v6 - 1) * v7 + *v6 * normal2;
						if (v11 < -0.99f)
						{
							v12 = com[1] * v7 + *com * v8 + com[2] * normal2;
							v9 = v12;
							v13 = *(v4 - 1) * *(v4 - 5) + *(v4 - 2) * *(v4 - 6) + *v4 * *(v4 - 4);

							if (v9 - v13 < 0.0f)
							{
								memcpy(v4 - 6, v6 - 6, sizeof(dContactGeomExt)); // was 48u
								normal2 = *v4;
								v7 = *(v4 - 1);
								v8 = *(v4 - 2);
								v6 = v16;
							}

							memcpy(v6 - 6, &contacts->contacts[--contacts->contactCount], sizeof(dContactGeomExt)); // was 48u
							v6 = v16;
							v3 = v14;
						}
						v6 += (sizeof(dContactGeomExt) / 4); //12;
						v10 = ++v15 < contacts->contactCount;
						v16 = v6;

					} while (v10);
				}

				++v3;
				result = contacts->contactCount - 1;
				v4 += (sizeof(dContactGeomExt) / 4); //12;
				v14 = v3;

			} while (v3 - 1 < result);
		}
	}

	void Phys_CreateJointForEachContact(ContactList* contacts, dxBody* body0, dxBody* body1, dSurfaceParameters* surfParams, PhysWorld worldIndex)
	{
		dxBody* v6; // edi
		int v7; // eax
		float* v8; // esi
		float v11; // st7
		int v15; // esi
		float* v16; // edi
		float v17; // st6
		float v18; // st7
		float v19; // st5
		float v20; // st4
		float v24[6];
		float v26[4];
		BYTE v28[6]; // [esp+58h] [ebp-8h]
		char debug_contact; // [esp+5Fh] [ebp-1h]
		int worldIndexa; // [esp+78h] [ebp+18h]

		if (worldIndex >= 3)
		{
			Assert();
		}

		v6 = body0;
		Phys_CheckOpposingNormals(body0, body1, contacts);
		v7 = 0;
		debug_contact = 0;
		//v29 = 0.0;

		int idx = 0;

		v8 = &v24[2];
		do
		{
			v28[v7] = 0;
			if (v6)
			{
				const auto data = static_cast<PhysObjUserData*>(dBodyGetData(v6));
				if (!data)
				{
					Assert();
				}

				if(data->debugContacts)
				{
					debug_contact = 1;
				}

				v28[idx] = data->state == PHYS_OBJ_STATE_STUCK;

				if (physGlob.worldData[worldIndex].useContactCentroids)
				{
					*(v8 - 2) = data->contactCentroid[0];
					*(v8 - 1) = data->contactCentroid[1];
					v11 = data->contactCentroid[2];
				}
				else
				{
					auto v12 = dBodyGetPosition(v6);
					*(v8 - 2) = v12[0];
					*(v8 - 1) = v12[1];
					v11 = v12[2];
				}

				v8[0] = v11;
			}

			v6 = body1;
			v7 = idx + 1;
			v8 += 3;
			++idx;

		} while (idx < 2);

		if (debug_contact)
		{
			printf("Pre Oppose Check:\n");
			if (contacts->contactCount)
			{
				for (auto cc = 0; cc < contacts->contactCount; cc++)
				{
					Phys_DumpContact(cc, &contacts->contacts[cc].contact);
				}
			}
		}

		if (v28[0])
		{
			Phys_RemoveOpposingNormalContacts(contacts, v24);
		}
		else
		{
			if (!v28[1])
			{
				goto LABEL_22;
			}
			Phys_RemoveOpposingNormalContacts(contacts, &v24[3]);
		}

	LABEL_22:
		if (debug_contact)
		{
			printf("Final Contacts:\n");
		}
		v15 = 0;
		worldIndexa = 0;

		if (contacts->contactCount)
		{
			// we need to wrap it into a dContact
			// *(v16 - 6) = dContactGeom
			v16 = &contacts->contacts[0].contact.normal[2];
			do
			{
				auto dot = *(v16 - 1) * *(v16 - 1) + *(v16 - 2) * *(v16 - 2) + *v16 * *v16;
				if (dot <= ZERO_EPSILON)
				{
					Assert();
				}

				if (v28[0])
				{
					dBodyGetPointVel(body0, *(v16 - 6), *(v16 - 5), *(v16 - 4), v26);
					dNormalize3(v26);
					v17 = v26[0];
					v18 = v26[1];
					v19 = v26[2];
					dot = *v16 * v26[2] + *(v16 - 1) * v26[1] + *(v16 - 2) * v26[0];
					v20 = dot;
					if (dot > 0.0099999998f)
					{
						goto LABEL_30;
					}
				}
				if (v28[1] && (dBodyGetPointVel(body1, *(v16 - 6), *(v16 - 5), *(v16 - 4), v26), dNormalize3(v26), v17 = v26[0], v18 = v26[1], v19 = v26[2], dot = *v16 * v26[2] + *(v16 - 1) * v26[1] + *(v16 - 2) * v26[0], v20 = dot, dot > 0.0099999998f))
				{
				LABEL_30:
					if (debug_contact)
					{
						printf("Killing contact %d, pointvel %f %f %f dot %f\n", v15, v17, v18, v19, v20);
					}
				}
				else
				{
					auto contact_geom = reinterpret_cast<dContactGeom*>(v16 - 6);

					Phys_ApplyContactJitter(body1, body0, worldIndex, contact_geom);
					if (physGlob.dumpContacts || debug_contact)
					{
						Phys_DumpContact(worldIndexa, contact_geom);
					}

					dContact c = {};
					c.surface = *surfParams;
					c.geom = *(dContactGeom*)(v16 - 6);

					if (phys_noIslands->current.enabled && body0 && body1)
					{
						auto j0 = dJointCreateContact(physGlob.world[worldIndex], physGlob.contactgroup[worldIndex], &c);
						if (!j0)
						{
							printf("Maximum number of ODE physics contact points exceeded.\n");
							return;
						}

						dJointAttach(j0, body0, nullptr);


						//auto v22 = dJointCreateContact(physGlob.world[v5], physGlob.contactgroup[v5], surfParams, (dContactGeom*)(v16 - 6));
						auto j1 = dJointCreateContact(physGlob.world[worldIndex], physGlob.contactgroup[worldIndex], &c);
						if (!j1)
						{
							printf("Maximum number of ODE physics contact points exceeded.\n");
							return;
						}

						// would set debug to true here

						dJointAttach(j1, nullptr, body1);
						v15 = worldIndexa;
					}
					else
					{
						//auto v23 = dJointCreateContact(physGlob.world[v5], physGlob.contactgroup[v5], surfParams, (dContactGeom*)(v16 - 6));
						auto j = dJointCreateContact(physGlob.world[worldIndex], physGlob.contactgroup[worldIndex], &c);
						if (!j)
						{
							printf("Maximum number of ODE physics contact points exceeded.\n");
							return;
						}

						// would set debug to true here
						
						dJointAttach(j, body0, body1);
						v15 = worldIndexa;
					}
				}

				++v15;
				v16 += (sizeof(dContactGeomExt) / 4); //12;
				worldIndexa = v15;

			} while (v15 != contacts->contactCount);
		}
	}
}