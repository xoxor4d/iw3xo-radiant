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
		int* v7; // r18
		float* v8; // r11
		int v9; // r10
		int v10; // r8
		int* v11; // r9
		//float* v12; // r10
		//dReal v12;
		int v13; // r11
		int v14; // r7
		float* v15; // r11
		double v16; // fp0
		int v17; // r19
		float(*v18)[3]; // r20
		int v19; // r11
		double v20; // fp31
		int v21; // r21
		int v22; // r23
		int v23; // r28
		//ContactList* v24; // r27
		int* v25; // r24
		int v26; // r29
		int* v27; // r31
		const float* v28; // r30
		double v29; // fp1
		int v30; // r11
		double v31; // fp13
		double v32; // fp12
		float* v33; // r9
		double v34; // fp11
		int i; // r11
		double v36; // fp0
		int v37; // r28
		int v38; // r29
		int v39; // r11
		BOOL v40; // cr58
		int v41; // r30
		int v42; // r31
		int v43[8]; // [sp+50h] [-E0h] BYREF
		float v44[4]; // [sp+70h] [-C0h] BYREF
		float v45[6];

		v7 = group;
		v8 = &(*centroid)[2];
		v9 = 3;
		out->contactCount = 0;
		do
		{
			--v9;
			*(v8 - 2) = 0.0;
			*(v8 - 1) = 0.0;
			*v8 = 0.0;
			v8 += 3;
		} while (v9);

		v10 = 0;
		if (contacts->contactCount)
		{
			v11 = group;
			auto v12 = &contacts->contacts[0].contact.normal[2];
			do
			{
				v13 = *v11;
				++v10;
				v14 = 2 * *v11++;
				v15 = &(*centroid)[v13 + v14];
				*v15 = *(v12 - 2) + *v15;
				v15[1] = *(v12 - 1) + v15[1];
				v16 = *v12;
				v12 += (sizeof(dContactGeomExt) / 4);
				v15[2] = v15[2] + v16;
			} while (v10 != contacts->contactCount);
		}

		v17 = 0;
		v18 = centroid;
		do
		{
			utils::vector::normalize2(v18[0], v44); //Vec3NormalizeTo(v18, v44);
			Phys_CreateBasisFromNormal(&v45[3], v44, v45);
			v19 = contacts->contactCount;
			v20 = -3.4028235e38;
			v43[2] = -1;
			v21 = -1;
			v43[6] = -8388609;
			v22 = 0;
			v43[0] = -1;
			v23 = 0;
			v43[3] = -1;
			v43[4] = 2139095039;
			v43[7] = -8388609;
			v43[1] = -1;
			v43[5] = 2139095039;
			if (v19)
			{
				auto v24 = contacts->contacts;
				v25 = v7;
				do
				{
					if (*v25 == v17)
					{
						v26 = v23 + 1;
						if (v23 + 1 != v19)
						{
							v27 = v25 + 1;
							v28 = v24->contact.pos;
							do
							{
								if (*v27 == v17)
								{
									v29 = fx_system::Vec3DistanceSq(v24->contact.pos, v28);
									v30 = -1;
									if (v29 >= 0.010000001)
										v30 = *v27;
									*v27 = v30;
								}
								++v26;
								v28 += (sizeof(dContactGeomExt) / 4);
								++v27;
							} while (v26 != contacts->contactCount);
						}

						v31 = v24->contact.pos[0];
						++v22;
						v32 = v24->contact.pos[2];
						v33 = &v45[1];
						v34 = v24->contact.pos[1];

						for (i = 0; i != 2; ++i)
						{
							v36 = ((*v33 * v34) + ((*(v33 - 1) * v31) + (v33[1] * v32)));
							if (v36 < *&v43[i + 4])
							{
								*&v43[i + 4] = (*v33 * v34) + ((*(v33 - 1) * v31) + (v33[1] * v32));
								v43[i] = v23;
							}
							if (v36 > *&v43[i + 6])
							{
								*&v43[i + 6] = v36;
								v43[i + 2] = v23;
							}
							v33 += 3;
						}
						if (v24->contact.depth > v20)
						{
							v20 = v24->contact.depth;
							v21 = v23;
						}
					}
					v19 = contacts->contactCount;
					++v23;
					++v25;
					++v24; // += (sizeof(dContactGeomExt));
				} while (v23 != v19);

				if (v22)
				{
					v37 = v43[0];
					memcpy(
						&out->contacts[out->contactCount],
						&contacts->contacts[v43[0]],
						sizeof(out->contacts[out->contactCount]));
					v38 = v43[1];
					v39 = out->contactCount + 1;
					v40 = v43[1] == v37;
					out->contactCount = v39;
					if (!v40)
					{
						memcpy(&out->contacts[v39], &contacts->contacts[v38], sizeof(out->contacts[v39]));
						v39 = out->contactCount + 1;
						out->contactCount = v39;
					}
					v41 = v43[2];
					if (v43[2] != v38 && v43[2] != v37)
					{
						memcpy(&out->contacts[v39], &contacts->contacts[v43[2]], sizeof(out->contacts[v39]));
						v39 = out->contactCount + 1;
						out->contactCount = v39;
					}
					v42 = v43[3];
					if (v43[3] != v41 && v43[3] != v38 && v43[3] != v37)
					{
						memcpy(&out->contacts[v39], &contacts->contacts[v43[3]], sizeof(out->contacts[v39]));
						v39 = out->contactCount + 1;
						out->contactCount = v39;
					}
					if (v21 != v42 && v21 != v41 && v21 != v38 && v21 != v37)
					{
						memcpy(&out->contacts[v39], &contacts->contacts[v21], sizeof(out->contacts[v39]));
						++out->contactCount;
					}
				}
			}
			++v17;
			++v18;
		} while (v17 != 3);
	}

	//void Phys_GenerateGroupContacts(const ContactList* contacts, float(*centroid)[3], int* group, ContactList* out)
	//{
	//	float* v4; // edx
	//	int v12; // eax
	//	int v13; // ebx
	//	int v15; // ecx
	//	int v17; // edx
	//	float* v18; // edx
	//	int i; // ecx
	//	float v20; // st6
	//	float v21; // st6
	//	int v22; // ebx
	//	int v23; // edx
	//	int v24; // ecx
	//	int v25; // edi
	//	int v26; // edx
	//	//float v27[3][3]; // [esp+Ch] [ebp-68h] BYREF
	//	float basis[3];
	//	float v27[3];
	//	float normal[3];
	//	float v28; // [esp+30h] [ebp-44h]
	//	float v29; // [esp+34h] [ebp-40h]
	//	float v30; // [esp+38h] [ebp-3Ch]
	//	int v31[9];
	//	int v38; // [esp+60h] [ebp-14h]
	//	float v39; // [esp+64h] [ebp-10h]
	//	float* v40; // [esp+68h] [ebp-Ch]
	//	int v41; // [esp+6Ch] [ebp-8h]
	//	int v42; // [esp+70h] [ebp-4h]

	//	out->contactCount = 0;
	//	{
	//		auto v6 = &(*centroid)[0]; // started at 2
	//		auto idx = 3;
	//		do
	//		{
	//			v6[0] = 0.0f;
	//			v6[1] = 0.0f;
	//			v6[2] = 0.0f;

	//			v6 += 3;
	//			--idx;

	//		} while (idx);
	//	}

	//	if (contacts->contactCount)
	//	{
	//		const auto geom = contacts->contacts;
	//		for (auto it = 0; it < contacts->contactCount; it++)
	//		{
	//			if (group[it] == -1)
	//			{
	//				Assert();
	//			}

	//			float* cent = centroid[3 * group[it]];
	//			cent[0] = geom[it].contact.normal[0] + cent[0];
	//			cent[1] = geom[it].contact.normal[1] + cent[1];
	//			cent[2] = geom[it].contact.normal[2] + cent[2];
	//		}
	//	}

	//	v42 = 0;
	//	v40 = (float*)centroid;

	//	do
	//	{
	//		utils::vector::normalize2(v40, normal);
	//		Phys_CreateBasisFromNormal(&v27[3], normal, basis);
	//		v39 = -FLT_MAX;

	//		v31[6] = -1;
	//		v31[7] = -1;
	//		v31[0] = -8388609;
	//		v31[1] = -8388609;
	//		v31[4] = -1;
	//		v31[5] = -1;
	//		v31[2] = 2139095039;
	//		v31[3] = 2139095039;

	//		v13 = 0;
	//		v38 = -1;
	//		v41 = 0;

	//		// og dContactGeom struct
	//		// + 2  depth
	//		// + 3  g1
	//		// + 4  g2
	//		// ! + 5  surfFlags (in dContactGeomExt)
	//		// + 6  next pos0
	//		// + 10  next normal0
	//		// + 12  next normal2
	//		
	//		// dContactGeom now
	//		// + 2  depth
	//		// + 3  g1
	//		// + 4  g2
	//		// + 5  side1
	//		// + 6  side2
	//		// !+ 7  surfFlags (in dContactGeomExt)
	//		// + 8  next pos0
	//		// + 12  next normal0
	//		// + 14  next normal2

	//		if (contacts->contactCount)
	//		{
	//			auto v14 = &contacts->contacts[0].contact.pos[0];
	//			do
	//			{
	//				if (group[v13] == v42)
	//				{
	//					v15 = v13 + 1;
	//					for (auto xx = 1; v15 != contacts->contactCount; xx++)
	//					{
	//						v17 = group[v15];
	//						if (v17 == v42)
	//						{
	//							v28 = contacts->contacts[xx].contact.pos[0] - v14[0];
	//							v29 = contacts->contacts[xx].contact.pos[1] - v14[1];
	//							v30 = contacts->contacts[xx].contact.pos[2] - v14[2];

	//							if (v29 * v29 + v28 * v28 + v30 * v30 < 0.01f)
	//							{
	//								v17 = -1;
	//							}

	//							group[v15] = v17;
	//						}

	//						++v15;
	//					}


	//					++v41;
	//					v18 = &basis[0];

	//					for (i = 0; i != 2; ++i)
	//					{
	//						*(float*)&v31[8] = v18[1] * v14[1] + v18[0] * v14[0] + v18[2] * v14[2];  //*(v18 - 1) * *v14 + *(v18 - 2) * *(v14 - 1) + v14[1] * *v18;
	//						v20 = *(float*)&v31[8];

	//						if (static_cast<float>(v31[i + 2]) > static_cast<float>(v31[8]))
	//						{
	//							v31[i + 2] = v31[8];
	//							v31[i + 4] = v13;
	//						}

	//						if (static_cast<float>(v31[i]) < v20)
	//						{
	//							*(float*)&v31[i] = v20;
	//							v31[i + 6] = v13;
	//						}
	//						v18 += 3;
	//					}

	//					if (v39 < v14[6]) // depth
	//					{
	//						v21 = v14[6];
	//						v38 = v13;
	//						v39 = v21;
	//					}
	//				}

	//				v12 = contacts->contactCount;
	//				++v13;
	//				v14 += (sizeof(dContactGeomExt) / 4); //12;

	//			} while (v13 != v12);


	//			if (v41)
	//			{
	//				v22 = v31[4];
	//				v23 = v31[5];
	//				memcpy(&out->contacts[out->contactCount++], &contacts->contacts[v31[4]], sizeof(out->contacts[out->contactCount++]));
	//				v24 = out->contactCount;
	//				if (v23 != v22)
	//				{
	//					memcpy(&out->contacts[v24], &contacts->contacts[v23], sizeof(out->contacts[v24]));
	//					v24 = ++out->contactCount;
	//				}
	//				v25 = v31[6];
	//				if (v31[6] != v23 && v31[6] != v22)
	//				{
	//					memcpy(&out->contacts[v24], &contacts->contacts[v31[6]], sizeof(out->contacts[v24]));
	//					v24 = ++out->contactCount;
	//					v25 = v31[6];
	//				}
	//				v26 = v31[7];
	//				if (v31[7] != v25 && v31[7] != v31[5] && v31[7] != v22)
	//				{
	//					memcpy(&out->contacts[v24], &contacts->contacts[v31[7]], sizeof(out->contacts[v24]));
	//					v24 = ++out->contactCount;
	//					v25 = v31[6];
	//				}
	//				if (v38 != v26 && v38 != v25 && v38 != v31[5] && v38 != v22)
	//				{
	//					memcpy(&out->contacts[v24], &contacts->contacts[v38], sizeof(out->contacts[v24]));
	//					++out->contactCount;
	//				}
	//			}
	//		}

	//		v40 += 3;
	//		++v42;

	//	} while (v42 != 3);
	//}

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

	void Phys_CreateJointForEachContact_NEW(ContactList* contacts, dxBody* body0, dxBody* body1, dSurfaceParameters* surfParms, PhysWorld worldIndex)
	{
		//float* v10; // r4
		int v11; // r25
		int v12; // r31
		dxBody* v13; // r30
		float* v14; // r29
		BOOL v15; // r27
		PhysObjUserData* data; // r3
		double v17[3]; // fp0
		//double v18; // fp13
		//double v19; // fp12
		const float* v20; // r3
		int v21; // r16
		unsigned __int8 v22; // r15
		float* v23; // r3
		float* i; // r29
		dxJoint* v25; // r3
		dxJoint* v26; // r3
		dxBody* v27; // r4
		const float* v28; // r5
		unsigned __int8 v29[16]; // [sp+60h] [-D0h] BYREF
		float v30[3]; // [sp+70h] [-C0h] BYREF
		float v33[6]; // [sp+80h] [-B0h] BYREF
		//float v34[3]; // [sp+8Ch] [-A4h] BYREF

		Phys_CheckOpposingNormals(body0, body1, contacts);
		v11 = 0;
		v12 = 0;
		v13 = body0;
		v14 = &v33[2];
		v15 = physGlob.worldData[worldIndex].useContactCentroids;
		do
		{
			v29[v12] = 0;
			if (v13)
			{
				data = static_cast<PhysObjUserData*>(dBodyGetData(v13));
				v29[v12] = data->state == PHYS_OBJ_STATE_STUCK;
				if (v15)
				{
					v17[0] = data->contactCentroid[0];
					v17[1] = data->contactCentroid[1];
					v17[2] = data->contactCentroid[2];
				}
				else
				{
					v20 = dBodyGetPosition(v13);
					v17[0] = *v20;
					v17[1] = v20[1];
					v17[2] = v20[2];
				}
				*v14 = v17[2];
				*(v14 - 1) = v17[1];
				*(v14 - 2) = v17[0];
			}
			++v12;
			v14 += 3;
			v13 = body1;
		} while (v12 < 2);
		v21 = v29[0];
		v22 = v29[1];
		if (v29[0])
		{
			v23 = v33;
		LABEL_12:
			Phys_RemoveOpposingNormalContacts(contacts, v23);
			goto LABEL_13;
		}
		if (v29[1])
		{
			v23 = &v33[3];
			goto LABEL_12;
		}
	LABEL_13:
		if (!contacts->contactCount)
		{
			return;
		}

		for (i = &contacts->contacts[0].contact.normal[2]; ; i += (sizeof(dContactGeomExt) / 4))
		{
			dContact c = {};
			c.surface = *surfParms;
			c.geom = *(dContactGeom*)(i - 6);

			if (v21)
			{
				if(!utils::vector::compare(body0->lvel, game::vec3_origin) || !utils::vector::compare(body0->avel, game::vec3_origin))
				{
					dBodyGetPointVel(body0, *(i - 6), *(i - 5), *(i - 4), v30);
					dNormalize3(v30);
					if ((float)((float)(*i * v30[2]) + (float)((float)(*(i - 1) * v30[1]) + (float)(*(i - 2) * v30[0]))) > 0.0099999998)
					{
						goto LABEL_31;
					}
				}
			}

			if (v22)
			{
				if (!utils::vector::compare(body1->lvel, game::vec3_origin) || !utils::vector::compare(body1->avel, game::vec3_origin))
				{
					dBodyGetPointVel(body1, *(i - 6), *(i - 5), *(i - 4), v30);
					dNormalize3(v30);
					if ((float)((float)(*i * v30[2]) + (float)((float)(*(i - 1) * v30[1]) + (float)(*(i - 2) * v30[0]))) > 0.0099999998)
					{
						goto LABEL_31;
					}
				}
				
			}

			Phys_ApplyContactJitter(body1, body0, worldIndex, (dContactGeom*)(i - 6));

			if (physGlob.dumpContacts)
			{
				game::printf_to_console("Contact %i: P:(%g %g %g), N:(%g %g %g), D:%g\n");
			}

			if (!phys_noIslands->current.enabled || !body0 || !body1)
			{
				v26 = dJointCreateContact(physGlob.world[worldIndex], physGlob.contactgroup[worldIndex], &c);
				if (!v26)
				{
					break;
				}

				v27 = body0;
				goto LABEL_29;
			}

			v25 = dJointCreateContact(physGlob.world[worldIndex], physGlob.contactgroup[worldIndex], &c);
			if (!v25)
			{
				break;
			}

			dJointAttach(v25, body0, 0);
			v26 = dJointCreateContact(physGlob.world[worldIndex], physGlob.contactgroup[worldIndex], &c);

			if (!v26)
			{
				break;
			}
			v27 = 0;

		LABEL_29:
			dJointAttach(v26, v27, body1);
			if (phys_drawcontacts->current.enabled)
			{
				auto gg = (dContactGeom*)(i - 6);
				//Phys_DebugDrawContactPoint(i - 6, i - 2, i[2], v28);

				float mins[3] = { -1.0f, -1.0f, -1.0f };
				float maxs[3] = { 1.0f, 1.0f, 1.0f };

				utils::vector::add(gg->pos, mins, mins);
				utils::vector::add(gg->pos, maxs, maxs);

				const float color[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
				game::R_AddDebugBox(game::get_frontenddata()->debugGlobals, mins, maxs, color);
			}

		LABEL_31:
			if (++v11 == contacts->contactCount)
			{
				return;
			}
		}

		game::printf_to_console("[WARN] Maximum number of ODE physics contact points exceeded.\n");
	}

	void Phys_CreateJointForEachContact(ContactList* contacts, dxBody* body0, dxBody* body1, dSurfaceParameters* surfParams, PhysWorld worldIndex)
	{
		dxBody* v6; // edi
		int v7; // eax
		float* v8; // esi
		float v11; // st7
		int v15; // esi
		//float* v16; // edi
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
			//v16 = &contacts->contacts[0].contact.normal[2];
			auto v16 = contacts->contacts;
			do
			{
				auto dot = v16->contact.normal[1] * v16->contact.normal[1] + v16->contact.normal[0] * v16->contact.normal[0] + v16->contact.normal[2] * v16->contact.normal[2];
				if (dot <= ZERO_EPSILON)
				{
					Assert();
				}

				if (v28[0])
				{
					if (!utils::vector::compare(body0->lvel, game::vec3_origin) || !utils::vector::compare(body0->avel, game::vec3_origin))
					{
						dBodyGetPointVel(body0, v16->contact.pos[0], v16->contact.pos[1], v16->contact.pos[2], v26);
						dNormalize3(v26);
						v17 = v26[0];
						v18 = v26[1];
						v19 = v26[2];
						dot = v16->contact.normal[2] * v26[2] + v16->contact.normal[1] * v26[1] + v16->contact.normal[0] * v26[0];
						v20 = dot;
						if (dot > 0.0099999998f)
						{
							goto LABEL_30;
						}
					}
					
				}

				if (v28[1])
				{
					if (!utils::vector::compare(body1->lvel, game::vec3_origin) || !utils::vector::compare(body1->avel, game::vec3_origin))
					{
						dBodyGetPointVel(body1, v16->contact.pos[0], v16->contact.pos[1], v16->contact.pos[2], v26);
						dNormalize3(v26);
						v17 = v26[0];
						v18 = v26[1];
						v19 = v26[2];
						dot = v16->contact.normal[2] * v26[2] + v16->contact.normal[1] * v26[1] + v16->contact.normal[0] * v26[0];
						v20 = dot;

						if (dot > 0.0099999998f)
						{
						LABEL_30:
							if (debug_contact)
							{
								printf("Killing contact %d, pointvel %f %f %f dot %f\n", v15, v17, v18, v19, v20);
							}
						}
						else
						{
							goto DO_CONTACT;
						}
					}
					else
					{
						goto DO_CONTACT;
					}
				}
				else
				{
					DO_CONTACT:
					auto contact_geom = &v16->contact; //reinterpret_cast<dContactGeom*>(v16 - 6);

					Phys_ApplyContactJitter(body1, body0, worldIndex, contact_geom);
					if (physGlob.dumpContacts || debug_contact)
					{
						Phys_DumpContact(worldIndexa, contact_geom);
					}

					dContact c = {};
					c.surface = *surfParams;
					c.geom = v16->contact; //*(dContactGeom*)(v16 - 6);

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

					if (phys_drawcontacts->current.enabled)
					{
						auto gg = v16->contact;
						//Phys_DebugDrawContactPoint(i - 6, i - 2, i[2], v28);

						float mins[3] = { -1.0f, -1.0f, -1.0f };
						float maxs[3] = { 1.0f, 1.0f, 1.0f };

						utils::vector::add(gg.pos, mins, mins);
						utils::vector::add(gg.pos, maxs, maxs);

						const float color[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
						game::R_AddDebugBox(game::get_frontenddata()->debugGlobals, mins, maxs, color);
					}
				}

				++v15;
				++v16; // += (sizeof(dContactGeomExt) / 4); //12;
				worldIndexa = v15;

			} while (v15 != contacts->contactCount);
		}
	}
}